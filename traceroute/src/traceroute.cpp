#include <string.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>

#include <iostream>
#include <iomanip>
#include <thread>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>        // struct udphdr
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netdb.h>              // getaddrinfo()

#include "argvparser.h"
#include "signal-handler.h"
#include "icmp-utils.h"

#define TIMEOUT_REPLY       -3
#define TTL_EXCEEDED_REPLY  -2
#define HOSTNAME_HIT_REPLY  -1

#define NUM_RETRIES     1       // send up to NUM_RETRIES udp packets per ttl
#define REPLY_TIMEOUT   1       // wait REPLY_TIMEOUT secs for an icmp reply
#define DST_PORT        32768 + 666 // this is how Stevens sets the upd dst 
                                    // port. i'll follow the same (note that 
                                    // the sockaddr_in->sin_port attr. is a 
                                    // 16 bit unsigned value, which can go up 
                                    // till 65536)

#define MAX_TTL         30          // following Stevens' lead again

#define SERVICE_HTTP    "http"
// as defined in Steven's unp book, fig. 28.4
#define MAX_BUFFER_SIZE 1500
#define MAX_STRING_SIZE 256

#define OPTION_HOSTNAME     (char *) "hostname"
#define OPTION_USE_PING     (char *) "use-ping"

using namespace CommandLineProcessing;

struct snd_record {

    uint16_t snd_seq;
    uint16_t snd_ttl;
    struct timeval snd_timestamp;
};

struct traceroute_params {

    int snd_sckt_fd;
    int rcv_sckt_fd;

    struct sockaddr rcv_addr;

    bool icmp_echo_probe;
    SignalHandler signal_handler;
};

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ntraceroute (yet another ping)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_HOSTNAME,
            "hostname to traceroute to",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_USE_PING,
            "use ICMP ECHO packets instead of UDP packets",
            ArgvParser::NoOptionAttribute);

    return parser;
}

struct timeval * tv_sub(struct timeval * out, struct timeval * in) {

    if ((out->tv_usec -= in->tv_usec) < 0) {   /* out -= in */

        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    
    out->tv_sec -= in->tv_sec;

    return out;
}

int get_icmp_response(
    int rcv_sckt_fd,
    sockaddr * rcv_addr,
    socklen_t * rcv_addrlen,
    int snd_ttl,
    int snd_seq,
    int snd_src_port,
    bool icmp_echo_probe,
    SignalHandler signal_handler,
    struct timeval * rcv_timestamp) {

    int rcv_bytes = 0, return_code = 0;
    char rcv_buff[MAX_STRING_SIZE] = "";
    // we'll need to read both ipv4, icmp and udp headers
    struct ip * ipv4_hdr = NULL;
    struct icmp * icmp_hdr = NULL, * in_icmp_hdr = NULL;
    struct udphdr * udp_hdr = NULL;

    // raise SIGALRM in 3 seconds and disarm the signal
    signal_handler.disarm_signal();
    alarm(REPLY_TIMEOUT);

