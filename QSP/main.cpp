#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include <string.h>
using namespace std;

#include "qsp.h"
#include "systime.h"
#include "wrap.h"

#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <windows.h>
#elif !defined(__unix)
#define __unix
#endif
#ifdef __unix
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#endif

#include <list>
#include <vector>

// ģ������
#if 0
// ���ӳٵ����ݰ�
class DelayPacket
{
public:
	virtual ~DelayPacket() {
		if (_ptr) delete _ptr;
		_ptr = NULL;
	}

	DelayPacket(int size, const void *src = NULL) {
		_ptr = new unsigned char[size];
		_size = size;
		if (src) {
			memcpy(_ptr, src, size);
		}
	}

	unsigned char* ptr() { return _ptr; }
	const unsigned char* ptr() const { return _ptr; }

	int size() const { return _size; }
	IUINT32 ts() const { return _ts; }
	void setts(IUINT32 ts) { _ts = ts; }

protected:
	unsigned char *_ptr;
	int _size;
	IUINT32 _ts;
};

// ���ȷֲ��������
class Random
{
public:
	Random(int size) {
		this->size = 0;
		seeds.resize(size);
	}

	int random() {
		int x, i;
		if (seeds.size() == 0) return 0;
		if (size == 0) {
			for (i = 0; i < (int)seeds.size(); i++) {
				seeds[i] = i;
			}
			size = (int)seeds.size();
		}
		i = rand() % size;
		x = seeds[i];
		seeds[i] = seeds[--size];
		return x;
	}

protected:
	int size;
	std::vector<int> seeds;
};

// �����ӳ�ģ����
class LatencySimulator
{
public:

	virtual ~LatencySimulator() {
		clear();
	}

	// lostrate: ����һ�ܶ����ʵİٷֱȣ�Ĭ�� 10%
	// rttmin��rtt��Сֵ��Ĭ�� 60
	// rttmax��rtt���ֵ��Ĭ�� 125
	LatencySimulator(int lostrate = 10, int rttmin = 60, int rttmax = 125, int nmax = 1000) :
		r12(100), r21(100) {
		current = iclock();
		this->lostrate = lostrate / 2;	// �������������������ʣ����̳���2
		this->rttmin = rttmin / 2;
		this->rttmax = rttmax / 2;
		this->nmax = nmax;
		tx1 = tx2 = 0;
	}

	// �������
	void clear() {
		DelayTunnel::iterator it;
		for (it = p12.begin(); it != p12.end(); it++) {
			delete *it;
		}
		for (it = p21.begin(); it != p21.end(); it++) {
			delete *it;
		}
		p12.clear();
		p21.clear();
	}

	// ��������
	// peer - �˵�0/1����0���ͣ���1���գ���1���ʹ�0����
	void send(int peer, const void *data, int size) {
		if (peer == 0) {
			tx1++;
			if (r12.random() < lostrate) return;
			if ((int)p12.size() >= nmax) return;
		}
		else {
			tx2++;
			if (r21.random() < lostrate) return;
			if ((int)p21.size() >= nmax) return;
		}
		DelayPacket *pkt = new DelayPacket(size, data);
		current = iclock();
		IUINT32 delay = rttmin;
		if (rttmax > rttmin) delay += rand() % (rttmax - rttmin);
		pkt->setts(current + delay);
		if (peer == 0) {
			p12.push_back(pkt);
		}
		else {
			p21.push_back(pkt);
		}
	}

	// ��������
	int recv(int peer, void *data, int maxsize) {
		DelayTunnel::iterator it;
		if (peer == 0) {
			it = p21.begin();
			if (p21.size() == 0) return -1;
		}
		else {
			it = p12.begin();
			if (p12.size() == 0) return -1;
		}
		DelayPacket *pkt = *it;
		current = iclock();
		if (current < pkt->ts()) return -2;
		if (maxsize < pkt->size()) return -3;
		if (peer == 0) {
			p21.erase(it);
		}
		else {
			p12.erase(it);
		}
		maxsize = pkt->size();
		memcpy(data, pkt->ptr(), maxsize);
		delete pkt;
		return maxsize;
	}

public:
	int tx1;
	int tx2;

protected:
	IUINT32 current;
	int lostrate;
	int rttmin;
	int rttmax;
	int nmax;
	typedef std::list<DelayPacket*> DelayTunnel;
	DelayTunnel p12;
	DelayTunnel p21;
	Random r12;
	Random r21;
};

