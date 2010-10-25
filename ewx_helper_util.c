#include <cvmx.h>
#include <cvmx-wqe.h>
#include <cvmx-packet.h>

int ewx_helper_dump_packet(cvmx_buf_ptr_t  buffer_ptr, uint64_t len)
{
    uint32_t        count, print_count;
    uint64_t        remaining_bytes = len;
    uint64_t        start_of_buffer;
    uint8_t *       data_address;
    uint8_t *       end_of_data;

    cvmx_dprintf("------ packet_ptr ------\n");

    while (remaining_bytes)
    {
        start_of_buffer = ((buffer_ptr.s.addr >> 7) - buffer_ptr.s.back) << 7;
        cvmx_dprintf("Buffer Start(PHY)         :   %llx\n", (unsigned long long)start_of_buffer);
        cvmx_dprintf("Buffer I          s.i     :   %u\n", buffer_ptr.s.i);
        cvmx_dprintf("Buffer Back       s.back  :   %u\n", buffer_ptr.s.back);
        cvmx_dprintf("Buffer Pool       s.pool  :   %u\n", buffer_ptr.s.pool);
        cvmx_dprintf("Buffer Addr(PHY)  s.addr  :   %llx\n", (unsigned long long)buffer_ptr.s.addr);
        cvmx_dprintf("Buffer Size       s.size  :   %u\n", buffer_ptr.s.size);

        data_address = (uint8_t *)cvmx_phys_to_ptr(buffer_ptr.s.addr);
        end_of_data = data_address + buffer_ptr.s.size;
        count = 0;
        print_count = 0;
        cvmx_dprintf("            00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F\n");
        while (data_address < end_of_data)
        {
            if (remaining_bytes == 0)
                break;
            else
                remaining_bytes--;
            if ((print_count & 0xf) == 0) {
                cvmx_dprintf("    %04x    ", print_count);
            }
            cvmx_dprintf("%02x  ", (unsigned int)*data_address);
            data_address++;
            print_count++;
            if (remaining_bytes && (count == 0xf))
            {
                cvmx_dprintf("\n");
                count = 0;
            }
            else
                count++;
        }
        if (remaining_bytes)
            buffer_ptr = *(cvmx_buf_ptr_t*)cvmx_phys_to_ptr(buffer_ptr.s.addr - 8);
    }
    cvmx_dprintf("\n");
    return 0;
}

