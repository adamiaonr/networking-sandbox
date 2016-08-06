#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include <iostream>
#include <thread>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netdb.h>              // getaddrinfo()

#include "argvparser.h"

// Steven's unp book sets the icmp's ECHO optional data field (i.e. after the 
// 8 byte icmp header) to 56 bytes, which yields a 84 byte ipv4 datagram:
//  -# 20 byte ipv4 header
//  -# 8 byte icmp header
//  -# 56 byte for icmp optional data (we only use 8 byte for a timeval struct)
#define ICMP_DATA_LEN   56
#define SERVICE_HTTP    "http"
// as defined in Steven's unp book, fig. 28.4
#define MAX_BUFFER_SIZE 1500
#define MAX_STRING_SIZE 256

#define OPTION_HOSTNAME     (char *) "hostname"

using namespace CommandLineProcessing;

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\npingy (yet another ping)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_HOSTNAME,
            "hostname to ping",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

struct icmp * prepare_icmp_pckt(
    uint8_t type, 
    uint8_t code) {

    // we allocate memory with a char[] of size 1500, and then use the memory 
    // block for the struct icmp *
    char * icmp_raw = (char *) calloc(MAX_BUFFER_SIZE, sizeof(char));
    struct icmp * icmp_pckt = (struct icmp *) icmp_raw;

    // the <type, code> tuple identifies the icmp message purpose
    icmp_pckt->icmp_type = type;
    icmp_pckt->icmp_code = code;
    // we set the identifier field of the icmp message as the calling process 
    // pid
    icmp_pckt->icmp_id = (getpid() & 0xFFFF);
    // the icmp message is started w/ a seq number of 0, it will be increased 
    // as needed in subsequent uses of the icmp struct
    icmp_pckt->icmp_seq = 0;    
    // following Steven's UNP book, we fill the data portion with '0XA5', then 
    // with data
    memset(icmp_pckt->icmp_data, 0xA5, ICMP_DATA_LEN);

    return icmp_pckt;
}

