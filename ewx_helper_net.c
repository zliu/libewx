#include <cvmx.h>
#include "ewx_helper_net.h"

void ewx_ip_checksum(ewx_ip_hdr_t *ip_head)
{
    int word = ip_head->IHL * 2;
    uint64_t sum = 0;
    uint16_t *ip = (uint16_t *)ip_head;
    ip_head->hdr_chksm = 0;
    while(word--) {
        sum += *ip++;
    }
    /* while(sum>>16) */
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    ip_head->hdr_chksm = (uint16_t)(~sum);
}

