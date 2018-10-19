#ifndef __QUEUE_H_
#define __QUEUE_H_

//=====================================================================
// QUEUE DEFINITION    ���ж��� -  iqueue_head   
//=====================================================================

//=====================================================================
// iqueue_init		IQUEUE_INIT			��ʼ�����У���ʼ��Ϊ���ζ��У�
// iqueue_entry		IQUEUE_ENTRY		�ڵ��׵�ַ
// iqueue_add		IQUEUE_ADD			��head�ڵ�֮������½ڵ�
// iqueue_add_tail	IQUEUE_ADD_TAIL		��head�ڵ�֮ǰ�����½ڵ�(���ζ��У����뵽��β)
// iqueue_del		IQUEUE_DEL			ɾ���ýڵ㣬����prev��next�ڵ㣬���Ըýڵ�����
// iqueue_del_init	IQUEUE_DEL_INIT		ɾ���ýڵ㣬����prev��next�ڵ㣬���Ըýڵ��ʼ�������Σ�
// iqueue_is_empty	IQUEUE_IS_EMPTY		�жϽڵ��Ƿ���next�ڵ㣨�жϽڵ��nextָ���Ƿ����Լ���
//=====================================================================
// iqueue_foreach	IQUEUE_FOREACH		�����ö��У���ʽ��for(xxx; xxx; xxx)�����������Զ���ṹ��
// iqueue_foreach_entry					�����ö��У���ʽ��for(xxx; xxx; xxx)����ֻ�����ڶ��нڵ�����
// __iqueue_splice						��list�ֽ�ƴ�ӵ�head������
// iqueue_splice						���list��Ϊ�գ����list�ֽ�ƴ�ӵ�head������
// iqueue_splice_init					��listƴ�ӵ�head�ϣ�����list����ʼ��
//=====================================================================
#ifndef __IQUEUE_DEF__
#define __IQUEUE_DEF__

#ifdef __cplusplus
extern "C" {
#endif

struct IQUEUEHEAD {
	struct IQUEUEHEAD *next, *prev;
};

typedef struct IQUEUEHEAD iqueue_head;

//---------------------------------------------------------------------
// queue init                                                         
//---------------------------------------------------------------------
#define IQUEUE_HEAD_INIT(name) { &(name), &(name) }
#define IQUEUE_HEAD(name) \
	struct IQUEUEHEAD name = IQUEUE_HEAD_INIT(name)

// ��ʼ���ڵ㣬ʹ���Ϊ���ζ���
#define IQUEUE_INIT(ptr) ( \
	(ptr)->next = (ptr), (ptr)->prev = (ptr))

// ��ȡ�ṹ���г�Ա���׵�ַƫ����
#define IOFFSETOF(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

// ��ȡ���нڵ���׵�ַ(���ݽṹ���еĳ�Ա��ַ - �ṹ���г�Ա���׵�ַƫ���� = �ڵ��׵�ַ)
// ����node.next�ĵ�ַ�����my_type�ڵ���׵�ַ
// struct my_type
// {
// 	int a;
// 	int b;
// 	struct node *next;
// };
#define ICONTAINEROF(ptr, type, member) ( \
		(type*)( ((char*)((type*)ptr)) - IOFFSETOF(type, member)) )

// ��ȡ���нڵ���׵�ַ
#define IQUEUE_ENTRY(ptr, type, member) ICONTAINEROF(ptr, type, member)



//---------------------------------------------------------------------
// queue operation                     
//---------------------------------------------------------------------
// ��head�ڵ�֮������½ڵ�
#define IQUEUE_ADD(node, head) ( \
	(node)->prev = (head), (node)->next = (head)->next, \
	(head)->next->prev = (node), (head)->next = (node))

// ��head�ڵ�֮ǰ�����½ڵ�
#define IQUEUE_ADD_TAIL(node, head) ( \
	(node)->prev = (head)->prev, (node)->next = (head), \
	(head)->prev->next = (node), (head)->prev = (node))

// ɾ�������ڵ�֮��Ľڵ㣬���������ڵ�
#define IQUEUE_DEL_BETWEEN(p, n) ((n)->prev = (p), (p)->next = (n))

// ɾ���ýڵ㣬����prev��next�ڵ�
#define IQUEUE_DEL(entry) (\
	(entry)->next->prev = (entry)->prev, \
	(entry)->prev->next = (entry)->next, \
	(entry)->next = 0, (entry)->prev = 0)

// ɾ���ýڵ㣬���Ըýڵ���г�ʼ�������Σ�
#define IQUEUE_DEL_INIT(entry) do { \
	IQUEUE_DEL(entry); IQUEUE_INIT(entry); } while (0)

// �Ƿ�Ϊ�սڵ㣨�жϽڵ��next�Ƿ�ָ���Լ���
#define IQUEUE_IS_EMPTY(entry) ((entry) == (entry)->next)

#define iqueue_init		IQUEUE_INIT
#define iqueue_entry	IQUEUE_ENTRY
#define iqueue_add		IQUEUE_ADD
#define iqueue_add_tail	IQUEUE_ADD_TAIL
#define iqueue_del		IQUEUE_DEL
#define iqueue_del_init	IQUEUE_DEL_INIT
#define iqueue_is_empty IQUEUE_IS_EMPTY

// �����ö��У��������Զ���ṹ�壨�ṹ���к��ж��нڵ㣩����ʽ��for(xxx; xxx; xxx)��
#define IQUEUE_FOREACH(iterator, head, TYPE, MEMBER) \
	for ((iterator) = iqueue_entry((head)->next, TYPE, MEMBER); \
		&((iterator)->MEMBER) != (head); \
		(iterator) = iqueue_entry((iterator)->MEMBER.next, TYPE, MEMBER))

// �����ö��У��������Զ���ṹ�壨�ṹ���к��ж��нڵ㣩����ʽ��for(xxx; xxx; xxx)��
#define iqueue_foreach(iterator, head, TYPE, MEMBER) \
	IQUEUE_FOREACH(iterator, head, TYPE, MEMBER)

// �����ö��У�ֻ�����ڶ��нڵ�ı�����ֻ����ַ��������ʽ��for(xxx; xxx; xxx)��
#define iqueue_foreach_entry(pos, head) \
	for( (pos) = (head)->next; (pos) != (head) ; (pos) = (pos)->next )

// ƴ���������ζ��г�Ϊһ�����ζ��У���list�ֽ�ƴ�ӵ�head�����ϣ�
#define __iqueue_splice(list, head) do {	\
		iqueue_head *first = (list)->next, *last = (list)->prev; \
		iqueue_head *at = (head)->next; \
		(first)->prev = (head), (head)->next = (first);		\
		(last)->next = (at), (at)->prev = (last); }	while (0)

// �ж����list��Ϊ�գ���ƴ�ӵ�head��
#define iqueue_splice(list, head) do { \
	if (!iqueue_is_empty(list)) __iqueue_splice(list, head); } while (0)

// ��listƴ�ӵ�head�ϣ�����list����ʼ��
#define iqueue_splice_init(list, head) do {	\
	iqueue_splice(list, head);	iqueue_init(list); } while (0)


// VS��������ֹ����
#ifdef _MSC_VER
#pragma warning(disable:4311)
#pragma warning(disable:4312)
#pragma warning(disable:4996)
#endif // !_MSC_VER

#endif // !__IQUEUE_DEF__

#ifdef __cplusplus
}
#endif

#endif // !__QUEUE_H_
