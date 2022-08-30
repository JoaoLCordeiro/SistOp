unsigned int systime();

void controla_tempo();

int inicia_timer_e_tratador();

void task_setprio (task_t *task, int prio);

int task_getprio (task_t *task);

task_t* scheduler();