#ifndef THREADS_SYNCH_H
#define THREADS_SYNCH_H

#include <list.h>
#include <stdbool.h>

/* A counting semaphore. */
struct semaphore {
	unsigned value;             /* Current value. */
	struct list waiters;        /* List of waiting threads. */
};

void sema_init (struct semaphore *, unsigned value);       // 주어진 초기값 사용하여, 새로운 세마포어 생성
void sema_down (struct semaphore *);                       // 세마포어의 값이 양수가 될 때까지 기다린 다음, 그 값을 1만큼 감소
bool sema_try_down (struct semaphore *);                   // 값이 성공적으로 감소 = true
void sema_up (struct semaphore *);                         // 세마포어 값 증가 -> 외부 인터럽트 핸들러 내에서도 호출 가능 
void sema_self_test (void);

/* Lock. */
struct lock {
	struct thread *holder;      /* Thread holding lock (for debugging). */
	struct semaphore semaphore; /* Binary semaphore controlling access. */
};

void lock_init (struct lock *);                           // 락을 새로운 상태로 초기화 -> 처음에는 어떤 스레드도 그 락을 소유하고 있지 않음
void lock_acquire (struct lock *);                        // 현재 스레드가 락을 획득  
bool lock_try_acquire (struct lock *);                    // 현재 스레드가 사용할 수 있도록 락을 획득하려고 시도
void lock_release (struct lock *);                        // 현재 스레드가 소유하고 있는 락을 해제 
bool lock_held_by_current_thread (const struct lock *);

/* Condition variable. */
struct condition {
	struct list waiters;        /* List of waiting threads. */
};

void cond_init (struct condition *);
void cond_wait (struct condition *, struct lock *);
void cond_signal (struct condition *, struct lock *);
void cond_broadcast (struct condition *, struct lock *);

/* Optimization barrier.
 *
 * The compiler will not reorder operations across an
 * optimization barrier.  See "Optimization Barriers" in the
 * reference guide for more information.*/
#define barrier() asm volatile ("" : : : "memory")

#endif /* threads/synch.h */
