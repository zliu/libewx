#ifndef _SPI_H_
#define _SPI_H_

/**@addtogroup spi_function spi操作函数
 * @ingroup hardware_function
 */
//@{
/** 
 * spi读函数
 * 
 * @param cs 片选 
 * @param page 页
 * @param addr 地址
 * @param size 大小
 * 
 * @return spi读取的值
 */
unsigned long long ewx_5324_reg_read(int cs, int page, int addr, int size);

/** 
 * spi写函数
 * 
 * @param cs 片选 
 * @param page 页
 * @param addr 地址
 * @param value 写入的值
 * @param len 大小
 * 
 * @return 0，成功
 */
int ewx_5324_reg_write(int cs, int page, int addr, char* value, int len);
//@}

#endif
