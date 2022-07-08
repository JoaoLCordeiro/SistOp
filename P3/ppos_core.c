#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

//#define DEBUG

//DEFINES STATUS/////////////////////////////

#define PRONTA		0
#define TERMINADA	1
#define SUSPENSA	2

/////////////////////////////////////////////

//VARIÁVEIS GLOBAIS//////////////////////////

task_t* current_task;
task_t* main_task;
int 	current_id	= 1;

queue_t*	fila_tasks;
int			user_tasks;

task_t	dispatcher_task;

/////////////////////////////////////////////

task_t* scheduler(){
	//se a fila nao possuir elementos
	if (fila_tasks == NULL)
		return (NULL);

	//passa o inicio para o proximo
	task_t* aux	= (task_t *) fila_tasks;
	fila_tasks	= fila_tasks->next;

	//retorna o antigo atual
	return (aux);
}

void dispatcher(){
	//enquanto tivermos tarefas na fila de tarefas
	while (user_tasks > 0){
		task_t* prox_task = scheduler();

		if (prox_task != NULL){
			task_switch (prox_task);

			switch (prox_task->status){
				case (PRONTA):

					break;

				case (TERMINADA):
					//retira a tarefa da fila de tarefas e dá free na estrutura
					queue_remove (&fila_tasks, (queue_t *) prox_task);

					free (prox_task->context.uc_stack.ss_sp);
					break;
			}

		}

	}

	task_exit(0);
}

void ppos_init (){
	setvbuf (stdout, 0, _IONBF, 0);

	ucontext_t main_context;
	getcontext (&main_context);

	main_task = malloc (sizeof(task_t));

	main_task->id		= 0;
	main_task->context	= main_context;
	main_task->status	= PRONTA; 
	//main_task->preemptable = 

	current_task = main_task;

	//coloca 1 na current_id novamente só pra garantir
	current_id = 1;

	task_create(&dispatcher_task, (void *) dispatcher, NULL);
	user_tasks = 0;

	//testa se a tarefa dispatcher foi criada
	/*if (dispatcher_task == NULL){
		fprintf (stderr, "Erro no ppos_init: erro ao criar tarefa dispatcher\n");
	}*/

	#ifdef DEBUG
	printf ("ppos_init: iniciou o ppos com o id da main: %d\n", main_task->id) ;
	#endif
}

int task_create (task_t *task,
                 void (*start_func)(void *),
                 void *arg){
	
	//testa se a task existe
	if (task == NULL){
		fprintf (stderr, "Erro na criacao de task: task inexistente\n");
		return (-1);
	}

	//testa se a funcao existe
	if (start_func == NULL){
		fprintf (stderr, "Erro na criacao de task: funcao inexistente\n");
		return (-1);
	}

	//VERIFICAR MAIS COISAS

	//cria um contexto pra funcao desejada
	ucontext_t new_context;
	getcontext (&new_context);

	char *stack = malloc (STACKSIZE);

	if (stack != NULL){
		new_context.uc_stack.ss_sp		= stack;
    	new_context.uc_stack.ss_size	= STACKSIZE;
    	new_context.uc_stack.ss_flags	= 0;
		new_context.uc_link				= 0;
	}
	else{
		fprintf (stderr, "Erro na criacao de task: criacao da pilha nao foi possivel\n");
		return (-4);
	}

	makecontext(&new_context, (void *) start_func, 1, arg);


	//coloca as informacoes da task na estrutura
	task->id			= current_id;
	task->context		= new_context;
	task->status		= PRONTA;
	//task->preemptable	=

	current_id++;

	//se a tarefa nao for o dispatcher, adiciona na fila de tarefas
	if (task->id > 1){
		user_tasks++;
		queue_append ((queue_t **) &fila_tasks, (queue_t *) task);
	}

	#ifdef DEBUG
	printf ("task_create: criou a tarefa %d\n", task->id);
	#endif

	return (0);
}

void task_exit (int exit_code){
	#ifdef DEBUG
	printf ("task_exit: a task %d deu exit\n", current_task->id);
	#endif

	//termina a task atual
	current_task->status	= TERMINADA;

	if (current_task->id > 1){
	//retorno para o dispatcher terminar a tarefa
		user_tasks--;
		task_switch (&dispatcher_task);
	}
	//quando a funcao a dar exit é o próprio dispatcher
	else if (current_task->id == 1){
		task_switch (main_task);
	}
}

int task_switch (task_t *task){
	//testa se a tarefa existe
	if (task == NULL){
		fprintf (stderr, "Erro na mudança de task: task inexistente\n");
		return (-1);
	}

	//testa se o id da task é válido
	if (task->id < 0){
		fprintf (stderr, "Erro na mudança de task: id da task inválido\n");
		return (-2);
	}

	//INSERIR AQUI MAIS VERIFICAÇÕES

	//transforma a task que será executada em task atual (current_task)
	task_t* aux_task = current_task;
	current_task = task;

	#ifdef DEBUG
	printf ("task_switch: trocou da tarefa %d para a tarefa %d\n", aux_task->id, current_task->id);
	#endif

	//faz a troca de contexto
	swapcontext (&(aux_task->context), &(current_task->context));
	return (0);
}

int task_id (){
	if (current_task == NULL){
		fprintf (stderr, "Erro na consulta de id: task inválida\n");
		return (-1);
	}

	return (current_task->id);
}

void task_yield (){
	//só isso? hmm
	task_switch(&dispatcher_task);
}