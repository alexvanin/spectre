// Made by AlexVanin

#include "n_packet.h"
#include "n_plan.h"
#include "rand_exp.h"


#include <stdio.h>  
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>

#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ether.h>

#define eexit(text) printf((text)); exit(1);

const unsigned long long nano = 1000000000;

char* init_packet(int sockfd, char* if_name, char* ip_s, uint16_t port_s, char* ip_d, uint16_t port_d, protocol p_proto,
                uint8_t eth_s[6], uint8_t eth_d[6], struct sockaddr_ll * sockaddr, uint16_t* h_size)
{
        struct ifreq if_idx;
        struct ifreq if_mac;
        struct ifreq if_ip;

        struct ether_header *eh;
        struct iphdr *iph;
        struct udphdr *udph;
        struct tcphdr *tcph;

        uint16_t tx_len = 0;

        char* p_buf = malloc(BUF_SIZ);

        eh = (struct ether_header *) p_buf;
        iph = (struct iphdr *) (p_buf + sizeof(struct ether_header));
        udph = (struct udphdr *) (p_buf + sizeof(struct iphdr) + sizeof(struct ether_header));
        tcph = (struct tcphdr *) (p_buf + sizeof(struct iphdr) + sizeof(struct ether_header));

        /* Get the index of the interface to send on */
        memset(&if_idx, 0, sizeof(struct ifreq));
        strncpy(if_idx.ifr_name, if_name, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0) {
		perror("SIOCGIFINDEX");
		exit(1);
	}
        /* Get the MAC address of the interface to send on */
        memset(&if_mac, 0, sizeof(struct ifreq));
        strncpy(if_mac.ifr_name, if_name, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0) {
		perror("SIOCGIFHWADDR");
		exit(1);
	}

        /* Get the IP address of the interface to send on */
        memset(&if_ip, 0, sizeof(struct ifreq));
        strncpy(if_ip.ifr_name, if_name, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFADDR, &if_ip) < 0) {
		perror("SIOCGIFADDR");
		exit(1);
	}

        /* Construct the Ethernet header */
        //memset(sendbuf, 0, BUF_SIZ);
        if (eth_s == NULL) {
                eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
                eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
                eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
                eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
                eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
                eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
        }
        else {
                eh->ether_shost[0] = eth_s[0];
                eh->ether_shost[1] = eth_s[1];
                eh->ether_shost[2] = eth_s[2];
                eh->ether_shost[3] = eth_s[3];
                eh->ether_shost[4] = eth_s[4];
                eh->ether_shost[5] = eth_s[5];
        }
        if (eth_d == NULL) {
                eh->ether_dhost[0] = MY_DEST_MAC0;
                eh->ether_dhost[1] = MY_DEST_MAC1;
                eh->ether_dhost[2] = MY_DEST_MAC2;
                eh->ether_dhost[3] = MY_DEST_MAC3;
                eh->ether_dhost[4] = MY_DEST_MAC4;
                eh->ether_dhost[5] = MY_DEST_MAC5;
        }
        else {
                eh->ether_dhost[0] = eth_d[0];
                eh->ether_dhost[1] = eth_d[1];
                eh->ether_dhost[2] = eth_d[2];
                eh->ether_dhost[3] = eth_d[3];
                eh->ether_dhost[4] = eth_d[4];
                eh->ether_dhost[5] = eth_d[5];
        }
        eh->ether_type = htons(ETH_P_IP);
        tx_len += sizeof(struct ether_header);

        iph->ihl = 5;
        iph->version = 4;
        iph->tos = 16; // Low delay
        iph->id = htons(54321);
        iph->ttl = IPDEFTTL; // hops

        iph->protocol = p_proto;

        if (ip_s == NULL) iph->saddr = inet_addr(inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr));
        else {
		iph->saddr = inet_addr(ip_s);
		if (iph->saddr == -1) {
			eexit("Cannot parse ipS\n");
		}
	}

        if (ip_d == NULL) iph->daddr = inet_addr(MY_DEST_IP);
        else {
		iph->daddr = inet_addr(ip_d);
		if (iph->daddr == -1) {
			eexit("Cannot parse ipD\n");
		}
	}

        tx_len += sizeof(struct iphdr);

        if (p_proto == UDP) {
                if (port_s == 0) udph->source = htons(MY_SRC_PORT);
                else udph->source = htons(port_s);

                if (port_d == 0) udph->dest = htons(MY_DEST_PORT);
                else udph->dest = htons(port_d);

                udph->check = 0;
                tx_len += sizeof(struct udphdr);
        }
        else if (p_proto == TCP) {
                if (port_s == 0)
                        tcph->source = htons(MY_SRC_PORT);
                else
                        tcph->source = htons(port_s);
                if (port_d == 0)
                        tcph->dest = htons(MY_DEST_PORT);
                else
                        tcph->dest = htons(port_d);

                tcph->seq = htons(MY_SEQ_NUMBER); // inital sequence number
                tcph->ack_seq = htons(0); // acknowledgement number
                tcph->ack = 0; // acknowledgement flag
                tcph->syn = 1; // synchronize flag
                tcph->rst = 0; // reset flag
                tcph->psh = 0; // push flag
                tcph->fin = 0; // finish flag
                tcph->urg = 0; // urgent flag
                tcph->check = 0; // tcp checksum
                tcph->doff = 5; // data offset
                tcph->res1 = 0;
                tcph->res2 = 0;

                tx_len += sizeof(struct tcphdr);
        }

        /* Index of the network device */
        sockaddr->sll_ifindex = if_idx.ifr_ifindex;
        /* Address length*/
        sockaddr->sll_halen = ETH_ALEN;
        /* Destination MAC */
        sockaddr->sll_addr[0] = MY_DEST_MAC0;
        sockaddr->sll_addr[1] = MY_DEST_MAC1;
        sockaddr->sll_addr[2] = MY_DEST_MAC2;
        sockaddr->sll_addr[3] = MY_DEST_MAC3;
        sockaddr->sll_addr[4] = MY_DEST_MAC4;
        sockaddr->sll_addr[5] = MY_DEST_MAC5;

        *h_size = tx_len;
        return p_buf;
}

