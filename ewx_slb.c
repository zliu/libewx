#include <cvmx.h>
#include <cvmx-spinlock.h>
#include "ewx_slb.h"

#ifdef __KERNEL__
#include <asm/semaphore.h>
DECLARE_MUTEX(slb_lock);
#define SLB_LOCK down(&slb_lock)
#define SLB_UNLOCK up(&slb_lock)
#define print printk
#else 

#include <errno.h>
static CVMX_SHARED cvmx_spinlock_t slb_lock;
#define SLB_LOCK cvmx_spinlock_lock(&slb_lock)
#define SLB_UNLOCK cvmx_spinlock_unlock(&slb_lock)
#define print printf
#endif


#define SLB_BASE_ADDR ((cvmx_read_csr(0x8001180000000010)&0xffff)<<16)
//#define SLB_BASE_ADDR (0x17000000)
//0x18000000
#define SLB_DEVICE_LO_ID (SLB_BASE_ADDR+0x4)
#define SLB_DEVICE_HI_ID (SLB_BASE_ADDR+0x6)
#define SLB_ADDR_LO_REG (SLB_BASE_ADDR+0x20000)
#define SLB_ADDR_HI_REG (SLB_BASE_ADDR+0x20004)
#define SLB_RDATA_LO_REG (SLB_BASE_ADDR+0x20008)
#define SLB_RDATA_HI_REG (SLB_BASE_ADDR+0x2000c)
#define SLB_WDATA_LO_REG (SLB_BASE_ADDR+0x20010)
#define SLB_WDATA_HI_REG (SLB_BASE_ADDR+0x20014)
#define SLB_CMD_REG (SLB_BASE_ADDR+0x20018)
#define SLB_READ_CMD 0x1
#define SLB_WRITE_CMD 0x2
#define SLB_CMD_FIN 0x4
#define OP_SLB(REG) (*(volatile uint16_t *)cvmx_phys_to_ptr(REG))

#define SLB_MAX_RETRY_TIMES 20

/*
Address 24: 1: high 8bits; 0: low 8bits
Address 23: 1: access CPLD; 0: access sub card
Address 22: 1: access CPLD; 0: access FPGA
Address 21~16, 4~1: 10bits address
*/

uint16_t ewx_slb_local_read(uint8_t device_id, uint64_t address)
{
	uint16_t value;
	uint16_t temp;

	SLB_LOCK;
	
	address = ((address & 0xf) << 1) + (((address & 0x3f0)>>4) << 16);
	address += SLB_BASE_ADDR;
	temp = *(uint16_t *)(cvmx_phys_to_ptr(address));
	value = temp & 0xff;
	temp = *(uint16_t *)(cvmx_phys_to_ptr(address | (1<<24)));
	value |= (temp & 0xff) << 16;
	
	SLB_UNLOCK;
	
	return value;
}

void ewx_slb_local_write(uint8_t device_id, uint64_t address, uint16_t value)
{
	SLB_LOCK;
	
	address = ((address & 0xf) << 1) + (((address & 0x3f0)>>4) << 16);
	address += SLB_BASE_ADDR;
	*(uint16_t *)(cvmx_phys_to_ptr(address)) = value & 0xff;
	*(uint16_t *)(cvmx_phys_to_ptr(address | (1<<24))) =  (value & 0xff00) >> 16;
	
	SLB_UNLOCK;
}

/*
Address 23: 1: access CPLD; 0: access sub card
Address 22: 1: access CPLD; 0: access FPGA
Address 21~16, 4~1: 10bits address
*/

uint16_t ewx_slb_subc_read(uint16_t device_id, uint8_t sub_mode, uint16_t address)
{
	uint16_t slb_address;
	uint16_t rdata;
	uint64_t cycle;
	uint32_t times=0;
	uint8_t access_mode = 0;
#ifdef __KERNEL__
	uint8_t errno;
#endif
	//printf("address=%04x, access_mode=%d, sub_mode=%d\n", address, access_mode, sub_mode);
	do{
		errno = 0;
		SLB_LOCK;
		
		slb_address = (address) | (((uint16_t)access_mode) << 11) | (((uint16_t)sub_mode) << 10);
		OP_SLB(SLB_DEVICE_LO_ID) = LOW_BYTE(device_id);
		OP_SLB(SLB_DEVICE_HI_ID) = HIGH_BYTE(device_id);
		OP_SLB(SLB_ADDR_LO_REG) = LOW_BYTE(slb_address);
		OP_SLB(SLB_ADDR_HI_REG) = HIGH_BYTE(slb_address);
		OP_SLB(SLB_CMD_REG) = SLB_READ_CMD;
		cycle = cvmx_get_cycle();
		while((OP_SLB(SLB_CMD_REG) & 0x4) != SLB_CMD_FIN){
			if(cvmx_get_cycle()-cycle >= (SEC_CYCLE_COUNT/100/*1000*/)) {
				errno = -1;
				SLB_UNLOCK;
				break;
			}
		}
		if(errno == 0){
			break;
		}else{
			times++;
		}
	}while(times<5);

	if(errno != 0){
		printf("retry failed!\n");
		return -1;
	}
	rdata = OP_SLB(SLB_RDATA_LO_REG) | (OP_SLB(SLB_RDATA_HI_REG) << 8); 
	
	SLB_UNLOCK;
	
	return rdata;
}


void ewx_slb_subc_write(uint16_t device_id, uint8_t sub_mode, uint16_t address, uint16_t value)
{
	uint16_t slb_address;
	uint8_t access_mode = 0;
		
	SLB_LOCK;
	
	slb_address = (address) | (((uint16_t)access_mode) << 11) | (((uint16_t)sub_mode) << 10);
	OP_SLB(SLB_DEVICE_LO_ID) = LOW_BYTE(device_id);
	OP_SLB(SLB_DEVICE_HI_ID) = HIGH_BYTE(device_id);
	OP_SLB(SLB_ADDR_LO_REG) = LOW_BYTE(slb_address);
	OP_SLB(SLB_ADDR_HI_REG) = HIGH_BYTE(slb_address);
	OP_SLB(SLB_WDATA_LO_REG) = LOW_BYTE(value);
	OP_SLB(SLB_WDATA_HI_REG) = HIGH_BYTE(value);
	OP_SLB(SLB_CMD_REG) = SLB_WRITE_CMD;
	
	SLB_UNLOCK;
	return;
}


	
