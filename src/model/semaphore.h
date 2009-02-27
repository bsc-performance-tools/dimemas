#ifndef __semaphore_h
#define __semaphore_h
/**
 * External routines defined in file semaphore.c
 **/
extern void SEMAPHORE_general(int value, struct t_thread *thread);
extern void SEMAPHORE_init(void);
extern void SEMAPHORE_end(void);
extern void SEMAPHORE_signal(int sem_id, struct t_thread *thread);
extern void SEMAPHORE_wait(int sem_id, struct t_thread *thread);
extern void SEMAPHORE_signal_n(int sem_id, int n, struct t_thread *thread);

#endif
