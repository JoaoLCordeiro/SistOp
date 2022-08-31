#ifndef __PPOSCORE__
#define __PPOSCORE__

int mqueue_create (mqueue_t *queue, int max_msgs, int msg_size);

int mqueue_send (mqueue_t *queue, void *msg);

int mqueue_recv (mqueue_t *queue, void *msg);

int mqueue_destroy (mqueue_t *queue);

int mqueue_msgs (mqueue_t *queue);

int sem_count (semaphore_t *s);

int sem_create (semaphore_t *s, int value);

int sem_down (semaphore_t *s);

int sem_up (semaphore_t *s);

int sem_destroy (semaphore_t *s);

unsigned int systime();

void controla_tempo();

int inicia_timer_e_tratador();

void task_setprio (task_t *task, int prio);

int task_getprio (task_t *task);

void acorda_tarefas();

task_t* scheduler();

void dispatcher();

void ppos_init ();

void task_sleep (int t);

int task_join (task_t *task);

void task_suspend (task_t **queue);

void task_resume (task_t * task, task_t **queue);

int task_create (task_t *task, void (*start_func)(void *), void *arg);

void task_exit (int exit_code);

int task_switch (task_t *task);

int task_id ();

void task_yield ();

#endif