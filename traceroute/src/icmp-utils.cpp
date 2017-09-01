#include "icmp-utils.h"

uint16_t ICMPUtils::in_cksum(uint16_t * addr, int len) {
    int nleft = len;
    int sum = 0;
    uint16_t * w = addr;
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

int ICMPUtils::get_inner_ip_hdr(
    char * icmp_pckt,               // array of icmp packet bytes
    int icmp_pckt_len,              // length of icmp packet
    struct ip * & inner_ip_hdr) {

    // at least icmp header + an ip header within its payload
    if (icmp_pckt_len < 8 + (int) sizeof(struct ip)) {

        std::cerr << "icmp-utils::get_inner_ip_hdr() : [ERROR] malformed ICMP "\
            "reply. payload too short to be meaningful (" 
            << icmp_pckt_len << " byte). skip processing." << std::endl;

        return -1;            
    }

    // inner ip header should be after icmp header : 8 byte in the icmp packet
    inner_ip_hdr = (struct ip *) (icmp_pckt + 8);

    return 0;
}

int ICMPUtils::get_inner_icmp_hdr(
    char * icmp_pckt, 
    int icmp_pckt_len,
    struct icmp * & inner_icmp_hdr) {

    if (icmp_pckt_len < 8 + (int) sizeof(struct ip)) {

        std::cerr << "icmp-utils::get_inner_icmp_hdr() : [ERROR] malformed ICMP "\
            "reply. payload too short to be meaningful (" 
            << icmp_pckt_len << " byte). skip processing." << std::endl;

        return -1;
    }

    // let's now look at the ip header embedded in the icmp reply
    struct ip * inner_ipv4_hdr = NULL;
    if (get_inner_ip_hdr(icmp_pckt, icmp_pckt_len, inner_ipv4_hdr) < 0)
        return -1;
    int inner_ipv4_hdr_len = (inner_ipv4_hdr->ip_hl << 2);

    if (inner_ipv4_hdr->ip_p != IPPROTO_ICMP) {

        std::cerr << "icmp-utils::get_inner_icmp_hdr() : [ERROR] not "\
            "an icmp packet (proto code = " << inner_ipv4_hdr->ip_p << "). skip "\
            "processing." << std::endl;

        return -1; 
    }

    // we offset the pckt_buff starting address with the length of all the 
    // headers in between, till the start of the icmp header
    inner_icmp_hdr = (struct icmp *) (icmp_pckt + 8 + inner_ipv4_hdr_len);
    // the icmp_len should be at least 8 byte (size of icmp header). 
    // if not, abort.
    int inner_icmp_pckt_len = icmp_pckt_len - (8 + inner_ipv4_hdr_len);

    if (inner_icmp_pckt_len < 8) {

        std::cerr << "icmp-utils::get_inner_icmp_hdr() : [ERROR] malformed ICMP "\
            "packet. header too short (" << inner_icmp_pckt_len << " byte). not "\
            "processing." << std::endl;  

        return -1;              
    }

    // return a positive int if inner icmp packet isn't an icmp ECHO reply
    if (inner_icmp_hdr->icmp_type == ICMP_ECHOREPLY) {

        if (inner_icmp_pckt_len < 16) {

            std::cerr << "icmp-utils::get_inner_icmp_hdr() : [ERROR] malformed ICMP "\
                "echo reply. payload too short to be meaningful (" 
                << inner_icmp_pckt_len << " byte). not processing." << std::endl;  

            return -1;            
        }

    } else {

        return (inner_icmp_hdr->icmp_type);
    }

    return 0;
}

int ICMPUtils::get_inner_udp_hdr(
    char * icmp_pckt, 
    int icmp_pckt_len,
    struct udphdr * & inner_udp_hdr) {

    if (icmp_pckt_len < 8 + (int) sizeof(struct ip)) {

        std::cerr << "icmp-utils::get_inner_udp_hdr() : [ERROR] malformed ICMP "\
            "reply. payload too short to be meaningful (" 
            << icmp_pckt_len << " byte). skip processing." << std::endl;  

        return -1;            
    }

    struct ip * inner_ipv4_hdr = NULL;
    if (get_inner_ip_hdr(icmp_pckt, icmp_pckt_len, inner_ipv4_hdr) < 0)
        return -1;
    int inner_ipv4_hdr_len = (inner_ipv4_hdr->ip_hl << 2);

    if (icmp_pckt_len < 8 + inner_ipv4_hdr_len + 4) {

        std::cerr << "icmp-utils::get_inner_udp_hdr() : [ERROR] not "\
            "enough data to look at udp ports (" << icmp_pckt_len << " byte). skip "\
            "processing." << std::endl;  

        return -1;              
    }

    if (inner_ipv4_hdr->ip_p != IPPROTO_UDP) {

        std::cerr << "icmp-utils::get_inner_udp_hdr() : [ERROR] not "\
            "and udp packet (proto code = " << inner_ipv4_hdr->ip_p << "). skip "\
            "processing." << std::endl;  

        return -1;                      
    }

    inner_udp_hdr = (struct udphdr *) (icmp_pckt + 8 + inner_ipv4_hdr_len);

    return 0;
}

struct icmp * ICMPUtils::prepare_icmp_pckt(
    char * buffer,
    uint8_t type, 
    uint8_t code) {

    // we allocate memory with a char[] (passed as arg), and then use the memory 
    // block for the struct icmp *
    struct icmp * icmp_pckt = (struct icmp *) buffer;

    // the <type, code> tuple identifies the icmp message purpose
    icmp_pckt->icmp_type = type;
    icmp_pckt->icmp_code = code;

    // following Steven's UNP book, we fill the data portion with '0XA5', then 
    // with data
    memset(icmp_pckt->icmp_data, 0xA5, ICMP_DATA_LEN);

    return icmp_pckt;
}

void ICMPUtils::print_icmp_hdr(struct icmp * icmp_pckt) {

    std::cout << std::endl << "icmp-utils::print_icmp_hdr() : [INFO] icmp header fields :" << std::endl;
    std::cout << "\ticmp_type = " << std::hex << (uint16_t) icmp_pckt->icmp_type 
        << " icmp_code = " << std::hex << (uint16_t) icmp_pckt->icmp_code 
        << " icmp_cksum = " << std::hex << (uint16_t) icmp_pckt->icmp_cksum 
        << std::endl;
    std::cout << "\ticmp_id = " << std::hex << (uint16_t) icmp_pckt->icmp_id 
        << " icmp_seq = " << std::hex << (uint16_t) icmp_pckt->icmp_seq 
        << std::endl;
}