int destroy_packet(char* buf) 
{
	free( buf );
	return 0;
}
	
		

int fill_dataunit(t_dataunit* d_unit,  char* p_buf, uint16_t h_size, uint16_t p_size, protocol p_proto)
{
        struct iphdr *iph = (struct iphdr *) (p_buf + sizeof(struct ether_header));
        struct udphdr *udph = (struct udphdr *) (p_buf + sizeof(struct ether_header)+sizeof(struct iphdr));

        if (h_size > p_size) { eexit("Header size is more than packet size, plaese specify right params\n") } 
        d_unit->p_size = p_size;

        d_unit->p_buf = p_buf;
        d_unit->p_proto = p_proto;

        d_unit->p_ip_size = calc_ip_size(p_size);
        iph->tot_len = htons(d_unit->p_ip_size);
        d_unit->p_ip_csum = calc_ip_csum(p_buf);

        if (p_proto == UDP)
        {
                d_unit->p_tr_size = calc_udp_size(p_size);
                udph->len = htons(d_unit->p_tr_size);
                d_unit->p_tr_csum = calc_tr_csum(p_buf, p_size);
        }
        else if (p_proto == TCP)
        {
                d_unit->p_tr_csum = calc_tr_csum(p_buf, p_size);
        }
        return 0;
}

int extract_dataunit(t_dataunit* d_unit)
{
        struct iphdr *iph = (struct iphdr*)(d_unit->p_buf + sizeof(struct ether_header));
        struct udphdr *udph = (struct udphdr*)(d_unit->p_buf + sizeof(struct ether_header)+sizeof(struct iphdr));
        struct tcphdr *tcph = (struct tcphdr*)(d_unit->p_buf + sizeof(struct ether_header)+sizeof(struct iphdr));

        iph->tot_len = htons(d_unit->p_ip_size);
        iph->check = d_unit->p_ip_csum;

        if (d_unit->p_proto == UDP) {
                udph->len = htons(d_unit->p_tr_size);
                udph->check = d_unit->p_tr_csum;
        }
        else {
                tcph->check = d_unit->p_tr_csum;
        }

        return 0;
}


