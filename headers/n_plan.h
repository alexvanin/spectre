#ifndef NPLAN_H
#define NPLAN_H

#include <stdio.h>
#include <n_packet.h> 

typedef struct 
{
	uint16_t p_size;
	uint32_t p_delay;		
} t_metaunit;
	
typedef struct
{
	char* p_buf;
	uint16_t p_size;
	protocol p_proto;
	uint16_t p_ip_size;
	uint16_t p_ip_csum;
	uint16_t p_tr_size;
	uint16_t p_tr_csum;
} t_dataunit;

typedef struct
{
	t_dataunit d_unit;
	uint32_t time;
} t_planunit;



t_metaunit* cr_meta(uint32_t, uint16_t, uint16_t, double,  uint32_t, uint32_t, double, FILE*, FILE*);
int destroy_meta(t_metaunit*);

#endif
