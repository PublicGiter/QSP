#ifndef __LOG_H_
#define __LOG_H_

// VS��������ֹ����
#ifdef _MSC_VER
#pragma warning(disable:4311)
#pragma warning(disable:4312)
#pragma warning(disable:4996)
#endif

// ����log����Ļص�����
void set_outlog(void(*write_log)(const char *log));

// ��ӡlog��־
void write_log(const char *fmt, ...);

#endif // !__LOG_H_
