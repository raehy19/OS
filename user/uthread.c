#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/uthread.h"

int thread_create(thread_t *t, void *(*func)(void *), void *arg) {

	int tid = tfork(func, arg);
	*t = tid;
	return !tid;
}

int thread_join(thread_t t, void **ret) {
	return -1;
}

void thread_exit(void *ret) {
	exit(1);
}

void thread_mutex_init(thread_mutex_t *mtx) {
	mtx->locked = 0;

}

void thread_mutex_destroy(thread_mutex_t *mtx) {

}

void thread_mutex_lock(thread_mutex_t *mtx) {
}

void thread_mutex_unlock(thread_mutex_t *mtx) {
	mtx->locked = 0;
}
