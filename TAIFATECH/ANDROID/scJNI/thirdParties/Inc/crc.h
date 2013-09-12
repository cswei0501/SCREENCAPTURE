#ifndef CRC_H
#define CRC_H

#include "capture_type.h"

/*
WiMo采用CRC-32算法对消息进行校验，算法采用的多项式为：
x32 + x26 + x23 + x22 + x16 + x12 + x11 + x10 + x8 + x7 + x5 + x4 + x2 + x + 1
都是x的次方
其中：初始值为0xffffffff，最终异或值为：0xffffffff。
*/

#ifdef __cplusplus
extern "C" {
#endif
/*Append 4 Bytes CRC Code. The variable "pbuf_len" refers to the whole length of the WiMo message.*/
void append_crc_32(Byte *pbuf, Dword pbuf_len);
/*Check 4 Bytes CRC Code.*/
int check_crc_32(Byte *pbuf, Dword pbuf_len);
#ifdef __cplusplus
}
#endif

#endif //CRC_H
