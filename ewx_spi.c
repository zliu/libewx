#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif
#include "ewx_slb.h"

#define FPGA_SPI_RW 0x8

enum
{
	BIT_MSCH1_SS_L,
	BIT_MSCH0_SS_L,
	BIT_SCK,
	BIT_MOSI,
	BIT_MISO,
//	BIT_SS_N,
};
int SysHwInit()
{
	return 0;
}

int SysHwFini()
{
	return 0;
}

static int SysHWReadReg16(int reg)
{
    return ewx_slb_subc_read(0, 1, reg);
}

static int SysHWWriteReg16(int reg,int result)
{
	ewx_slb_subc_write(0, 1, reg, result);
	return 0;
}
static void REG16_READ(int reg, int *result)
{
	*(result) = SysHWReadReg16(reg);
//	printf("read addr 0x%08x: 0x%08x\n",reg,*result);
}
static void REG16_WRITE(int reg, int result)
{
	SysHWWriteReg16(reg,/*(unsigned short)*/result);
//	printf("write addr 0x%08x: 0x%08x\n",reg,result);
	
}

int u_sleep(int n)
{
#if 1
	int i,j;
	for(i=0;i<n;i++)
	{
		for(j=0;j<10;j++);
	}
#endif
	//usleep(1*n);
//	usleep(10000*n);
	return 0;
}

unsigned char spi_send_byte(unsigned char data)
{
	int i;
	int reg;
	unsigned char ret_data = 0;

	for(i=7;i>=0;i--)
	{
		// SCK set 0   MOSI set
		REG16_READ(FPGA_SPI_RW, &reg);
		reg &= ~(1<<BIT_SCK);
		REG16_WRITE(FPGA_SPI_RW, reg);

 		u_sleep(1);

		reg = (reg & ~(1<<BIT_MOSI)) + (((data>>(i))&0x1)<<BIT_MOSI);
		REG16_WRITE(FPGA_SPI_RW, reg);

 		u_sleep(1);
 		
		// SCK set 1
		reg |= (1<<BIT_SCK);
		REG16_WRITE(FPGA_SPI_RW, reg);

		// get MISO
		REG16_READ(FPGA_SPI_RW, &reg);
		ret_data += (((reg>>BIT_MISO)&0x1)<<i);

		u_sleep(1);
	}

	u_sleep(1);
//	printf("send:0x%02x, recv:0x%02x\n",data, ret_data);
	return ret_data;
}

unsigned long long ewx_5324_reg_read(int cs, int page, int addr, int size)
{
	int i;
	unsigned long long value=0;
	int ret=0;
	int reg;
	int bit_ss_n;

	cs = cs & 0x1;
	bit_ss_n = (cs == 0)?BIT_MSCH0_SS_L:BIT_MSCH1_SS_L;

// SSEL = 0
	REG16_READ(FPGA_SPI_RW, &reg);
	reg &= ~(1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);
// read page
	ret = spi_send_byte(0x60);
	ret = spi_send_byte(0xff);
	ret = spi_send_byte('?');
// SSEL = 1
	REG16_READ(FPGA_SPI_RW, &reg);
	reg |= (1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);

// SSEL = 0
	REG16_READ(FPGA_SPI_RW, &reg);
	reg &= ~(1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);
// write page
	ret = spi_send_byte(0x61);
	ret = spi_send_byte(0xff);
	ret = spi_send_byte(page);
// SSEL = 1
	REG16_READ(FPGA_SPI_RW, &reg);
	reg |= (1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);

// SSEL = 0
	REG16_READ(FPGA_SPI_RW, &reg);
	reg &= ~(1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);
// read reg addr
	ret = spi_send_byte(0x60);
	ret = spi_send_byte(addr);
	for(i=0;i<size;i++)
		ret = spi_send_byte('?');
// SSEL = 1
	REG16_READ(FPGA_SPI_RW, &reg);
	reg |= (1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);

// SSEL = 0
	REG16_READ(FPGA_SPI_RW, &reg);
	reg &= ~(1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);
// check rack
	ret = spi_send_byte(0x60);
	ret = spi_send_byte(0xf0/*+i*/);
	for(i=0;i<size;i++)
	{
		// read data
		ret = spi_send_byte('?');
		value += (ret << (8*i));
	}
// SSEL = 1 
	REG16_READ(FPGA_SPI_RW, &reg);
	reg |= (1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);

	return value;
}
int ewx_5324_reg_write(int cs, int page, int addr, char* value, int len)
{
	int reg, ret, i;
	int bit_ss_n;
	
	cs = cs & 0x1;
	bit_ss_n = (cs == 0)?BIT_MSCH0_SS_L:BIT_MSCH1_SS_L;

// SSEL = 0
	REG16_READ(FPGA_SPI_RW, &reg);
reg &= ~(1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);
// read page
	ret = spi_send_byte(0x60);
	ret = spi_send_byte(0xff);
	ret = spi_send_byte('?');
// SSEL = 1
	REG16_READ(FPGA_SPI_RW, &reg);
	reg |= (1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);

// SSEL = 0
	REG16_READ(FPGA_SPI_RW, &reg);
	reg &= ~(1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);
// write page
	ret = spi_send_byte(0x61);
	ret = spi_send_byte(0xff);
	ret = spi_send_byte(page);
// SSEL = 1
	REG16_READ(FPGA_SPI_RW, &reg);
	reg |= (1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);

// SSEL = 0
	REG16_READ(FPGA_SPI_RW, &reg);
	reg &= ~(1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);
// write
	ret = spi_send_byte(0x61);
	ret = spi_send_byte(addr);
	for (i = 0; i < len; i++)
		ret = spi_send_byte(value[i]);
// SSEL = 1
	REG16_READ(FPGA_SPI_RW, &reg);
	reg |= (1<<bit_ss_n);
	REG16_WRITE(FPGA_SPI_RW, reg);
	return 0;
}

