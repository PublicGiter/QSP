#ifndef __ALLOCATOR_H_
#define __ALLOCATOR_H_

#ifdef __cplusplus
extern "C" {
#endif

// malloc/free���Ӻ�����Ĭ��Ϊ�գ�
static void* (*__malloc_hook)(size_t) = 0;
static void(*__free_hook)(void *) = 0;

// ���÷�����
#define set_allocator(malloc, free) \
	__malloc_hook=malloc,__free_hook=free

// ����ռ�
#define malloc_hook(size) \
	__malloc_hook ? __malloc_hook(size) : malloc(size)

// �ͷſռ�
#define free_hook(ptr) \
	__free_hook ? __free_hook(ptr) : free(ptr)

#ifdef __cplusplus
}
#endif

#endif // !__ALLOCATOR_H_