t_planunit* creat_plan(char* p_buf, uint16_t h_size, t_metaunit* m_plan, uint32_t p_cnt, protocol p_proto)
{
        t_planunit* plan  = malloc(sizeof(t_planunit) * p_cnt);
        for (int i = 0; i < p_cnt; i++) {
                plan[i].time = m_plan[i].p_delay;
                fill_dataunit(&(plan[i].d_unit), p_buf, h_size, m_plan[i].p_size, p_proto);
        }
        return plan;
}


int exec_plan(t_planunit* plan, uint32_t p_cnt, int sockfd, struct sockaddr_ll* sockaddr)
{
        struct timespec tm;
        uint64_t t1, t2;
        t1 = 0; t2 = 0;
        int64_t t3;

        for (int i = 0; i < p_cnt; i++) {
                extract_dataunit(&(plan[i].d_unit));
                if (i != 0) {
                        clock_gettime( CLOCK_REALTIME, &tm );
                        t1 = tm.tv_nsec + tm.tv_sec * nano;
                        t3 = (int64_t)(plan[i-1].time) - (int64_t)((t1 - t2)/1000);
                        if (t3 - 700 > 0) //Magic Number 600
                               usleep(t3-700);
                }
                if (sendto(sockfd, plan[i].d_unit.p_buf, plan[i].d_unit.p_size, 0, (struct sockaddr*)sockaddr, sizeof(struct sockaddr_ll)) < 0)
                        printf("Send failed\n");
                clock_gettime( CLOCK_REALTIME, &tm );
                t2 = tm.tv_nsec + tm.tv_sec * nano;
        }
        return 0;
}

int destroy_plan(t_planunit* plan) 
{
	free( plan );
	return 0;
}

