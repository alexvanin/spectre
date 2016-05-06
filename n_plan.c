#include <malloc.h>
#include "n_plan.h"
#include "rand_exp.h"

t_metaunit* cr_meta(uint32_t p_cnt, uint16_t s_a, uint16_t s_b, double s_mu,
			uint32_t t_a, uint32_t t_b, double t_mu, FILE* s_file, FILE* t_file) 
{
	int i;
	t_metaunit* m_plan;
	gsl_rng *gr1, *gr2;

	cd_unit t_unit; 
	cd_unit s_unit; 

	if ((t_a == 0) ^ (s_a == 0)) {
		return NULL;
	}

	if (s_file != NULL) creat_cd_unit(s_file, &s_unit);	
	if (t_file != NULL) creat_cd_unit(t_file, &t_unit);


	gr1 = init_exp_d();
        m_plan = malloc(sizeof(t_metaunit) * p_cnt);
	gr2 = init_exp_d();

        for (i = 0; i < p_cnt; i++) {
		if (t_file != NULL) {
			m_plan[i].p_delay = get_custom(gr1, t_a, t_b, &t_unit);
		}
		else {
			if (t_b == 0) m_plan[i].p_delay = t_a;
			else m_plan[i].p_delay =  get_exp(gr1, t_mu, t_a, t_b);
		}
		
		if (s_file != NULL) {
			m_plan[i].p_size = get_custom(gr2, s_a, s_b, &s_unit);
		}
		else {
			if (s_b == 0) m_plan[i].p_size = s_a;
			else m_plan[i].p_size =  get_exp(gr2, s_mu, s_a, s_b);
		}
        }
	
	if (t_file != NULL) destroy_cd_unit(&t_unit);
	if (s_file != NULL) destroy_cd_unit(&s_unit);

	destroy_exp_d(gr1);	
	destroy_exp_d(gr2);	

        return m_plan;
}

int destroy_meta(t_metaunit* m_plan) 
{
	free( m_plan );
	return 0;
}
