#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"
#include "ppos_core.h"

#include <stdio.h>
#include <stdlib.h>

#define NPRODUTOS	60
#define NCONSUMIDOR	40

semaphore_t s_vaga;
semaphore_t s_buffer;
semaphore_t s_item;

queue_t *buffer;

int args[5];

task_t prod1;
task_t prod2;
task_t cons1;
task_t cons2;
task_t cons3;

typedef struct int_q{
	struct queue_t *prev ;  // aponta para o elemento anterior na fila
	struct queue_t *next ;  // aponta para o elemento seguinte na fila
	int valor;
} int_q;

void produtor (void* num){
	for (int i = 1 ; i <= NPRODUTOS ; i++){
		int item = rand() % 100;

		sem_down (&s_vaga);

		sem_down (&s_buffer);

		int_q *elemento = malloc (sizeof(int_q));
		elemento->valor	= item;
		elemento->next	= NULL;
		elemento->prev	= NULL;
		queue_append (&buffer, (queue_t *) elemento);

		printf("Produtor %d:	Colocou o elemento %d\n", *((int*) num), item);

		sem_up (&s_buffer);
		sem_up (&s_item);

		task_sleep (250);
	}
	task_exit(0);
}

void consumidor (void* num){
	for (int i = 1 ; i <= NCONSUMIDOR ; i++){
		task_sleep (250);
		sem_down(&s_item);

		sem_down(&s_buffer);

		int item = ((int_q *) buffer)->valor;
		queue_remove (&buffer, buffer);

		printf("					Consumidor %d:	Retirou o elemento %d\n", *((int*) num), item);

		sem_up(&s_buffer);
		sem_up(&s_vaga);
	}
	task_exit(0);
}

int main(){
	ppos_init ();

	sem_create(&s_vaga	, 5);
	sem_create(&s_buffer, 1);
	sem_create(&s_item	, 0);

	printf ("Criou os três semáforos\n");

	args[0] = 1;
	args[1] = 2;
	args[2] = 1;
	args[3] = 2;
	args[4] = 3;

	task_create (&prod1, produtor	, (void*) &args[0]);
	printf ("Criou o produtor %d\n", args[0]);

	task_create (&prod2, produtor	, (void*) &args[1]);
	printf ("Criou o produtor %d\n", args[1]);

	task_create (&cons1, consumidor	, (void*) &args[2]);
	printf ("Criou o consumidor %d\n", args[2]);

	task_create (&cons2, consumidor	, (void*) &args[3]);
	printf ("Criou o consumidor %d\n", args[3]);

	task_create (&cons3, consumidor	, (void*) &args[4]);
	printf ("Criou o consumidor %d\n", args[4]);

	printf ("Esperando o produtor %d\n", args[0]);
    task_join (&prod1);

	printf ("Esperando o produtor %d\n", args[1]);
	task_join (&prod2);

	printf ("Esperando o consumidor %d\n", args[2]);
	task_join (&cons1);

	printf ("Esperando o consumidor %d\n", args[3]);
	task_join (&cons2);

	printf ("Esperando o consumidor %d\n", args[4]);
	task_join (&cons3);

	sem_destroy (&s_vaga	);
	sem_destroy (&s_buffer	);
	sem_destroy (&s_item	);

	task_exit (0) ;
	exit (0) ;

	return 0;
}