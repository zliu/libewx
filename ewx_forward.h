#ifndef __EWX_FORWARD_H__
#define __EWX_FORWARD_H__

int ewx_send_packet_to_pow0(cvmx_wqe_t *work);
int ewx_send_packet_to_pow0_and_portx(cvmx_wqe_t *work, int port);
int ewx_send_packet_to_portx(cvmx_wqe_t *work, int port);

#endif