// ģ������
LatencySimulator *vnet;

// ģ�����磺ģ�ⷢ��һ�� udp��
int udp_output(const char *buf, int len, QSP *qsp, void *user)
{
	union { int id; void *ptr; } parameter;
	parameter.ptr = user;
	vnet->send(parameter.id, buf, len);
	return len;
}
// ģ�����磺ģ�����һ�� udp��
int udp_input(char *buf, int len, QSP *qsp, void *user)
{
	union { int id; void *ptr; } parameter;
	parameter.ptr = user;

	return vnet->recv(parameter.id, buf, len);
}


void test(int mode)
{
	// ����ģ�����磺������10%��Rtt 60ms~125ms
	vnet = new LatencySimulator(10, 60, 125);

	// ���������˵�� kcp���󣬵�һ������ conv�ǻỰ��ţ�ͬһ���Ự��Ҫ��ͬ
	// ���һ���� user�������������ݱ�ʶ
	QSP *qsp1 = qsp_create(0x11223344, (void*)0);
	QSP *qsp2 = qsp_create(0x11223344, (void*)1);

	// ����kcp���²����������Ϊ udp_output��ģ��udp�����������
	qsp1->input = udp_input;
	qsp2->input = udp_input;
	qsp1->output = udp_output;
	qsp2->output = udp_output;
	qsp1->systime = iclock;
	qsp2->systime = iclock;

	IUINT32 current = iclock();
	IUINT32 slap = current - 100;
	IUINT32 index = 0;
	IUINT32 next = 0;
	IINT64 sumrtt = 0;
	int count = 0;
	int maxrtt = 0;

	// ���ô��ڴ�С��ƽ���ӳ�200ms��ÿ20ms����һ������
	// �����ǵ������ط�����������շ�����Ϊ128
	//ikcp_wndsize(kcp1, 128, 128);
	//ikcp_wndsize(kcp2, 128, 128);

#if 0
	// �жϲ���������ģʽ
	if (mode == 0) {
		// Ĭ��ģʽ
		ikcp_nodelay(kcp1, 0, 10, 0, 0);
		ikcp_nodelay(kcp2, 0, 10, 0, 0);
	}
	else if (mode == 1) {
		// ��ͨģʽ���ر����ص�
		ikcp_nodelay(kcp1, 0, 10, 0, 1);
		ikcp_nodelay(kcp2, 0, 10, 0, 1);
	}
	else {
		// ��������ģʽ
		// �ڶ������� nodelay-�����Ժ����ɳ�����ٽ�����
		// ���������� intervalΪ�ڲ�����ʱ�ӣ�Ĭ������Ϊ 10ms
		// ���ĸ����� resendΪ�����ش�ָ�꣬����Ϊ2
		// ��������� Ϊ�Ƿ���ó������أ������ֹ
		ikcp_nodelay(kcp1, 1, 10, 2, 1);
		ikcp_nodelay(kcp2, 1, 10, 2, 1);
		kcp1->rx_minrto = 10;
		kcp1->fastresend = 1;
	}
#endif

	char buffer[2000];
	int hr;

	IUINT32 ts1 = iclock();

	while (1) {
		cout << "----" << endl;
		isleep(1);
		current = iclock();
		//ikcp_update(kcp1, iclock());
		//ikcp_update(kcp2, iclock());

		// ÿ�� 20ms��kcp1��������
		for (; current >= slap; slap += 20) {
			((IUINT32*)buffer)[0] = index++;
			((IUINT32*)buffer)[1] = current;

			// �����ϲ�Э���
			qsp_send(qsp1, buffer, 8);
			qsp_print(&qsp1->snd_queue);
			sleep(20);
		}

		// �����������磺����Ƿ���udp����p1->p2
		while (1) {
			hr = qsp_recv(qsp2, buffer, 2000);
			//hr = vnet->recv(1, buffer, 2000);
			if (hr < 0) break;
			// ��� p2�յ�udp������Ϊ�²�Э�����뵽kcp2
			//ikcp_input(kcp2, buffer, hr);
		}

		// �����������磺����Ƿ���udp����p2->p1
		while (1) {
			hr = qsp_recv(qsp1, buffer, 2000);
			//hr = vnet->recv(0, buffer, 2000);
			if (hr < 0) break;
			// ��� p1�յ�udp������Ϊ�²�Э�����뵽kcp1
			//ikcp_input(kcp1, buffer, hr);
		}

		// kcp2���յ��κΰ������ػ�ȥ
		while (1) {
			hr = qsp_recv(qsp2, buffer, 10);
			// û���յ������˳�
			if (hr < 0) break;
			// ����յ����ͻ���
			qsp_send(qsp2, buffer, hr);
		}

		// kcp1�յ�kcp2�Ļ�������
		while (1) {
			hr = qsp_recv(qsp1, buffer, 10);
			// û���յ������˳�
			if (hr < 0) break;
			IUINT32 sn = *(IUINT32*)(buffer + 0);
			IUINT32 ts = *(IUINT32*)(buffer + 4);
			IUINT32 rtt = current - ts;

			if (sn != next) {
				// ����յ��İ�������
				printf("ERROR sn %d<->%d\n", (int)count, (int)next);
				return;
			}

			next++;
			sumrtt += rtt;
			count++;
			if (rtt > (IUINT32)maxrtt) maxrtt = rtt;

			printf("[RECV] mode=%d sn=%d rtt=%d\n", mode, (int)sn, (int)rtt);
		}
		if (next > 1000) break;
	}

	ts1 = iclock() - ts1;

	qsp_release(qsp1);
	qsp_release(qsp2);

	const char *names[3] = { "default", "normal", "fast" };
	printf("%s mode result (%dms):\n", names[mode], (int)ts1);
	printf("avgrtt=%d maxrtt=%d tx=%d\n", (int)(sumrtt / count), (int)maxrtt, (int)vnet->tx1);
	printf("press enter to next ...\n");
	char ch; scanf("%c", &ch);
}

