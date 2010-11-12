#ifndef __EWX_HELPER_UTIL_H__
#define __EWX_HELPER_UTIL_H__

#include <cvmx.h>
#include <cvmx-wqe.h>
#include <cvmx-packet.h>

extern int ewx_helper_dump_packet(cvmx_buf_ptr_t buffer_ptr, uint64_t len);
extern int ewx_helper_dump_work(cvmx_wqe_t *work);

#endif
