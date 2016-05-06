#include "rand_exp.h"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <sys/time.h>
#include <malloc.h>
#include <stdint.h>

#define eexit(text) printf((text)); exit(1);

double r_max = -1;
double r_min = 10000;
double mu = 0;


int creat_cd_unit(FILE* file, cd_unit* unit)
{
	double summ = 0;
	double temp = 0;
	char string[100];
	if (fgets(string, 100, file) != NULL)
	{
		if ( 1 != sscanf(string, "%d", (int*)&(unit->rows)) )
			{ eexit("Cannot read file\n") }	
	}	
	unit->data = malloc(sizeof(double) * unit->rows);

	for (int i = 0; i < unit->rows; i++) {
		fgets(string,100,file);
		if ( 1 != sscanf(string, "%lf", &temp) )
			{ eexit("Cannot read file\n") } 
		unit->data[i] = summ;
		summ += temp;
		if (summ > 1.0) { eexit("Wrong distribution file containg\n") }
	}
	return 0;
}

int destroy_cd_unit(cd_unit* unit) 
{
	free(unit->data);
	return 0;
}

gsl_rng* init_exp_d()
{
        struct timeval t1;
        const gsl_rng_type* T;
        gsl_rng* gr = malloc( sizeof( gsl_rng ) );

        T = gsl_rng_default;
        gr = gsl_rng_alloc( T );

        gettimeofday( &t1, NULL );
        gsl_rng_set( gr, t1.tv_usec * t1.tv_sec );

	return gr;
}

uint32_t get_exp(gsl_rng* gr, double mu, uint32_t a, uint32_t b) 
{
	double res = a + gsl_ran_exponential( gr, mu - a );
	if ( res > b ) res = gsl_ran_flat( gr, a, b );	
	return (uint32_t)res;
}


int destroy_exp_d(gsl_rng* gr)
{
        gsl_rng_free( gr );       //Free allocated memory for generator
	return 0;
}

uint32_t get_custom(gsl_rng* gr, uint32_t a, uint32_t b, cd_unit* cd)
{
	int i = 0;
	uint32_t dif = (b-a) / cd->rows;
	double rand = gsl_rng_uniform(gr);

	for (i = 1; i < cd->rows; i++) {
		if (rand < cd->data[i])	
			break;
	}
	uint32_t inner = gsl_rng_uniform_int(gr, dif+1); 
	return a+(i-1)*dif+inner;
}