#endif

// ��������
#if 0
void qsp_print(struct IQUEUEHEAD *head);

int output(const char *buf, int len, QSP *qsp, void *user)
{
	printf("[sendto]:%s\n", buf);
	return len;
}

int input(char *buf, int len, QSP *qsp, void *user)
{
	return 10;//fscanf(stdin, buf, len);
}
#endif

// Linux Socket
#if 1

struct user_info
{
	int fd;
	struct sockaddr_in addr;
	socklen_t addrlen;
};

struct test_data
{
	int id;
	unsigned long ctime;
};

void print_mem(const void *buf, int len)
{
	if (buf == NULL || len <= 0)
		return;

	for (int i = 0; i < len; i++)
	{
		printf("%02X", ((char*)buf)[i]);
	}
	printf("\n");
}

int udp_output(const char *buf, int len, QSP *qsp, void *user)
{
	qsp++;
	struct user_info * info = (struct user_info*)user;
	if (rand() % 100 < 0)
	{
		return len;
	}
	int ret = sendto(info->fd, buf, len, 0, (struct sockaddr*)&info->addr, sizeof(info->addr));
	//printf("sendto data : %d byte\n", ret);
	//printf("sendto data:\n");
	//print_mem(buf, ret);
	printf("[send][%s][send to is ok]\n", len == 24 || len == 1 ? "ACK" : "DATA");

	return ret;
}

int udp_input(char *buf, int len, QSP *qsp, void *user)
{
	qsp++;
	struct user_info * info = (struct user_info*)user;
	int ret = recvfrom(info->fd, buf, len, 0, (struct sockaddr*)&info->addr, &info->addrlen);
	//printf("recvfrom data : %d byte\n", ret);
	//printf("recvfrom data:\n");
	//print_mem(buf, ret);
	if (ret == -1 && errno == EAGAIN)
	{
		//isleep(1);
		return 0;
		for (int i = 0; i < rand() % 10 + 10; i++)
		{
			printf("--");
		}
		printf("\n");
		return 0;
	}

	printf("[recv][%s][recv from is ok]\n", ret == 24 || ret == 1 ? "ACK" : "DATA");
	return ret;
}

int init_socket()
{
	int fd;

	fd = Socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8989);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(fd, (struct sockaddr*)&addr, sizeof(addr));

	return fd;
}

