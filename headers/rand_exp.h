#ifndef N_RANDEXP_H
#define N_RANDEXP_H

#include <gsl/gsl_rng.h>
#include <stdint.h>
#include <stdio.h>

#define EXPONEN_D	1
#define FLAT_D		2
#define CUSTOM_D	3

typedef struct 
{
	uint16_t rows;
	double* data; 
} cd_unit;

int creat_cd_unit(FILE*, cd_unit*);
int destroy_cd_unit(cd_unit*);

gsl_rng* init_exp_d();
int destroy_exp_d(gsl_rng*);
uint32_t get_exp(gsl_rng*, double, uint32_t, uint32_t); 
uint32_t get_custom(gsl_rng*, uint32_t, uint32_t, cd_unit*); 

#endif