int ewx_helper_dump_work(cvmx_wqe_t *work)
{
    uint32_t        print_count;

    cvmx_dprintf("------ word0 ------\n");
    cvmx_dprintf("HW chksum     (16):   %u\n", work->hw_chksum);
    cvmx_dprintf("unused        ( 8):   %u\n", work->unused);
  //cvmx_dprintf("next_ptr      (40):   %p\n", work->next_ptr);
  //
    cvmx_dprintf("------ word1 ------\n");
    cvmx_dprintf("Packet Length (16):   %u\n", work->len);
    cvmx_dprintf("Input Port    ( 6):   %u\n", work->ipprt);
    cvmx_dprintf("QoS           ( 3):   %u\n", work->qos);
    cvmx_dprintf("Grp           ( 4):   %u\n", work->grp);
    cvmx_dprintf("Tag type      ( 3):   %u\n", work->tag_type);
    cvmx_dprintf("Tag order     (32):   %u\n", work->tag);

    cvmx_dprintf("------ word2 ------\n");
    if(work->word2.s.not_IP == 0) {
        cvmx_dprintf("This packet is an IP packet!\n");
        cvmx_dprintf("s.bufs        ( 8):   Bufs        %u\n", work->word2.s.bufs);
        cvmx_dprintf("s.ip_offset   ( 8):   IP_Offset   %u\n", work->word2.s.ip_offset);
        cvmx_dprintf("s.vlan_valid  ( 1):   VV          %u\n", work->word2.s.vlan_valid);
        cvmx_dprintf("s.unassigned  ( 2):   VS & 0      %u\n", work->word2.s.unassigned);
        cvmx_dprintf("s.vlan_cfi    ( 1):   VC          %u\n", work->word2.s.vlan_cfi);
        cvmx_dprintf("s.vlan_id     (12):   VID         %u\n", work->word2.s.vlan_id);
        cvmx_dprintf("s.unassigned2 (12):   PR(3) & 0   %u\n", work->word2.s.unassigned2);
        cvmx_dprintf("s.dec_ipcomp  ( 1):   CO          %u\n", work->word2.s.dec_ipcomp);
        cvmx_dprintf("s.tcp_or_udp  ( 1):   TU          %u\n", work->word2.s.tcp_or_udp);
        cvmx_dprintf("s.dec_ipsec   ( 1):   SE          %u\n", work->word2.s.dec_ipsec);
        cvmx_dprintf("s.is_v6       ( 1):   V6          %u\n", work->word2.s.is_v6);
        cvmx_dprintf("s.software    ( 1):               %u\n", work->word2.s.software);
        cvmx_dprintf("s.L4_error    ( 1):   LE          %u\n", work->word2.s.L4_error);
        cvmx_dprintf("s.is_frag     ( 1):   FR          %u\n", work->word2.s.is_frag);
        cvmx_dprintf("s.IP_exc      ( 1):   IE          %u\n", work->word2.s.IP_exc);
        cvmx_dprintf("s.is_bcast    ( 1):   B           %u\n", work->word2.s.is_bcast);
        cvmx_dprintf("s.is_mcast    ( 1):   M           %u\n", work->word2.s.is_mcast);
        cvmx_dprintf("s.not_IP      ( 1):   NI          %u\n", work->word2.s.not_IP);
        cvmx_dprintf("s.rcv_error   ( 1):   RE          %u\n", work->word2.s.rcv_error);
        cvmx_dprintf("s.err_code    ( 8):   Opcode      %u\n", work->word2.s.err_code);
    } else {
        cvmx_dprintf("This packet is Not an IP packet!\n");
        cvmx_dprintf("bufs          ( 8):               %u\n", work->word2.snoip.bufs);
        cvmx_dprintf("unused        ( 8):   0           %u\n", work->word2.snoip.unused);
        cvmx_dprintf("vlan_valid    ( 1):   VV          %u\n", work->word2.snoip.vlan_valid);
        cvmx_dprintf("unassigned    ( 2):   VS & 0      %u\n", work->word2.snoip.unassigned);
        cvmx_dprintf("vlan_cfi      ( 1):   VC          %u\n", work->word2.snoip.vlan_cfi);
        cvmx_dprintf("vlan_id       (12):   VID         %u\n", work->word2.snoip.vlan_id);
        cvmx_dprintf("unassigned2   (16):               %u\n", work->word2.snoip.unassigned2);
        cvmx_dprintf("software      ( 1):               %u\n", work->word2.snoip.software);
        cvmx_dprintf("unassigned3   ( 1):               %u\n", work->word2.snoip.unassigned3);
        cvmx_dprintf("is_rarp       ( 1):   IR          %u\n", work->word2.snoip.is_rarp);
        cvmx_dprintf("is_arp        ( 1):   IA          %u\n", work->word2.snoip.is_arp);
        cvmx_dprintf("is_bcast      ( 1):   B           %u\n", work->word2.snoip.is_bcast);
        cvmx_dprintf("is_mcast      ( 1):   M           %u\n", work->word2.snoip.is_mcast);
        cvmx_dprintf("not_IP        ( 1):   NI          %u\n", work->word2.snoip.not_IP);
        cvmx_dprintf("rcv_error     ( 1):   RE          %u\n", work->word2.snoip.rcv_error);
        cvmx_dprintf("err_code      ( 8):   Opcode      %u\n", work->word2.snoip.err_code);
    }

    if(work->word2.s.vlan_valid == 1) {
        cvmx_dprintf("------ vlan  ------\n");
        cvmx_dprintf("svlan.unused1 (16):               %u\n", work->word2.svlan.unused1);
        cvmx_dprintf("svlan.vlan    (16):               %u\n", work->word2.svlan.vlan);
        cvmx_dprintf("svlan.unused2 (32):               %u\n", work->word2.svlan.unused2);
        cvmx_dprintf("\n");
        cvmx_dprintf("vlan_valid    ( 1):   VV          %u\n", work->word2.s.vlan_valid);
        cvmx_dprintf("double_vlan   ( 1):   VS & 0      %u\n", work->word2.s.unassigned >> 1);
        cvmx_dprintf("vlan_cfi      ( 1):   VC          %u\n", work->word2.s.vlan_cfi);
        cvmx_dprintf("vlan_id       (12):   VID         %u\n", work->word2.s.vlan_id);
    }

    if (work->word2.s.bufs == 0) {
        cvmx_dprintf("Packet is stored at packet_data array in work\n");
    }
    else {
        ewx_helper_dump_packet(work->packet_ptr, work->len);
    }

    cvmx_dprintf("------ packet_data ------\n");
    cvmx_dprintf("            00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F\n");
    for(print_count = 0; print_count < 96; print_count++)
    {
        if ((print_count & 0xf) == 0) {
            cvmx_dprintf("    %04x    ", print_count);
        }
        cvmx_dprintf("%02x  ",work->packet_data[print_count]);
        if ((print_count & 0xf) == 0xf){
            cvmx_dprintf("\n");
        }
    }
    cvmx_dprintf("\n****** DUMP END ******\n");
    cvmx_dprintf("\n");
    return 0;
}