    for ( ; ; ) {

        if (signal_handler.is_signal())
            return TIMEOUT_REPLY;

        if ((rcv_bytes = recvfrom(rcv_sckt_fd, rcv_buff, sizeof(rcv_buff), 0, rcv_addr, rcv_addrlen)) < 0) {

            // since we're using a SIGALRM handler, recvfrom() may have been 
            // interrupted due to it. we therefore 're-cycle' to check if this is 
            // what really happened.
            if (errno == EINTR) {
                continue;
            } else {

                std::cerr << "traceroute::get_icmp_response() : [ERROR] error in recvfrom(): " 
                    << strerror(errno) << std::endl;
            }
        }

        // we start by reading the ipv4 header. this is as simple as typecasting 
        // the byte payload into a struct ip *
        ipv4_hdr = (struct ip *) rcv_buff;
        // 1st, the ipv4 header len : in ipv4, the header length isn't fixed (as 
        // with ipv6, fixed as 40 byte). this is the total length of the header, 
        // including options. therefore we must retrieve it to know 'where' the 
        // icmp header starts. also note the '<< 2': the length field is 4 bit 
        // long, and its unit is '4 byte blocks'. to know the length in bytes, 
        // one should thus multiply it by 4, i.e. the same as left-shifting by 
        // 2 bit ('<< 2').
        int ipv4_hdr_len = ipv4_hdr->ip_hl << 2;
        // if the protocol field isn't ICMP, abort
        if (ipv4_hdr->ip_p != IPPROTO_ICMP) {

            std::cerr << "traceroute::get_icmp_response() : [ERROR] not an ICMP "\
                "packet. skip processing." << std::endl;        

            continue;
        }

        // with the ipv4 header length, we now know the start of the icmp 
        // header
        icmp_hdr = (struct icmp *) (rcv_buff + ipv4_hdr_len);
        // the icmp_len should be at least 8 byte (size of icmp header). 
        // if not, abort.
        int icmp_len = 0;
        if ((icmp_len = rcv_bytes - ipv4_hdr_len) < 8) {

            std::cerr << "traceroute::get_icmp_response() : [ERROR] malformed ICMP "\
                "packet. header too short (" << icmp_len << " byte). skip "\
                "processing." << std::endl;  

            continue;              
        }

        // we now check if this is an icmp reply sent by a router-in-the-middle 
        // which dropped the ttl to 0.
        if (icmp_hdr->icmp_type == ICMP_TIMXCEED 
            && icmp_hdr->icmp_code == ICMP_TIMXCEED_INTRANS) {

            // if icmp echos were used as probes, we should read the payload 
            // of the 'outer' icmp packet as an icmp packet, otherwise as an 
            // udp packet.
            if (icmp_echo_probe) {

                if (ICMPUtils::get_icmphdr_from_icmp(rcv_buff, rcv_bytes, ipv4_hdr_len, icmp_len, in_icmp_hdr) < 0)
                    continue;
 
                // since we don't have access to port numbers w/ icmp packets, 
                // we validate the reply based on the payload fields
                struct snd_record * snd_rec = (struct snd_record *) in_icmp_hdr->icmp_data;          

                if ((snd_rec->snd_seq == snd_seq) && (snd_rec->snd_ttl == snd_ttl)) {

                    return_code = TTL_EXCEEDED_REPLY;
                    break;
                }

            } else {

                if (ICMPUtils::get_updhdr_from_icmp(rcv_buff, ipv4_hdr_len, icmp_len, udp_hdr) < 0)
                    continue;

                // std::cout << "got udp packet w/ src port " << ntohs(udp_hdr->uh_sport) << " vs. " << snd_src_port
                //     << " dst port " << ntohs(udp_hdr->uh_dport) << " vs. " << DST_PORT + snd_seq << std::endl;

                // if the protocol, src and dst port checks pass, we're in the 
                // presence of a TTL_EXCEEDED reply
                if (udp_hdr->uh_sport == htons(snd_src_port) &&
                    udp_hdr->uh_dport == htons(DST_PORT + snd_seq)) {

                    return_code = TTL_EXCEEDED_REPLY;
                    break;
                }
            }

        } else if (icmp_hdr->icmp_type == ICMP_UNREACH 
            || icmp_hdr->icmp_type == ICMP_ECHOREPLY) {

            if (icmp_echo_probe) {
 
                // since we don't have access to port numbers w/ icmp packets, 
                // we validate the reply based on the payload fields
                struct snd_record * snd_rec = (struct snd_record *) icmp_hdr->icmp_data;          

                if ((snd_rec->snd_seq == snd_seq) && (snd_rec->snd_ttl == snd_ttl)) {

                    return_code = HOSTNAME_HIT_REPLY;
                    break;
                }

            } else {

                if (ICMPUtils::get_updhdr_from_icmp(rcv_buff, ipv4_hdr_len, icmp_len, udp_hdr) < 0)
                    continue;

                // if the protocol, src and dst port checks pass, we're in the 
                // presence of a TTL_EXCEEDED reply
                if (udp_hdr->uh_sport == htons(snd_src_port) &&
                    udp_hdr->uh_dport == htons(DST_PORT + snd_seq)) {

                    if (icmp_hdr->icmp_code == ICMP_UNREACH_PORT)
                        return_code = HOSTNAME_HIT_REPLY;
                    else
                        return_code = icmp_hdr->icmp_code;

                    break;
                }
            }
        }

        // if this is an icmp packet but not of the indented type, post the contents 
        // anyway...
        std::cerr << "traceroute::get_icmp_response() : [WARNING] not an expected "\
            << "ICMP reply. processing anyway..." << std::endl;

        std::cout << "got " << icmp_len << " bytes from " 
            << inet_ntoa(ipv4_hdr->ip_src)
            << " : type = " << (uint16_t) icmp_hdr->icmp_type 
            << ", code = " << (uint16_t) icmp_hdr->icmp_code << std::endl;    
    }    

