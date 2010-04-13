#include "cvmx.h"
#include "cvmx-helper.h"
#include "ewx_led.h"

typedef struct led_status {
	int status;
	uint16_t led_mask;/*led mask, not used now*/
	uint32_t last_rcv_pkts[EWX_MAX_LED];
	uint32_t last_send_pkts[EWX_MAX_LED];
} led_status_t;

static CVMX_SHARED led_status_t led_status;

static int get_ipd_port_from_led_index(int led_index)
{
	if ((led_index >= 0) && (led_index < 4)) {
		return led_index;
	} else if ((led_index >= 4) && (led_index < EWX_MAX_LED)) {
		return led_index -4 + 16;
	} else {
		return -1;
	}
}

static int detect_packets(int led_index)
{
	int ipd_port = get_ipd_port_from_led_index(led_index);
	int interface = cvmx_helper_get_interface_num(ipd_port);
	int index = cvmx_helper_get_interface_index_num(ipd_port);

	cvmx_gmxx_rxx_stats_ctl_t rstats;
	cvmx_gmxx_rxx_stats_pkts_t rpkts;

	cvmx_gmxx_txx_stats_ctl_t tstats;
	cvmx_gmxx_txx_stat3_t tpkts;

	rstats.u64 = cvmx_read_csr(CVMX_GMXX_RXX_STATS_CTL(index, interface));
	rpkts.u64 = cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS(index, interface));
	
	tstats.u64 = cvmx_read_csr(CVMX_GMXX_TXX_STATS_CTL(index, interface));
	tpkts.u64 = cvmx_read_csr(CVMX_GMXX_TXX_STAT3(index, interface));
	

	if (rstats.s.rd_clr) {
		if (rpkts.s.cnt) {
			return 1;
		}
	} 
	if (tstats.s.rd_clr){
		if (tpkts.s.pkts) {
			return 1;
		}
	} 
	if (rstats.s.rd_clr || tstats.s.rd_clr){
		return 0;
	}else {
		if (rpkts.s.cnt != led_status.last_rcv_pkts[led_index]) {
			led_status.last_rcv_pkts[led_index] = rpkts.s.cnt;
			return 1;
		} else if (tpkts.s.pkts != led_status.last_send_pkts[led_index]) {
			led_status.last_send_pkts[led_index] = tpkts.s.pkts;
			return 1;
		} else {
			return 0;
		}
	}
}
int ewx_led_init()
{
	int interface;
	int imode;
	int flag = 0;

	for (interface=0; interface<2; interface++) {
		imode = cvmx_helper_interface_get_mode(interface);

		switch (imode){
		case CVMX_HELPER_INTERFACE_MODE_SGMII:
			flag = 1;
			break;
		default:
			break;
		}
		printf("imode=%d, flag=%d\n", imode, flag);
	}
	if (flag) {
		cvmx_led_clk_phase_t phase;
		cvmx_led_blink_t blink;
		cvmx_led_prt_fmt_t fmt;
		cvmx_led_prt_t prt;
		cvmx_led_udd_cntx_t cnt;
		cvmx_led_udd_datx_t dat;
		cvmx_led_en_t en;
		
		phase.u64 = 0;
		blink.u64 = 0;
		fmt.u64 = 0;
		prt.u64 = 0;
		cnt.u64 = 0;
		dat.u64 = 0;
		en.u64 = 0;

		phase.s.phase = 0x7f;
		blink.s.rate = 0xff;
		fmt.s.format = 0x5;
		prt.s.prt_en = 0xff;
		cnt.s.cnt = 0x3;
		dat.s.dat = 0x1;
		en.s.en = 0x1;
		cvmx_write_csr(CVMX_LED_CLK_PHASE, phase.u64);
		cvmx_write_csr(CVMX_LED_BLINK, blink.u64);
		cvmx_write_csr(CVMX_LED_PRT_FMT, fmt.u64);
		cvmx_write_csr(CVMX_LED_PRT, prt.u64);
		cvmx_write_csr(CVMX_LED_UDD_CNTX(0), cnt.u64);
		cvmx_write_csr(CVMX_LED_UDD_DATX(0), dat.u64);
		cvmx_write_csr(CVMX_LED_EN, en.u64);
		printf("LED enable\n");
	} else {
		return -1;
	}
	return 0;
}

int ewx_led_timer_process()
{
	int i, ipd_port;
	int index, interface;
	cvmx_pcsx_miscx_ctl_reg_t pcsx_miscx_ctl_reg;
	cvmx_pcsx_mrx_status_reg_t pcsx_mrx_status_reg;
	cvmx_led_prt_statusx_t status, status_save;
	uint64_t flag = 0;

	for (i=0; i<EWX_MAX_LED; i++) {
		ipd_port = get_ipd_port_from_led_index(i);
		index = cvmx_helper_get_interface_index_num(ipd_port);
		interface = cvmx_helper_get_interface_num(ipd_port);
		pcsx_miscx_ctl_reg.u64 = cvmx_read_csr(CVMX_PCSX_MISCX_CTL_REG(index, interface));          

		if (pcsx_miscx_ctl_reg.s.mode) {
			/*1000 base-x, need led process*/
			pcsx_mrx_status_reg.u64 = cvmx_read_csr(CVMX_PCSX_MRX_STATUS_REG(index, interface));
			status.u64 = cvmx_read_csr(CVMX_LED_PRT_STATUSX(i));
			
			if (pcsx_mrx_status_reg.s.lnk_st) {
				if (detect_packets(i)) {/*light flash*/
					if (flag) {
						status.u64 = status_save.u64;
					} else {
						status.s.status = (status.s.status | 0x2) ^ 0x1;
						flag = 1;
						status_save.u64 = status.u64;
					}
					cvmx_write_csr(CVMX_LED_PRT_STATUSX(i), status.u64);
					continue;
				} else if ((status.s.status & 0x3) != 0x2) {/*light on*/
					status.s.status = (status.s.status & 0xfc) |  0x2;
					cvmx_write_csr(CVMX_LED_PRT_STATUSX(i), status.u64); 
				} 
			} else {
				if ((status.s.status & 0x3) != 0x0) {/*light off*/
					status.s.status = status.s.status & 0xfc;
					cvmx_write_csr(CVMX_LED_PRT_STATUSX(i), status.u64); 
				}
			}
		}
	}
	return 0;
}
