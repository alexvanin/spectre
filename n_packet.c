#include "n_packet.h"

#include <malloc.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/ether.h>

unsigned short s_csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for(sum=0; nwords>0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

unsigned short l_csum(struct iphdr* iph, void* buf, int buflen)
{
	char data[4096];
	struct pseudo_hdr *phdr = (struct pseudo_hdr *)data; 
	
	phdr->saddr = iph->saddr;
	phdr->daddr = iph->daddr;
        phdr->zeros = 0;
        phdr->protocol = iph->protocol;
        phdr->tot_len = htons(buflen);

	memcpy(data+sizeof(struct pseudo_hdr), buf, buflen);

	return s_csum((unsigned short*)data, (buflen+sizeof(struct pseudo_hdr))/2);
}

uint16_t calc_ip_size(uint16_t p_size)
{
        return p_size - sizeof(struct ether_header);
}

uint16_t calc_ip_csum(char* p_buf)
{
        return s_csum((unsigned short *)(p_buf+sizeof(struct ether_header)), sizeof(struct iphdr)/2);
}

uint16_t calc_udp_size(uint16_t p_size)
{
        return p_size - sizeof(struct ether_header) - sizeof(struct iphdr);
}

uint16_t calc_tr_csum(char* p_buf, uint16_t p_size)
{
        struct iphdr *iph = (struct iphdr *) (p_buf + sizeof(struct ether_header));

        return l_csum(iph, p_buf+sizeof(struct ether_header)+sizeof(struct iphdr),
                        p_size - sizeof(struct ether_header) - sizeof(struct iphdr));
}