    // important: don't leave the alarm running
    alarm(0);
    gettimeofday(rcv_timestamp, NULL);

    return return_code;
}

// here's how traceroute's works:  
//  -# send udp datagrams to hostname, with a progressively large ip header 
//     ttl value (starting at 1)
//  -# receive icmp TIMXCEED or TIMXCEED_INTRANS from intermediate routers, 
//     which include the originating udp datagram in its icmp payload.
//     this happens because the ttl field reaches 0 at the router.
//  -# some routers may not send back icmp messages. therefore we cannot 
//     wait indefinitely for an answer. traceroute does the following:
//      - for every udp packet with ttl = t, wait for 3 seconds for an 
//        icmp reply, retry 2 times (so 3 udp messages are sent per ttl). 
//      - if no icmp reply arrives, increase the ttl, restart the procedure
//  -# the process ends when an icmp UNREACH message is received, indicating 
//     the udp packet got to hosname and hit a port which isn't used. to 
//     maximize the chances of hitting an unused port, we set the dst port 
//     to a diff. random value at each of the 3 retries.
int main (int argc, char ** argv) {

    char hostname[MAX_STRING_SIZE] = "";
    bool use_icmp_echo = false;

    ArgvParser * arg_parser = create_argv_parser();
    int parse_result = arg_parser->parse(argc, argv);

    if (parse_result != ArgvParser::NoParserError) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        std::cerr << "traceroute::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        return -1;

    } else if (parse_result == ArgvParser::ParserHelpRequested) {

        delete arg_parser;
        return -1;

    } else {

        if (arg_parser->foundOption(OPTION_HOSTNAME))
            strncpy(hostname, (char *) arg_parser->optionValue(OPTION_HOSTNAME).c_str(), MAX_STRING_SIZE);

        if (arg_parser->foundOption(OPTION_USE_PING))
            use_icmp_echo = true;
    }

    delete arg_parser;

    int rc = 0, rcv_sckt_fd = 0, snd_sckt_fd = 0;
    // since we can either send udp or icmp packets as probes, we keep 
    // placeholders for the socket type and protocol. by default we send 
    // udp packets.
    int snd_sckt_type = SOCK_DGRAM, snd_sckt_proto = 0;
    // buffer to hold payload of udp packets (snd messages)
    char snd_buff[MAX_BUFFER_SIZE];
    int snd_buff_len = 0;
    // in case icmp echos are used as probes, we need to build an icmp packet
    struct icmp * icmp_pckt = NULL;
    // also, we bind the sending socket to a particular src port, so that 
    // we can 'authenticate' icmp replies by looking into the udp header of 
    // the icmp reply payload
    uint16_t snd_src_port = 0;
    struct sockaddr rcv_addr;
    struct sockaddr last_rcv_addr;
    // NOTE: i'm still not sure how this arg works in socket API calls
    socklen_t addrlen = sizeof(struct sockaddr_in); 
    // the records sent in the payload of probes and read from replies
    struct snd_record * snd_rec;
    // addrinfo structs for hostname-to-ipv4 translation via getaddrinfo()
    struct addrinfo hints, * answer;

    // if we're using icmp echos as probes, change the default values of 
    // the socket type and protocol
    if (use_icmp_echo) {

        std::cout << "traceroute::main() : [INFO] using icmp echo" << std::endl;

        snd_sckt_type = SOCK_RAW;
        snd_sckt_proto = IPPROTO_ICMP;
    }

    // create the probe socket
    if ((snd_sckt_fd = socket(AF_INET, snd_sckt_type, snd_sckt_proto)) < 0) {

        std::cerr << "traceroute::main() : [ERROR] error opening send "\
            "socket: " << strerror(errno) << std::endl;

        return -1;
    }

    // we set snd_src_port to traceroute's process pid and bind() 
    // snd_sckt_fd to it
    struct sockaddr_in src_addr;
    src_addr.sin_family = AF_INET;
    src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    snd_src_port = (getpid() & 0xFFFF) | 0x8000;
    src_addr.sin_port = htons(snd_src_port);

    if (bind(snd_sckt_fd, (struct sockaddr *) &src_addr, sizeof(src_addr)) < 0) {

        std::cerr << "traceroute::main() : [ERROR] error binding socket to "\
            "port (udp) " << strerror(errno) << std::endl;

        return -1;
    }

