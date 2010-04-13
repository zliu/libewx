#ifndef __EWX_SLB_H__
#define __EWX_SLB_H__


#include <cvmx.h>
#include "ewx_helper.h"

/**@addtogroup slb_function slb操作函数
 * @ingroup hardware_function
 */
//@{
/** 
 * 本板slb读
 * 
 * @param device_id 设备ID，现在设备号都为0
 * @param address 读的地址
 * 
 * @return 本板slb读取的值
 */
uint16_t ewx_slb_local_read(uint8_t device_id, uint64_t address);

/** 
 * 本板slb写
 * 
 * @param device_id 设备ID，现在设备号都为0
 * @param address 写的地址
 * @param value 本板slb写的值
 */
void ewx_slb_local_write(uint8_t device_id, uint64_t address, uint16_t value);

/** 
 * 载板slb读操作
 * 
 * @param device_id 设备ID，现在设备号都为0 
 * @param sub_mode 器件选择
 * @param address 要读取的地址
 * 
 * @return 通过slb读取的值
 */
uint16_t ewx_slb_subc_read(uint16_t device_id,  uint8_t sub_mode, uint16_t address);

/** 
 * 载板slb写操作
 * 
 * @param device_id 设备ID，现在设备号都为0 
 * @param sub_mode 器件选择
 * @param address 要写入的地址
 * @param value 要写入的值
 */
void ewx_slb_subc_write(uint16_t device_id, uint8_t sub_mode, uint16_t address, uint16_t value);
//@}

#endif