uint16_t in_cksum(uint16_t * addr, int len) {
    int nleft = len;
    int sum = 0;
    uint16_t *w = addr;
    uint16_t answer = 0;

    /*
     * the algorithm is simple: using a 32 bit accumulator (sum), we add
     * sequential 16 bit words to it, and at the end, fold back all the
     * carry bits from the top 16 bits into the lower 16 bits.
     */
    while (nleft > 1)  {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if (nleft == 1) {
        *(unsigned char *) (&answer) = *(unsigned char *) w ;
        sum += answer;
    }

        /* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);         /* add carry */
    answer = ~sum;              /* truncate to 16 bits */
    
    return answer;
}

void send_icmp_echo(
    int interval,
    int socket_fd,
    struct icmp * icmp_pckt,
    struct sockaddr * dst_addr,
    socklen_t dst_addrlen) {

    // 56 byte of optional data + 8 byte icmp header
    int icmp_data_len = ICMP_DATA_LEN + 8;
    icmp_pckt->icmp_seq = 0;

    while (1) {

        // fill icmp_pct->icmp_data (payload) with the current timestamp. note 
        // how we just use the raw bytes of icmp_pckt->icmp_data.
        gettimeofday((struct timeval *) icmp_pckt->icmp_data, NULL); 

        // icmp packet checksum over the whole of its 64 byte
        icmp_pckt->icmp_cksum = 0;
        icmp_pckt->icmp_cksum = in_cksum((u_short *) icmp_pckt, icmp_data_len);

        sendto(
            socket_fd, 
            icmp_pckt, icmp_data_len,
            0,
            dst_addr, dst_addrlen);

        sleep(interval);

        // increment the sequence nr.
        icmp_pckt->icmp_seq++;
    }
}

void tv_sub(struct timeval * out, struct timeval * in) {

    if ((out->tv_usec -= in->tv_usec) < 0) {   /* out -= in */

        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    
    out->tv_sec -= in->tv_sec;
}

int proccess_icmp_ipv4_reply(
    int recv_bytes, 
    struct msghdr * msg, 
    struct timeval * rcv_timestamp) {

    // fetch the recv payload through the iovec of msg 
    char aux[MAX_STRING_SIZE] = "";
    char * recv_buffer = (char *) msg->msg_iov->iov_base;
    // we'll need to read both ipv4 and icmp headers
    struct ip * ipv4_hdr;
    struct icmp * icmp_hdr;

    // a nice chance to start using C++11 lambda expressions. to debug the 
    // ip_ttl problem, we define a lambda expression which returns the hex 
    // version of a byte, in string format
    auto to_hex_str = [] (uint8_t nr, char * str) { snprintf(str, MAX_STRING_SIZE, "%04X", nr); return str; };

    // we start by reading the ipv4 header. this is as simple as typecasting 
    // the byte payload into a struct ip *
    ipv4_hdr = (struct ip *) recv_buffer;
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

        std::cerr << "pingy::proccess_icmp_ipv4_reply() : [ERROR] not an ICMP "\
            "packet. not processing." << std::endl;        

        return -1;
    }

    // with the ipv4 header length, we now know the start of the icmp 
    // header
    icmp_hdr = (struct icmp *) (recv_buffer + ipv4_hdr_len);
    // the icmp_len should be at least 8 byte (size of icmp header). 
    // if not, abort.
    int icmp_len = 0;
    if ((icmp_len = recv_bytes - ipv4_hdr_len) < 8) {

        std::cerr << "pingy::proccess_icmp_ipv4_reply() : [ERROR] malformed ICMP "\
            "packet. header too short (" << icmp_len << "byte). not "\
            "processing." << std::endl;  

        return -1;              
    }

    if (icmp_hdr->icmp_type == ICMP_ECHOREPLY) {

        if (icmp_len < 16) {

            std::cerr << "pingy::proccess_icmp_ipv4_reply() : [ERROR] malformed ICMP "\
                "echo reply. payload too short to be meaningful (" 
                << icmp_len << "byte). not processing." << std::endl;  

            return -1;            
        }

        // extract the struct timeval in the echo reply. again through a simple 
        // typecast (which seems pretty convenient)
        struct timeval * snd_timestamp = (struct timeval *) icmp_hdr->icmp_data;
        tv_sub(rcv_timestamp, snd_timestamp);
        double rtt = rcv_timestamp->tv_sec * 1000.0 + rcv_timestamp->tv_usec / 1000.0;

        // believe it or not, one of the most complicated parts of unix network 
        // programming is translation between the raw bit representations of 
        // ip addresses and their 'dot decimal' strings...

        // in this case, our goal is to extract the src ipv4 address from 
        // the ipv4 header. this is quick : get the ip_src attribute of the 
        // struct ip is an in_addr, ready to be fed to inet_ntoa(), which 
        // in turn returns a dotted-decimal C string.
        std::cout << "got " << icmp_len << " bytes from " 
            << inet_ntoa(ipv4_hdr->ip_src) 
                // to print the canonical name of the host w/ ipv4 address 
                // ipv4_hdr->ip_src, we use gethostbyaddr(). this returns a 
                // srtuct hostent *, which has char * attribute with this 
                // cname. one curious thing: the 1st arg of gethostbyaddr() is 
                // listed as a char *, but should be a struct in_addr * instead.
                << " (" << gethostbyaddr(&(ipv4_hdr->ip_src), sizeof(struct in_addr), AF_INET)->h_name << ")"
            << " : icmp_seq = " << icmp_hdr->icmp_seq 
            << ", ttl = " << (uint16_t) ipv4_hdr->ip_ttl << " (" << to_hex_str(ipv4_hdr->ip_ttl, aux) << ")" 
            << ", rtt = " << rtt << " ms" << std::endl; 
 
    } else {

        // if this is an icmp packet but not an ECHO_REPLY, post the contents 
        // anyway...
        std::cerr << "pingy::proccess_icmp_ipv4_reply() : [WARNING] not ICMP ECHO REPLY. "\
            << "processing anyway..." << std::endl;

        std::cout << "got " << icmp_len << " bytes from " 
            << inet_ntoa(((struct sockaddr_in *) msg->msg_name)->sin_addr)
            << " : type = " << icmp_hdr->icmp_type 
            << ", code = " << icmp_hdr->icmp_code << std::endl;         
    }

    return 0;
}

int main (int argc, char ** argv) {

    char hostname[MAX_STRING_SIZE] = "";

    ArgvParser * arg_parser = create_argv_parser();
    int parse_result = arg_parser->parse(argc, argv);

    if (parse_result != ArgvParser::NoParserError) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        std::cerr << "pingy::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        return -1;

    } else if (parse_result == ArgvParser::ParserHelpRequested) {

        delete arg_parser;
        return -1;

    } else {

        if (arg_parser->foundOption(OPTION_HOSTNAME))
            strncpy(hostname, (char *) arg_parser->optionValue(OPTION_HOSTNAME).c_str(), MAX_STRING_SIZE);
    }

    delete arg_parser;

    int raw_sckt_fd = 0, recv_bytes = 0, rc = 0;
    // icmp pckt to send
    struct icmp * icmp_pckt = NULL;
    // structs used by the recvmsg() function. msghdr has an iovec attribute. 
    // iovec specifies a 'vector' of bytes in memory: its attributes are a base 
    // memory address and a number of bytes that follow.
    struct iovec recv_iovec;
    struct msghdr recv_msg;
    char recv_buffer[MAX_BUFFER_SIZE];
    // msghdr has a ctrl buffer for ancillary information
    char ctrl_buffer[MAX_BUFFER_SIZE];
    // to hold the source address supplied to recvmsg()
    struct sockaddr recv_addr;
    socklen_t recv_addr_len = 0; 
    // addrinfo structs for hostname-to-ipv4 translation via getaddrinfo()
    struct addrinfo hints, * answer;

    raw_sckt_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    // following the lead of Steven's UNP, setuid(getuid()) gives up the 
    // superuser privileges necessary to create RAW sockets. it is a good 
    // practice to give up on superuser privileges as soon as these are not 
    // necessary.
    setuid(getuid());

    // given the target hostname (e.g. google.com), extract its ip address 
    // via getaddrinfo().
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // we're interested in ipv4 
    hints.ai_socktype = SOCK_STREAM;    // tcp

    if ((rc = getaddrinfo(hostname, SERVICE_HTTP, &hints, &answer)) != 0) {

        // gai_error() translates a getaddrinfo() error code into 'english'
        std::cerr << "pingy::main() : [ERROR] error while getting address "\
            "of " << hostname << " (" << gai_strerror(rc) << ")" << std::endl;
    }

    // understand what's going on here? we want to translate a raw bit 
    // representation of an ipv4 addr to its 'dotted-decimal' representation. 
    // to do so, we use inet_ntoa(), which takes a sockaddr_in * as arg. 
    // we get sockaddr_in * from the struct addrinfo * provided by 
    // getaddrinfo(). struct addrinfo has one sockaddr * attribute, which can 
    // be typecast to sockaddr_in * in this case, since we're dealing with 
    // AF_INET family addresses.
    std::cout << "pingy::main() : [INFO] " << hostname << " translated to IPv4 addr "\
         << inet_ntoa(((struct sockaddr_in *) answer->ai_addr)->sin_addr) << std::endl;

    // prepare the base icmp ECHO packet for sending
    icmp_pckt = prepare_icmp_pckt(ICMP_ECHO, 0);

    // start sending ping requests to hostname, using C++11's threads
    std::thread icmp_msg_sender(
        send_icmp_echo,     // the function to be called by the thread
        1,                  // std::thread() accepts as many args as you want! 
        raw_sckt_fd,
        icmp_pckt,
        answer->ai_addr,
        answer->ai_addrlen);

    // ECHO responses will start coming back now. initialize recv_msg and 
    // recv_iovec structs:
    //  -# iovec's base addr & size are initialized to recv_buffer
    //  -# msg_name (which is void *) is set to the generic sockaddr pointer 
    //     created to point to the remote peer's address  
    recv_iovec.iov_base = recv_buffer;
    recv_iovec.iov_len = sizeof(recv_buffer);

    recv_msg.msg_name = &recv_addr;
    recv_msg.msg_iov = &recv_iovec;
    recv_msg.msg_iovlen = 1;
    recv_msg.msg_control = ctrl_buffer;

    // we take note of the reception timestamp, compare it to that carried 
    // by the ECHO's payload, which should contain the 'send time' timestamps
    struct timeval recv_timestamp;

    while (1) {

        recv_msg.msg_namelen = recv_addr_len;
        recv_msg.msg_controllen = sizeof(ctrl_buffer);

        recv_bytes = recvmsg(raw_sckt_fd, &recv_msg, 0);

        if (recv_bytes < 0) {

            // EINTR means 'interrupted function call', i.e. an asynchronous 
            // signal occurred and prevented completion of recvmsg(). that's 
            // we hit continue and call recvmsg() again.
            if (errno == EINTR) {

                continue;

            } else {

                std::cerr << "pingy::main() : [ERROR] error in recvmsg(): " 
                    << strerror(errno) << std::endl;

                break;
            }

        } else {

            // gather the reception timestamp (now)
            gettimeofday(&recv_timestamp, NULL);
            proccess_icmp_ipv4_reply(recv_bytes, &recv_msg, &recv_timestamp);
        }
    }

    // join the icmp_msg_sender thread with the main thread
    icmp_msg_sender.join();

    // all the storage returned by getaddrinfo() are allocated dynamically 
    // (i.e. w/ malloc()). so one must free it w/ freeaddrinfo()
    freeaddrinfo(answer);

    return 0;
}