#include "cvmx.h"
#include "cvmx-wqe.h"
#include "cvmx-pow.h"
#include "cvmx-pko.h"
#include "cvmx-helper.h"

#include "ewx_board_init.h"  // Maybe need to fix later for more flexible GROUP

#define LINUX_PORT 40

int ewx_send_packet_to_pow0(cvmx_wqe_t *work)
{
    work->ipprt = LINUX_PORT;
    if (work->word2.s.software) {
        cvmx_pow_work_submit(work, work->tag, work->tag_type, work->qos, TO_LINUX_GROUP);
    } else {
        cvmx_pow_tag_sw_desched(work->tag, CVMX_POW_TAG_TYPE_ATOMIC, TO_LINUX_GROUP, 0);
    }
    return 0;
}

int ewx_send_packet_to_pow0_and_portx(cvmx_wqe_t *work, int port)
{
    int queue, result = 0;
    cvmx_pko_command_word0_t pko_command;
    cvmx_wqe_t *linux_work = cvmx_fpa_alloc(CVMX_FPA_WQE_POOL);
    if(linux_work == NULL) {
        printf("Err: Failed to send packet, cannot alloc WQE for WQE pool.\n");
        return -1;
    }
    memcpy(linux_work, work, sizeof(cvmx_wqe_t));
    linux_work->ipprt = LINUX_PORT;
    linux_work->grp = TO_LINUX_GROUP;
    linux_work->word2.s.software = 1;

    queue = cvmx_pko_get_base_queue(port);
    cvmx_pko_send_packet_prepare(port, queue, CVMX_PKO_LOCK_ATOMIC_TAG);

    pko_command.u64 = 0;
    pko_command.s.total_bytes = work->len;
    pko_command.s.segs = work->word2.s.bufs;
    pko_command.s.dontfree = 1; //不释放, 反转位设置后会反转这一位
    pko_command.s.wqp = 1;
    pko_command.s.rsp = 1;

    if ((result = cvmx_pko_send_packet_finish3(port, queue, pko_command, work->packet_ptr,
                    cvmx_ptr_to_phys(linux_work), CVMX_PKO_LOCK_CMD_QUEUE)) != 0) {
        printf("Err: Failed to send packet by cvmx_pko_send_packet_finish3, error code %d\n", result);
        cvmx_helper_free_packet_data(work);
//        cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0);
        cvmx_fpa_free(linux_work, CVMX_FPA_WQE_POOL, 0);
        return -1;
    }
    return 0;
}

int ewx_send_packet_to_portx(cvmx_wqe_t *work, int port)
{
    int queue, result;
    cvmx_pko_command_word0_t pko_command;

    queue = cvmx_pko_get_base_queue(port);
    cvmx_pko_send_packet_prepare(port, queue, CVMX_PKO_LOCK_ATOMIC_TAG);

    /* Build a PKO pointer to this packet */
    pko_command.u64 = 0;
    pko_command.s.total_bytes = work->len;
    pko_command.s.segs = work->word2.s.bufs;

    /*
     * Send the packet and wait for the tag switch to complete before
     * accessing the output queue. This ensures the locking required
     * for the queue.
     *
     */
    if ((result = cvmx_pko_send_packet_finish(port, queue, pko_command, work->packet_ptr, CVMX_PKO_LOCK_ATOMIC_TAG) != 0))
    {
        printf("Failed to send packet by cvmx_pko_send_packet_finish, error code %d\n", result);
        return -1;
    }
    return 0;
}
