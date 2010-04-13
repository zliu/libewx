#include <cvmx.h>
#include <cvmx-wqe.h>
#include <cvmx-fpa.h>
#include <cvmx-config.h>
#include <cvmx-pow.h>
#include <cvmx-helper-util.h>
#include <cvmx-tim.h>
#include "ewx_thread.h"
#include "ewx_code.h"

int32_t ewx_thread_create(uint32_t tag, cvmx_pow_tag_type_t tag_type, uint64_t qos, uint64_t grp, ewx_thread_fn fn, 
								 void *param, uint32_t param_len)
{
	cvmx_wqe_t *wqe_p;
	ewx_thread_t *p;

	if (grp >= 16 || param_len > 96-sizeof(ewx_thread_t)) {
		return -EWX_PARAM_ERROR;
	}

	wqe_p = cvmx_fpa_alloc(CVMX_FPA_WQE_POOL);
	
	if (wqe_p == NULL) {
		return -EWX_NO_SPACE_ERROR;
	}
	
	memset(wqe_p, 0, sizeof(cvmx_wqe_t));
	wqe_p->unused = EWX_THREAD_WORK_UNUSED;
	p = (ewx_thread_t *)wqe_p->packet_data;
	p->magic = EWX_THREAD_MAGIC_NUM;
	p->fn = fn;
	p->param = (void *)p+sizeof(ewx_thread_t);
	p->free = 1;
	p->tick = 0;

	memcpy(p->param, param, param_len);
	//printf("p->param=%p, p->param=%lx\n", p->param, *(unsigned long *)p->param);
	cvmx_pow_work_submit(wqe_p, tag, tag_type, qos, grp);

	return 0;
}

int32_t ewx_timer_create(uint32_t tag, cvmx_pow_tag_type_t tag_type, uint64_t qos, uint64_t grp, ewx_thread_fn fn, 
								void *param, uint32_t param_len, uint64_t tick)
{
	cvmx_wqe_t *wqe_p;
	ewx_thread_t *p;
	int32_t result = 0;

	if (grp >= 16 || param_len > 96-sizeof(ewx_thread_t)) {
		return -EWX_PARAM_ERROR;
	}

	wqe_p = cvmx_fpa_alloc(CVMX_FPA_WQE_POOL);
	
	if (wqe_p == NULL) {
		return -EWX_NO_SPACE_ERROR;
	}
	
	memset(wqe_p, 0, sizeof(cvmx_wqe_t));
	wqe_p->unused = EWX_THREAD_WORK_UNUSED;
	wqe_p->tag = tag;
	wqe_p->tag_type = tag_type;
	wqe_p->qos = qos;
	wqe_p->grp = grp;

	p = (ewx_thread_t *)wqe_p->packet_data;
	p->magic = EWX_THREAD_MAGIC_NUM;
	p->fn = fn;
	p->param = (void *)p+sizeof(ewx_thread_t);
	p->free = 0;
	p->tick = tick;

	memcpy(p->param, param, param_len);
	//printf("p->param=%p, p->param=%lx\n", p->param, *(unsigned long *)p->param);
	if (tick == 0) {
		cvmx_pow_work_submit(wqe_p, wqe_p->tag, wqe_p->tag_type, wqe_p->qos, wqe_p->grp);
	} else {
		result = cvmx_tim_add_entry(wqe_p, p->tick, NULL);
	}
	return result;
	
}

int32_t ewx_thread_process(cvmx_wqe_t *wqe_p)
{
	ewx_thread_t *p;
	p = (ewx_thread_t *)wqe_p->packet_data;

	if ((wqe_p->unused != EWX_THREAD_WORK_UNUSED) || (p->magic != EWX_THREAD_MAGIC_NUM)) {
		return 0;
	}

	if (p->fn != NULL) {
		p->fn(p, p->param);
	}
	
	if (p->free) {
		//cvmx_helper_free_packet_data(wqe_p);
		cvmx_fpa_free(wqe_p, CVMX_FPA_WQE_POOL, 0);
	} else {
		if (p->tick == 0) {
			//cvmx_pow_tag_sw_desched(wqe_p->tag, wqe_p->tag_type, wqe_p->grp, 0);
			cvmx_pow_desched(0);
		} else {
			cvmx_tim_add_entry(wqe_p, p->tick, NULL);
		}
	}
	return 0;
}


int32_t ewx_thread_stop(ewx_thread_t *p)
{
	p->free = 1;
	return 0;
}
