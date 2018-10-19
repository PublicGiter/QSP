/**********************************************************
 *
 * Author        : zhangzhongyuan
 * Email         : zzyservers@163.com
 * Create time   : 2018-08-09 14:23
 * Last modified : 2018-08-09 14:23
 * Filename      : qsp.h
 * Description   : Quick Stable Protocol(QSP)
 *
 * *******************************************************/


#ifndef __QSP_H_
#define __QSP_H_

#include "log.h"
#include "queue.h"
#include "network.h"
#include "typedef.h"
#include "allocator.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------
//	QSP BASE CONFIG DEFINE
//--------------------------------------------------

#define QSP_MTU_SIZE 1400		// �ύ���²�Э���MTU��С
#define QSP_HEAD_SIZE (IOFFSETOF(QSPSEG, data) - IOFFSETOF(QSPSEG, conv))	// ����ͷ����С
#define QSP_BUF_SIZE (QSP_MTU_SIZE * 3)	// ��������С
#define QSP_PASS_NUM 2			// �����������Ƭ�Σ�����Ϊ�Ƕ����������ز����ģ��ǵ���ģʽ��

#define QSP_VERSION 65535		// QSPЭ��汾��16bit
#define QSP_TIME_OUT 200		// ACK��ʱ�������λ������
#define QSP_SINGLE_NUM 3		// ����ģʽ�£�ÿ������Ƭ�η���3��

#define QSP_CMD_PUSH 81			// cmd: push (��������)
#define QSP_CMD_ACK 82			// cmd: ack (AKCӦ��)
#define QSP_CMD_AGAIN 83		// cmd: again (Ҫ���ش�)

#define QSP_MODE_HALF 91		// mode: half (��˫��)�����Իظ���������
#define QSP_MODE_WEAK 92		// mode: weak (΢˫��)��ֻ�ܻظ�һ���ֽڵ�����
#define QSP_MODE_SINGLE 93		// mode: single (����)�����ظ��κ�����


//--------------------------------------------------
//	QSP TYPE DEFINE
//--------------------------------------------------

// head size 24
struct QSPSEG
{
	IUINT32 conv;			//�Ự��ţ����ı�ʶ��
	IUINT32 frg;			//����Ƭ�����fragment
	IUINT32 ts;				//ʱ���time stamp
	IUINT32 sn;				//����Ƭ����������seg num��
	IUINT16 cmd;			//���Ĵ�������push/ack/again��
	IUINT16 mode;			//ͨ��ģʽ��half/weak/single����weak�������������Ϊ��(MSS - HEAD_SIZE) * 256��
	IUINT16 ver;			//�汾�����ֵ��65535��
	IUINT16 len;			//data���ݵĳ���

	char data[1];			//���ݶΣ���len�������öεĴ�С
};

struct QSPNODE
{
	struct IQUEUEHEAD node;
	IUINT32  ts;				//time stampʱ��������룩
	struct QSPSEG seg;			//segment���ģ��ɱ䳤�ȣ�
};

struct QSP
{
	//�Ự��� / MTU������䵥Ԫ�� / MSS������ĳ��ȣ� / ͨ��ģʽ / �汾
	IUINT32 conv, mtu, mss, mode, ver;
	//��һ�������͵İ��� / ��һ�������յİ���
	IUINT32 snd_nxt, rcv_nxt;

	IUINT32 nrcv_que, nsnd_que;		// rcv_queue�еĽڵ�����snd_queue�еĽڵ���
	IUINT32 nrcv_buf, nsnd_buf;		// rcv_buf�еĽڵ�����snd_buf�еĽڵ���

	struct IQUEUEHEAD snd_queue;	// ����Ƭ�ı��ĵĶ���
	struct IQUEUEHEAD rcv_queue;	// ������Ƭ�ı��Ķ��У�����
	struct IQUEUEHEAD snd_buf;		// ���ͱ��ı��浽buf���У��ȴ�ack
	struct IQUEUEHEAD rcv_buf;		// ���ձ�����Ƭ��buf���У�������������������Ƭ�η���ö���

	void *user;						// �û���ַ������input��output�ص�������ʹ��
	void *buff;						// buffer������
	struct QSPSEG last;				// ������һ������Ƭ��״̬���ж��Ƿ����ظ�

	IUINT32(*systime)(void);		// ��ȡϵͳʱ��Ļص�����������ʱ�������λ�����룩
	int(*input)(char *buf, int len, struct QSP *kcp, void *user);			// ��������
	int(*output)(const char *buf, int len, struct QSP *kcp, void *user);	// �������
};

typedef struct QSP QSP;
typedef struct QSPSEG QSPSEG;
typedef struct QSPNODE QSPNODE;


//--------------------------------------------------
//	USER INTERFACE
//--------------------------------------------------

QSP* qsp_create(IUINT32 conv, void *user);
int qsp_release(QSP *qsp);
int qsp_version(QSP *qsp);
int qsp_send(QSP *qsp, const void *buf, int len);
int qsp_recv(QSP *qsp, void *buf, int len);

int qsp_setinput(QSP *qsp, int(*input)(char *buf, int len, QSP *qsp, void *user));
int qsp_setoutput(QSP *qsp, int(*output)(const char *buf, int len, QSP *qsp, void *user));
int qsp_setsystime(QSP *qsp, IUINT32(*systime)(void));
int qsp_setmode(QSP *qsp, IUINT32 mode);
void qsp_print(struct IQUEUEHEAD *head);

#ifdef __cplusplus
}
#endif

#endif // !__QSP_H_
