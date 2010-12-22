#include "cvmx.h"
#include "cvmx-fpa.h"
#include "cvmx-helper.h"
#include "cvmx-pko.h"
#include "cvmx-coremask.h"

#include "ewx_shell.h"
#include "ewx_mem.h"
#include "ewx_board_init.h"

extern int ewx_board_check();
extern int ewx_board_valid();

static CVMX_SHARED int single_se_running = 1;

static void __init_default_ipd_group_mask(uint16_t group)
{
	int interface, port;
	for (interface = 0; interface < 2; interface++) {
		int num_ports = cvmx_helper_ports_on_interface(interface);
    	for(port = 0; port < num_ports; port++) {
	    	int ipd_port = cvmx_helper_get_ipd_port(interface, port);
		    cvmx_pip_port_tag_cfg_t tag_config;

    		tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(ipd_port));
	    	tag_config.s.grp = group;
	    	cvmx_write_csr(CVMX_PIP_PRT_TAGX(ipd_port), tag_config.u64);
    	}
	}
}

/**
 * Setup the Cavium Simple Executive Libraries using defaults
 *
 * @param num_packet_buffers
 *               Number of outstanding packets to support
 * @return Zero on success
 */

static int __application_init_simple_exec(int num_packet_buffers)
{
    if (cvmx_helper_initialize_fpa(num_packet_buffers, num_packet_buffers, CVMX_PKO_MAX_OUTPUT_QUEUES * 4, 0, 0))
        return -1;

    if (!cvmx_octeon_is_pass1())
    {
        /* Don't enable RED for Pass 1 due to errata */
        if (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM)
            cvmx_helper_setup_red(num_packet_buffers/4, num_packet_buffers/8);
    }

    if (octeon_has_feature(OCTEON_FEATURE_NO_WPTR))
    {
        cvmx_ipd_ctl_status_t ipd_ctl_status;
//        printf("Enabling CVMX_IPD_CTL_STATUS[NO_WPTR]\n");
        ipd_ctl_status.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
        ipd_ctl_status.s.no_wptr = 0;
        cvmx_write_csr(CVMX_IPD_CTL_STATUS, ipd_ctl_status.u64);
    }

    int result = cvmx_helper_initialize_packet_io_global();
    __init_default_ipd_group_mask(FROM_INPUT_PORT_GROUP);
    cvmx_helper_ipd_and_packet_input_enable();

    if (!cvmx_octeon_is_pass1())
    {
        /* Leave 16 bytes space for the ethernet header */
        cvmx_write_csr(CVMX_PIP_IP_OFFSET, 3);
        int port, interface;
        /* Enable storing short packets only in the WQE */
        for (interface = 0; interface < 2; interface++)
        {
           /* Set the frame max size and jabber size to 65535, as the defaults
              are too small. */
           cvmx_helper_interface_mode_t imode = cvmx_helper_interface_get_mode(interface);
           int num_ports = cvmx_helper_ports_on_interface(interface);

           switch (imode)
           {
               case CVMX_HELPER_INTERFACE_MODE_SGMII:
               case CVMX_HELPER_INTERFACE_MODE_XAUI:
                   for (port=0; port < num_ports; port++)
                       cvmx_write_csr(CVMX_GMXX_RXX_JABBER(port,interface), 65535);
                   break;

               case CVMX_HELPER_INTERFACE_MODE_RGMII:
               case CVMX_HELPER_INTERFACE_MODE_GMII:
                   for (port=0; port < num_ports; port++)
                   {
                       cvmx_write_csr(CVMX_GMXX_RXX_FRM_MAX(port,interface), 65535);
                       cvmx_write_csr(CVMX_GMXX_RXX_JABBER(port,interface), 65535);
                   }
                   break;
               default:
                   break;
           }

            for (port=0; port < num_ports; port++)
            {
                cvmx_pip_port_cfg_t port_cfg;
                port_cfg.u64 = cvmx_read_csr(CVMX_PIP_PRT_CFGX(port + interface*16));
                port_cfg.s.dyn_rs = 0;
                cvmx_write_csr(CVMX_PIP_PRT_CFGX(port + interface*16), port_cfg.u64);
            }
        }
    }

    return result;
}

int ewx_board_init()
{
    cvmx_sysinfo_t *sysinfo;
	int i, result;

    cvmx_user_app_init();

    /* compute coremask_se on all cores for the first barrier sync below */
    sysinfo = cvmx_sysinfo_get();
    uint16_t coremask_se = sysinfo->core_mask;

#if 0 //for simulator
    if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
    {
        if (OCTEON_IS_MODEL(OCTEON_CN3005))
            packet_termination_num = 3032;
        else if (OCTEON_IS_MODEL(OCTEON_CN31XX) || OCTEON_IS_MODEL(OCTEON_CN3010) || OCTEON_IS_MODEL(OCTEON_CN50XX))
            packet_termination_num = 4548;
        else
			packet_termination_num = 6064;
    }
    else
       packet_termination_num = 1000;
#endif

    /*
     * elect a core to perform boot initializations, as only one core needs to
     * perform this function.
     *
     */
    if (cvmx_coremask_first_core(coremask_se)) {
        printf("Version: %s\n", cvmx_helper_get_version());
        ewx_board_check();
        if (!ewx_board_valid()) {
            return 0;
        }
		if (!(coremask_se & 1)) {
			/*there is another SE or linux, so we skip the io_global init, just as linux-filter*/
            single_se_running = 0;
			printf("waiting for ipd status\n");
			cvmx_ipd_ctl_status_t ipd_reg;
			do
			{
				ipd_reg.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
			} while (!ipd_reg.s.ipd_en);

			/* We need to call cvmx_cmd_queue_initialize() to get the pointer to
			   the named block. The queues are already setup by the ethernet
			   driver, so we don't actually need to setup a queue. Pass some
			   invalid parameters to cause the queue setup to fail */
			cvmx_cmd_queue_initialize(0, 0, -1, 0);

#if CVMX_PKO_USE_FAU_FOR_OUTPUT_QUEUES
    #error Linux-filter cannot be built with CVMX_PKO_USE_FAU_FOR_OUTPUT_QUEUES
#endif
			for (i=0; i<cvmx_helper_get_number_of_interfaces(); i++) {
                // 初始化静态全局变量 interface_port_count
				cvmx_helper_interface_probe(i);
			}
            __init_default_ipd_group_mask(FROM_INPUT_PORT_GROUP);
		} else if ((result = __application_init_simple_exec(7000)) != 0) {
            printf("Simple Executive initialization failed.\n");
            printf("TEST FAILED\n");
            return result;
        }
    }
    cvmx_coremask_barrier_sync(coremask_se);

    cvmx_helper_initialize_packet_io_local();

    if (!single_se_running) {
        cvmx_pow_set_group_mask(cvmx_get_core_num(), (1<<FROM_LINUX_GROUP) | (1<<FROM_INPUT_PORT_GROUP));
    } else {
        cvmx_pow_set_group_mask(cvmx_get_core_num(), 1<<FROM_INPUT_PORT_GROUP);
    }
    if (cvmx_coremask_first_core(coremask_se)) {
        printf("Done initializing board.\n");
	}
    cvmx_coremask_barrier_sync(coremask_se);
    return 0;
}
