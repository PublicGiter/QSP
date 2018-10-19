#ifndef __SYSTIME_H_
#define __SYSTIME_H_

#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/* get system time ��ȡϵͳʱ�� */
void itimeofday(long *sec, long *usec);

/* get clock in millisecond 64 ��ȡʱ�䣨���룩 */
IINT64 iclock64(void);

IUINT32 iclock();

/* sleep in millisecond ˯�ߣ����룩 */
void isleep(unsigned long millisecond);

#ifdef __cplusplus
}
#endif

#endif
