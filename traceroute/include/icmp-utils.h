#ifndef ICMP_UTILS_H
#define ICMP_UTILS_H

#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <iostream>
#include <iomanip>

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>        // struct udphdr

// Steven's unp book sets the icmp's ECHO optional data field (i.e. after the 
// 8 byte icmp header) to 56 bytes, which yields a 84 byte ipv4 datagram. after 
// checking what the real traceroute does (and to avoid fragmentation), we set 
// it to 32 byte, yielding a 60 byte ipv4 diagram:
//  -# 20 byte ipv4 header
//  -# 8 byte icmp header
//  -# 32 byte for icmp optional data (we only use 8 byte for a timeval struct)
#define ICMP_DATA_LEN   32

class ICMPUtils {

    public:

        ICMPUtils() {}
        ~ICMPUtils() {}

        static int get_inner_ip_hdr(
            char * icmp_pckt,               // array of icmp packet bytes
            int icmp_pckt_len,              // length of icmp packet
            struct ip * & inner_ip_hdr);

        static int get_inner_icmp_hdr(
            char * icmp_pckt, 
            int icmp_pckt_len,
            struct icmp * & in_icmp_hdr);

        static int get_inner_udp_hdr(
            char * icmp_pckt, 
            int icmp_pckt_len,
            struct udphdr * & inner_udp_hdr);

        static uint16_t in_cksum(uint16_t * addr, int len);

        static struct icmp * prepare_icmp_pckt(
            char * buffer, 
            uint8_t type, 
            uint8_t code);

        static void print_icmp_hdr(struct icmp * icmp_pckt);
};

#endif