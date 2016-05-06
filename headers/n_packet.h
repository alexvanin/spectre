#ifndef NPACK_H
#define NPACK_H

#include <stdint.h>
#include <netinet/ip.h>
#include <netinet/in.h>

#define DEFAULT_INTERFACE "enp0s3"

#define MY_DEST_MAC0	0x01
#define MY_DEST_MAC1	0x01
#define MY_DEST_MAC2	0x01
#define MY_DEST_MAC3	0x01
#define MY_DEST_MAC4	0x01
#define MY_DEST_MAC5	0x01

#define MY_DEST_IP	"192.168.0.111"
#define MY_SRC_PORT	2134
#define MY_DEST_PORT	3456
#define MY_SEQ_NUMBER 	4253

#define BUF_SIZ		2048

struct sockaddr_ll
{
	unsigned short	sll_family;
	unsigned short	sll_protocol;
	int		sll_ifindex;
	unsigned short	sll_hatype;
	unsigned char	sll_pkttype;
	unsigned char	sll_halen;
	unsigned char	sll_addr[8];
};

struct pseudo_hdr
{
	uint32_t saddr;
	uint32_t daddr;
	uint8_t  zeros;	
	uint8_t  protocol;	
	uint16_t tot_len;	
};

typedef enum
{
	TCP = IPPROTO_TCP,
	UDP = IPPROTO_UDP
} protocol;

unsigned short s_csum(unsigned short*, int);
unsigned short l_csum(struct iphdr*, void*, int);
uint16_t calc_ip_size(uint16_t);
uint16_t calc_ip_csum(char*);
uint16_t calc_udp_size(uint16_t);
uint16_t calc_tr_csum(char*, uint16_t);

#endif