int main (int argc, char* argv[]) 
{
        int sockfd;
        uint16_t h_size;
        struct sockaddr_ll sockaddr;
        t_metaunit* m_plan;
        t_planunit* plan;
        char* p_buff;

        static protocol p_proto = 0;
	static int distribution = 0;
	static char my_eth = 1;

        char* if_name = NULL;
        uint32_t p_cnt = 0;

	uint8_t eth_s[6];
	uint8_t eth_d[6] = {  MY_DEST_MAC0, MY_DEST_MAC1, MY_DEST_MAC2, 
				MY_DEST_MAC3, MY_DEST_MAC4, MY_DEST_MAC5};

	char *ip_s = NULL; 
	char *ip_d = NULL;
	uint16_t port_s = 0; 
	uint32_t port_d = 0;
	char* t_file_n = NULL;
	char* s_file_n = NULL;
	FILE* t_file = NULL;
	FILE* s_file = NULL;	

	uint16_t s_a = 0, s_b = 0;
	uint32_t t_a = 0, t_b = 0;
	double s_mu = 0.0, t_mu = 0.0;
	

	int c;
	while ( 1 )
	{
		static struct option long_options[] =
		{
			{"UDP",		no_argument,		(void*)&p_proto, UDP},
			{"TCP",		no_argument,		(void*)&p_proto, TCP},
			{"exponential",	no_argument,		(void*)&distribution,	EXPONEN_D},
			{"flat",	no_argument,		(void*)&distribution,	FLAT_D},
			{"custom",	no_argument,		(void*)&distribution,	CUSTOM_D},
			{"iface",	required_argument,	0,	'a'},
			{"count",	required_argument,	0,	'b'},
			{"ethS",	required_argument,	0,	'c'},
			{"ethD",	required_argument,	0,	'd'},
			{"ipS",		required_argument,	0,	'e'},
			{"ipD",		required_argument,	0,	'f'},
			{"portS",	required_argument,	0,	'g'},
			{"portD",	required_argument,	0,	'h'},
			{"minT",	required_argument,	0,	'i'},
			{"maxT",	required_argument,	0,	'j'},
			{"avgT",	required_argument,	0,	'k'},
			{"minS",	required_argument,	0,	'l'},
			{"maxS",	required_argument,	0,	'm'},
			{"avgS",	required_argument,	0,	'n'},
			{"fileT",	required_argument,	0,	'o'},
			{"fileS",	required_argument,	0,	'p'},
			{0,		0,		0,	0}
		};
		int option_index = 0;		
		c = getopt_long_only(argc, argv, "", long_options, &option_index);
		if (c == -1)
			break;
		switch (c)
		{
			case 0:
				break;
			case 'a':
				if_name = optarg;
				break;
			case 'b':
				p_cnt = atoi(optarg);
				if (!p_cnt) { eexit("Bad Package Count\n") }
				break;
			case 'c':
				my_eth = 0;	
				if( 6 != sscanf( optarg, "%x:%x:%x:%x:%x:%x",
    						(unsigned int*)&eth_s[0], (unsigned int*)&eth_s[1], (unsigned int*)&eth_s[2],
						(unsigned int*)&eth_s[3], (unsigned int*)&eth_s[4], (unsigned int*)&eth_s[5] ) )
				{ eexit("Bad Source Mac Address\n") }
				break;
			case 'd':
				if( 6 != sscanf( optarg, "%x:%x:%x:%x:%x:%x",
    						(unsigned int*)&eth_d[0], (unsigned int*)&eth_d[1], (unsigned int*)&eth_d[2],
						(unsigned int*)&eth_d[3], (unsigned int*)&eth_d[4], (unsigned int*)&eth_d[5] ) )
				{ eexit("Bad Destination Mac Address\n") }
				break;
			case 'e':
				ip_s = optarg;
				break;
			case 'f':
				ip_d = optarg;
				break;
			case 'g':
				if (atoi(optarg) > 65535 || atoi(optarg) <= 0)
					{ eexit("Bad Source Port\n") }
				else port_s = atoi(optarg);
				break;
			case 'h':
				if (atoi(optarg) > 65535 || atoi(optarg) <= 0)
					{ eexit("Bad Destination Port\n") }
				else port_d = atoi(optarg);
				break;
			case 'i':
				t_a = atoi(optarg) * 1000;		
				break;
			case 'j':
				t_b = atoi(optarg) * 1000;		
				break;
			case 'k':
				t_mu = atof(optarg) * 1000;
				break;
			case 'l':
				s_a = atoi(optarg);		
				break;
			case 'm':
				s_b = atoi(optarg);		
				break;
			case 'n':
				s_mu = atof(optarg);
				break;
			case 'o':
				t_file_n = optarg;	
				break;
			case 'p':
				s_file_n = optarg;	
				break;
			default:
				break;		
		}
	}

	if (p_proto == 0) { eexit("Protocol not specified\n") }
	if (distribution == 0) { eexit("Distribution not specified\n") }
	if (if_name == NULL) { eexit("No interface specified\n") }
	if (p_cnt == 0) { eexit("No package to send\n") }
	if ((ip_s == NULL) || (ip_d == NULL)) { eexit("IP-adresses not specified\n") }
		
	if (distribution == FLAT_D) {
		if ( (t_mu != 0.0) && (t_a == 0) && (t_b == 0) && (t_file_n == NULL) ) {
			t_a = (uint32_t)t_mu;
			t_mu = 0.0;
		}
		else { eexit("Wrong time paramaters combination for flat distribution\n") }

		if ( (s_mu != 0.0) && (s_a == 0) && (s_b == 0) && (s_file_n == NULL) ) {
			s_a = (uint32_t)s_mu;
			s_mu = 0.0;
		}
		else { eexit("Wrong size paramaters combination for flat distribution\n") }
	}
	else if (distribution == EXPONEN_D) {
		if ( (t_mu != 0.0) && (t_a == 0) && (t_b == 0) && (t_file_n == NULL) ) {
			t_a = (uint32_t)t_mu;
			t_mu = 0.0;
		}
		else if ( (t_mu != 0.0) && (t_a != 0) && (t_b != 0) && (t_file_n == NULL) ) {
			if (t_a >= t_b) { eexit("Wrong time params\n") }
			if (((uint32_t)t_mu <= t_a) || ((uint32_t)t_mu >= t_b))
				{ eexit("Wrong avg time\n") } 
		}
		else { eexit("Wrong time paramaters combination for exponential distribution\n") }

		if ( (s_mu != 0.0) && (s_a == 0) && (s_b == 0) && (s_file_n == NULL) ) {
			s_a = (uint32_t)s_mu;
			s_mu = 0.0;
		}
		else if ( (s_mu != 0.0) && (s_a != 0) && (s_b != 0) && (s_file_n == NULL) ) {
			if (s_a >= s_b) { eexit("Wrong time params\n") }
			if (((uint32_t)s_mu <= s_a) || ((uint32_t)s_mu >= s_b))
				{ eexit("Wrong avg time\n") } 
		}
		else { eexit("Wrong size paramaters combination for exponential distribution\n") }
	}
	else if (distribution == CUSTOM_D) {
		if ( (t_mu != 0.0) && (t_a == 0) && (t_b == 0) && (t_file_n == NULL) ) {
			t_a = (uint32_t)t_mu;
			t_mu = 0.0;
		}
		else if ( (t_mu != 0.0) && (t_a != 0) && (t_b != 0) && (t_file_n == NULL) ) {
			if (t_a >= t_b) { eexit("Wrong time params\n") }
			if (((uint32_t)t_mu <= t_a) || ((uint32_t)t_mu >= t_b))
				{ eexit("Wrong avg time\n") } 
		}
		else if ( (t_mu == 0.0) && (t_a != 0) && (t_b != 0) && (t_file_n != NULL) ) {
			if (t_a >= t_b) { eexit("Wrong time params\n") }
		}
		else { eexit("Wrong time paramaters combination for custom distribution\n") }

		if ( (s_mu != 0.0) && (s_a == 0) && (s_b == 0) && (s_file_n == NULL) ) {
			s_a = (uint32_t)s_mu;
			s_mu = 0.0;
		}
		else if ( (s_mu != 0.0) && (s_a != 0) && (s_b != 0) && (s_file_n == NULL) ) {
			if (s_a >= s_b) { eexit("Wrong time params\n") }
			if (((uint32_t)s_mu <= s_a) || ((uint32_t)s_mu >= s_b))
				{ eexit("Wrong avg time\n") } 
		}
		else if ( (s_mu == 0.0) && (s_a != 0) && (s_b != 0) && (s_file_n != NULL) ) {
			if (s_a >= s_b) { eexit("Wrong time params\n") }
		}
		else { eexit("Wrong size paramaters combination for custom distribution\n") }
	}

        if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
		perror("Cannot create Socket\n");
		exit(1);
        }

	if (t_file_n != NULL) t_file = fopen(t_file_n, "r");
	if (s_file_n != NULL) s_file = fopen(s_file_n, "r");


	if (my_eth) p_buff = init_packet(sockfd, if_name, ip_s, port_s, ip_d, port_d, p_proto, NULL, eth_d, &sockaddr, &h_size);
	else p_buff = init_packet(sockfd, if_name, ip_s, port_s, ip_d, port_d, p_proto, eth_s, eth_d, &sockaddr, &h_size);

	m_plan = cr_meta(p_cnt, s_a, s_b, s_mu, t_a, t_b, t_mu, s_file, t_file);
        plan = creat_plan(p_buff, h_size, m_plan, p_cnt, p_proto);
/*	for (int i = 0; i < p_cnt; i++) 
	{
		printf("%d\n",plan[i].d_unit.p_size);
	}
	return 0; */
        exec_plan(plan, p_cnt, sockfd, &sockaddr); 

	destroy_meta(m_plan);
	destroy_plan(plan);
	
	if (t_file != NULL) fclose(t_file);
	if (s_file != NULL) fclose(s_file);
        return 0;

}
