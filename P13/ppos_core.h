#ifndef __PPOSCORE__
#define __PPOSCORE__

#include "ppos.h"
#include "queue.h"
#include "disk.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

//DEFINES ///////////////////////////////////

#define PRONTA		0
#define TERMINADA	1
#define SUSPENSA	2
#define ADORMECIDA	3

#define QUANTUM		20

/////////////////////////////////////////////

//VARIÁVEIS GLOBAIS//////////////////////////

task_t* current_task;
task_t	main_task;
int 	current_id;

queue_t*	fila_tasks_prontas;
queue_t*	fila_tasks_dormindo;
int			user_tasks;

//guarda a tarefa do dispatcher
task_t	dispatcher_task;

//define um tratador de sinal
struct sigaction action;

//estrutura de inicialização to timer
struct itimerval timer;

//variáveis que controlam tempo
int temporizador;
int tempo_sistema;

/////////////////////////////////////////////

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