    // the reply socket (for icmp replies)
    if ((rcv_sckt_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {

        std::cerr << "traceroute::main() : [ERROR] error opening send (raw, icmp) "\
            "socket: " << strerror(errno) << std::endl;

        return -1;
    }

    // following the lead of Steven's unp book, setuid(getuid()) gives up the 
    // superuser privileges necessary to create RAW sockets. it is a good 
    // practice to give up on superuser privileges as soon as these are not 
    // necessary.
    setuid(getuid());

    // given the target hostname (e.g. google.com), extract its ip address 
    // via getaddrinfo().
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // we're interested in ipv4 
    hints.ai_socktype = SOCK_STREAM;    // tcp (?)

    if ((rc = getaddrinfo(hostname, SERVICE_HTTP, &hints, &answer)) != 0) {

        // gai_error() translates a getaddrinfo() error code into 'english'
        std::cerr << "traceroute::main() : [ERROR] error while getting address "\
            "of " << hostname << " (" << gai_strerror(rc) << ")" << std::endl;
    }

    // understand what's going on here? we want to translate a raw bit 
    // representation of an ipv4 addr to its 'dotted-decimal' representation. 
    // to do so, we use inet_ntoa(), which takes a sockaddr_in * as arg. 
    // we get sockaddr_in * from the struct addrinfo * provided by 
    // getaddrinfo(). struct addrinfo has one sockaddr * attribute, which can 
    // be typecast to sockaddr_in * in this case, since we're dealing with 
    // AF_INET family addresses.
    std::cout << "traceroute::main() : [INFO] " << hostname << " translated to IPv4 addr "\
         << inet_ntoa(((struct sockaddr_in *) answer->ai_addr)->sin_addr) << std::endl;

    // unlike the ping example, we nest the 'recv code' within the 'sending 
    // loop' code. this is because the parameters of the sent packets must be 
    // changed according to the recv outcome.
    int snd_seq = 0, icmp_rc = 0;         
    bool done = false;
    struct timeval rcv_timestamp;
    // a C++11 lambda expression to convert struct timeval to msecs
    auto to_msec = [] (struct timeval tv) { return (tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0); };

    // set the SIGALRM signal handler
    SignalHandler signal_handler;

    try {

        signal_handler.set_signal_handler(SIGALRM);

    } catch (SignalException& e) {

        std::cerr << "traceroute::main() : [ERROR] SignalException: " 
            << e.what() << ". SIGALRM handling won't work. aborting." << std::endl;

        return -1;
    }

    for (int ttl = 1; ttl <= MAX_TTL && !(done); ttl++) {

        // we can use setsockopt() to set the ttl value in the outgoing udp 
        // datagrams. the option to set is IP_TTL. the setsockopt() interface 
        // goes as follows:
        //  -# fd of socket to set option
        //  -# the protocol level at which the option resides. in our case, the 
        //     ttl field is in the ipv4 header, so IPPROTO_IP
        //  -# option name, IP_TTL
        //  -# the option value (void *). here we leave a int * (&ttl)
        //  -# size of the option : sizeof(int)
        if (setsockopt(snd_sckt_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int)) < 0) {

            std::cerr << "traceroute::main() : [ERROR] error setting ttl: "
                << strerror(errno) << std::endl;

            return -1;            
        }

        // clear the last_rcv_addr for a new ttl
        bzero(&last_rcv_addr, sizeof(struct sockaddr_in));

        // the displayed lines should start w/ the sending ttl
        std::cout << std::setw(log(MAX_TTL)) << ttl;

        for (int retries = NUM_RETRIES; retries > 0; retries--) {

            // depending on the type of packet to send, we set the contents of 
            // snd_buff differently
            if (use_icmp_echo) {

                // this step seems important to achieve a correct checksum
                memset(snd_buff, 0x00, 8 + ICMP_DATA_LEN);

                // if an icmp echo, we first build add an icmp header (8 byte) 
                // and then the 56 byte optional playload, more than enough 
                // to accommodate a struct snd_record (6 byte)
                icmp_pckt = ICMPUtils::prepare_icmp_pckt(snd_buff, ICMP_ECHO, 0);

                // we set the identifier field of the icmp message as the 
                // calling process pid
                icmp_pckt->icmp_id = htons(getpid() & 0xFFFF);
                // the icmp message is started w/ a seq number of 0, it will be 
                // increased as needed in subsequent uses of the icmp struct
                icmp_pckt->icmp_seq = htons((uint16_t) ++snd_seq);

                // 8 bytes for icmp header + ICMP_DATA_LEN (snd_record + 
                // 'stuffing')
                snd_buff_len = 8 + ICMP_DATA_LEN;

                // the snd_record struct should start after the icmp header, 
                // i.e. at snd_buff + 8 byte
                snd_rec = (struct snd_record *) (snd_buff + 8);

                // to match icmp reply to sent packets (either icmp or udp), we 
                // set the following info on the payload:
                //  -# sequence number
                //  -# ttl packet left with
                //  -# time at which packet left (struct timeval)
                // this is kept in a struct (struct snd_record), defined above
                snd_rec->snd_seq = snd_seq;
                snd_rec->snd_ttl = ttl;
                gettimeofday(&(snd_rec->snd_timestamp), NULL);

                // icmp packets carry a 2 byte checksum, which is calculated 
                // over its entire length
                icmp_pckt->icmp_cksum = ICMPUtils::in_cksum((u_short *) icmp_pckt, snd_buff_len);

                //print_icmp_hdr(icmp_pckt);

            } else {

                // if an udp packet, snd_rec is set at the starting address 
                // of the snd_buff char *
                snd_rec = (struct snd_record *) snd_buff;
                snd_rec->snd_seq = ++snd_seq;
                snd_rec->snd_ttl = ttl;
                gettimeofday(&(snd_rec->snd_timestamp), NULL);

                snd_buff_len = sizeof(struct snd_record);

                // set the port of the outgoing udp packet to a diff. value than 
                // before. we alter the struct sockaddr snd_addr to accomplish 
                // that, first by typecasting the general struct to a AF_INET 
                // sockaddr_in. the sin_port attribute must respect network byte 
                // ordering.
                ((struct sockaddr_in *) answer->ai_addr)->sin_port = htons(DST_PORT + snd_seq);
            }

            // send the upd packet
            if (sendto(snd_sckt_fd, snd_buff, snd_buff_len, 0, answer->ai_addr, answer->ai_addrlen) < 0) {

                std::cerr << "traceroute::main() : [ERROR] error sending packet: "
                    << strerror(errno) << std::endl;                
            }

            // now we call get_icmp_response() and handle it differently 
            // according to the return code. if icmp echos are sent as probes, 
            // one must extract an icmp packet from within the icmp reply 
            // payload, not a udp packet.

            // NOTE: this call looks horrible: so many arguments... i guess it's 
            // cleaner to use structs or C++ OOP instead.
            if ((icmp_rc = get_icmp_response(
                    rcv_sckt_fd,
                    &rcv_addr,
                    &addrlen,
                    ttl,
                    snd_seq,
                    snd_src_port,
                    use_icmp_echo, 
                    signal_handler,
                    &rcv_timestamp)) == TIMEOUT_REPLY) {

                std::cout << " ?";

            } else {

                // ok, we got something... 

                // if for some reason the ip address 
                // of the icmp reply changes when compared to the previous, 
                // we print a <hostname> (if possible) (<ipv4 address>) 
                // message. we use memcmp() for that, which compares the first 
                // n bytes of 2 const void *.
                if (memcmp(
                    &last_rcv_addr, 
                    &rcv_addr, 
                    sizeof(struct sockaddr_in)) != 0) {

                    char reply_hostname[MAX_STRING_SIZE] = "";

                    // getnameinfo() has a 0 success return code
                    if (getnameinfo(
                            &rcv_addr, addrlen, 
                            reply_hostname, sizeof(reply_hostname),
                            NULL, 0, 0) == 0) {

                        std::cout << " " << inet_ntoa(((struct sockaddr_in *) &rcv_addr)->sin_addr) 
                            << " (" << reply_hostname << ")";
                    } else {
                        std::cout << " " << inet_ntoa(((struct sockaddr_in *) &rcv_addr)->sin_addr);                        
                    }

                    // keep track of the rcv_addr which was received last
                    memcpy(&last_rcv_addr, &rcv_addr, sizeof(struct sockaddr_in));
                }

                // print the rtt of the snd udp > rcv icmp cycle
                std::cout << " " << to_msec(*(tv_sub(&rcv_timestamp, &(snd_rec->snd_timestamp)))) << " msec";

                if (icmp_rc == HOSTNAME_HIT_REPLY) {
                    done = true;
                } else if (icmp_rc >= 0) {
                    std::cout << " unknown icmp code (" << icmp_rc << ")";
                }
            }
        }

        std::cout << std::endl;
    }

    // all the storage returned by getaddrinfo() are allocated dynamically 
    // (i.e. w/ malloc()). so one must free it w/ freeaddrinfo()
    freeaddrinfo(answer);

    return 0;
}