int server()
{
	int fd = init_socket();

	struct sockaddr_in caddr;
	socklen_t addrlen = sizeof(caddr);

	char buf[1024];

	struct user_info user;
	user.fd = fd;
	user.addrlen = sizeof(user.addr);

	QSP *qsp = qsp_create(0xaabbccdd, &user);
	qsp_setinput(qsp, udp_input);
	qsp_setoutput(qsp, udp_output);
	qsp_setsystime(qsp, iclock);
	qsp_setmode(qsp, QSP_MODE_WEAK);

	int flag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag | O_NONBLOCK);

	int ret = -1;

	while (1)
	{
		bzero(buf, 1024);

		//ret = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&caddr, &addrlen);
		//if (ret < 0 && errno != EAGAIN)
		//	perror("recvfrom error");
		ret = qsp_recv(qsp, buf, sizeof(buf));
		if (ret < 0)
			perror("recvfrom error");

		printf("[recv package] : id=%d\n", *(int*)buf);

		//ret = sendto(fd, buf, ret, 0, (struct sockaddr*)&caddr, addrlen);
		//if (ret < 0 && errno != EAGAIN)
		//	perror("sento error");
		ret = qsp_send(qsp, buf, ret);
		if (ret < 0)
			perror("sento error");
	}

	return 0;
}

int client()
{
	int fd, ret;

	fd = Socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in s_addr;
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(8989);
	s_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	struct user_info user;
	user.fd = fd;
	user.addr = s_addr;
	user.addrlen = sizeof(user.addr);

	QSP *qsp = qsp_create(0xaabbccdd, &user);
	qsp_setinput(qsp, udp_input);
	qsp_setoutput(qsp, udp_output);
	qsp_setsystime(qsp, iclock);
	qsp_setmode(qsp, QSP_MODE_WEAK);

	struct test_data data, rdata;
	data.id = 0;
	data.ctime = iclock();

	char buf[1024];

	int flag = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flag | O_NONBLOCK);

	IUINT32 lastTime = iclock();

	while (1)
	{
		data.id++;
		data.ctime = iclock();
		if (data.id > 10000)
			break;
			//data.id = 1;

		//ret = sendto(fd, &data, sizeof(data), 0, (struct sockaddr*)&s_addr, sizeof(s_addr));
		//if (ret < 0)
		//	perror("sendto error");
		ret = qsp_send(qsp, &data, sizeof(data));
		if (ret < 0)
			perror("sendto error");

		memset(buf, 0, sizeof(buf));
		//ret = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
		//if (ret < 0 && errno != EAGAIN)
		//	perror("recvfrom error");
		ret = qsp_recv(qsp, buf, sizeof(buf));
		if (ret < 0 && errno != EAGAIN)
			perror("recvfrom error");

		memcpy(&rdata, buf, sizeof(rdata));

		IUINT32 curr_time = iclock();
		printf("[send pack] : id = %d send_time = %lu recv_time = %lu rtt = %lu\n", rdata.id, data.ctime, curr_time, curr_time - rdata.ctime);

		//printf("Please continue!\n");
		//scanf("%s", buf);
		//usleep(1000000);    // 1s = 1000000us = 1000ms
		//usleep(10000);    // 1s = 1000000us = 1000ms
	}

	printf("all use time : %d\n", iclock() - lastTime);

	return 0;
}


void udp_test()
{
	srand((unsigned int)time(NULL));

	int num;

	do
	{
		cout << "please select run: " << endl;
		cout << "1.server" << endl;
		cout << "2.client" << endl;
		cin >> num;
	} while (num < 1 || num > 2);

	switch (num)
	{
	case 1:server(); break;
	case 2:client(); break;
	default:
		break;
	}
}

#endif

int main()
{
	//test();
	//test(QSP_MODE_HALF);

	udp_test();

	//QSP * qsp = qsp_create(0xaabbccdd, NULL);
	//qsp_setoutput(qsp, output);
	//qsp_setinput(qsp, input);
	//qsp_setsystime(qsp, iclock);

	//char buf[4096] = "hello world+++++++++++++++++++++++++++++++++++++++++++++++++";

	//qsp_send(qsp, buf, sizeof(buf));
	//qsp_print(&qsp->snd_queue);

	getchar();
	return EXIT_SUCCESS;
}