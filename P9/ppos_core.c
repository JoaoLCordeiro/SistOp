//Nome: João Lucas Cordeiro			GRR:20190427

#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

//#define DEBUG
//#define DEBUGSWITCH
//#define DEBUGSCHEDULER
#define DEBUGSLEEP

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
int 	current_id	= 0;

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
int tempo_sistema = 0;

/////////////////////////////////////////////

unsigned int systime(){
	return tempo_sistema;
}

void controla_tempo(){
	if (current_task->preemptable == 1){
		temporizador--;
	}
	current_task->tempo_process++;

	tempo_sistema++;

	//se a task é preemptável...
	if ((temporizador == 0)&&(current_task->preemptable == 1))
		task_yield();
}

int inicia_timer_e_tratador(){
	//o action vai detectar interrupções do timer, sinal SIGALRM
	action.sa_handler = controla_tempo; //task que sera chamada a cada milisegundo;
	sigemptyset (&action.sa_mask);
	action.sa_flags = 0 ;
	if (sigaction (SIGALRM, &action, 0) < 0)
	{
		fprintf (stderr, "Erro no inicia_timer_e_tratador: sigaction retornou erro\n");
		return (-1) ;
	}

	//inicia os valores do timer
	timer.it_value.tv_usec = 1000;		//primeiro disparo, em micro-segundos
	timer.it_value.tv_sec  = 0;			//primeiro disparo, em segundos
	timer.it_interval.tv_usec = 1000;	//disparos seguintes, em micro-segundos
	timer.it_interval.tv_sec  = 0;		//disparos seguintes, em segundos	

	// arma o temporizador ITIMER_REAL (vide man setitimer)
	if (setitimer (ITIMER_REAL, &timer, 0) < 0)
	{
		fprintf (stderr, "Erro no incia_timer_e_tratador: setitimer retornou erro\n");
		return(-2);
	}

	temporizador = QUANTUM;

	#ifdef DEBUG
	printf ("inicia_timer_e_tratador executou com sucesso\n");
	#endif

	return 0;
}

void task_setprio (task_t *task, int prio){
	//testa se a prioridade é valida
	if ((prio < -20) || (prio > 20)){
		fprintf (stderr, "Erro no setprio: valor de prioridade inválido\n");
		return;
	}

	//caso seja nula, pegue a tarefa atual
	task_t* aux;
	if (task == NULL)
		aux = current_task;
	else
		aux = task;

	aux->prio_e = prio;
}

int task_getprio (task_t *task){
	//caso seja nula, pegue a tarefa atual
	task_t* aux;
	if (task == NULL)
		aux = current_task;
	else
		aux = task;

	return (aux->prio_e);
}

void print_elem_fila (void* elem){
	task_t* aux = (task_t *) elem;
	printf ("%d", aux->id);
}

void imprime_tarefas_dormindo(){
	queue_print ("Dormindo", fila_tasks_dormindo, print_elem_fila);
}

void acorda_tarefas(){
	task_t* aux = (task_t *) fila_tasks_dormindo;

	#ifdef DEBUGSLEEP2
	imprime_tarefas_dormindo();
	#endif

	//se houverem elementos na fila
	if (aux != NULL){
		//caso hajam mais que uma dormindo
		while(aux->next != (task_t *) fila_tasks_dormindo){
			//vai acordando as tarefas
			aux->tempo_dormir--;

			#ifdef DEBUGSLEEP
			printf ("acorda_tarefas: tarefa %d com tempo %d\n", aux->id, aux->tempo_dormir);
			#endif

			//acorda as tarefas
			if (aux->tempo_dormir == 0){
				#ifdef DEBUGSLEEP
				printf ("acorda_tarefas: acordou a tarefa %d pois esta com tempo %d\n", aux->id, aux->tempo_dormir);
				#endif

				task_t* task_acorda = aux;
				aux = aux->next;

				task_acorda->status	= PRONTA;
				queue_remove ((queue_t **) &fila_tasks_dormindo	, (queue_t *) task_acorda);
				queue_append ((queue_t **) &fila_tasks_prontas	, (queue_t *) task_acorda);
			}
			else{
				aux = aux->next;
			}
		}

		//acordando a última task
		aux->tempo_dormir--;

		#ifdef DEBUGSLEEP
		printf ("acorda_tarefas: tarefa %d com tempo %d\n", aux->id, aux->tempo_dormir);
		#endif

		//acordando a última task
		if (aux->tempo_dormir == 0){
			#ifdef DEBUGSLEEP
			printf ("acorda_tarefas: acordou a tarefa %d pois esta com tempo %d\n", aux->id, aux->tempo_dormir);
			#endif
			task_t* task_acorda = aux;
			task_acorda->status	= PRONTA;
			queue_remove ((queue_t **) &fila_tasks_dormindo	, (queue_t *) task_acorda);
			queue_append ((queue_t **) &fila_tasks_prontas	, (queue_t *) task_acorda);
		}
	}
}

task_t* scheduler(){
	//se a fila nao possuir elementos
	if (fila_tasks_prontas == NULL)
		return (NULL);

	task_t*	aux	= (task_t *) fila_tasks_prontas;
	int		maior_prio = aux->prio_e + aux->prio_d;
	task_t*	task_prio = aux;

	//envelhecimento
	if (aux->prio_d > -20)
		aux->prio_d--;

	//percorrendo toda a lista
	while (aux->next != (task_t *) fila_tasks_prontas){
		aux = aux->next;
		// usamos ">" pois a escala é negativa
		if (maior_prio > (aux->prio_e + aux->prio_d)){
			maior_prio	= aux->prio_e + aux->prio_d;
			task_prio 	= aux;
		}

		//envelhecimento
		if (aux->prio_d > -20)
			aux->prio_d--;
	}

	#ifdef DEBUGSCHEDULER
	printf ("scheduler: escolheu a tarefa %d da fila, ela possui prioridade %d\n", task_prio->id, (task_prio->prio_d + task_prio->prio_e)) ;
	#endif

	//reseta o envelhecimento da tarefa escolhida
	task_prio->prio_d = 0;
	return (task_prio);
}

void dispatcher(){
	//enquanto tivermos tarefas na fila de tarefas
	while (user_tasks > 0){
		acorda_tarefas();
		task_t* prox_task = scheduler();

		if (prox_task != NULL){

			//configura o quantum pra contar os quanta
			temporizador = QUANTUM;

			//aumenta as ativacoes
			task_switch (prox_task);

			switch (prox_task->status){
				case (PRONTA):
					break;

				case (TERMINADA):
					#ifdef DEBUG
					printf ("dispatcher: removeu a tarefa %d da fila\n", prox_task->id) ;
					#endif

					//retira a tarefa da fila de tarefas e dá free na estrutura
					queue_remove (&fila_tasks_prontas, (queue_t *) prox_task);

					free (prox_task->context.uc_stack.ss_sp);
					break;
			}

		}

	}

	task_exit(0);
}

void ppos_init (){
	setvbuf (stdout, 0, _IONBF, 0);

	current_task = &main_task;

	//coloca 0 na current_id novamente só pra garantir
	current_id = 0;
	
	user_tasks = 0;
	task_create(&main_task, NULL, NULL);
	task_create(&dispatcher_task, (void *) dispatcher, NULL);

	//inicia o timer e o tratador do timer em uma funcao separada
	if (inicia_timer_e_tratador() < 0){
		fprintf (stderr, "Erro no ppos_init: incia_timer_e_tratador retornou erro\n");
		exit(1);
	}

	#ifdef DEBUG
	printf ("ppos_init: iniciou o ppos com o id da main: %d\n", main_task.id) ;
	#endif
}

void task_sleep (int t){
	current_task->tempo_dormir	= t;
	current_task->status		= ADORMECIDA;

	queue_remove ((queue_t **) &fila_tasks_prontas	, (queue_t *) current_task);
	queue_append ((queue_t **) &fila_tasks_dormindo	, (queue_t *) current_task);
	task_yield();
}

int task_join (task_t *task){

	if (task == NULL)	
		return (-1);
	else if (task->status == TERMINADA)
		return (-1);

	#ifdef DEBUG
	printf ("task_join: a task %d quer esperar a task %d\n", current_task->id, task->id);
	#endif

	task_suspend ((task_t **) &task->fila_espera);

	//quando a tarefa "task" terminar, a tarefa atual retornará aqui

	return (task->codigo_saida);
}

void task_suspend (task_t **queue){
	queue_remove ((queue_t **) &fila_tasks_prontas, (queue_t *) current_task);

	#ifdef DEBUG
	printf ("task_suspend: a task %d foi removida da fila de prontas\n", current_task->id);
	#endif

	current_task->status	= SUSPENSA;

	queue_append ((queue_t **) queue, (queue_t *) current_task);

	#ifdef DEBUG
	printf ("task_suspend: a task %d foi colocada na fila desejada\n", current_task->id);
	#endif

	task_yield();
}

void task_resume (task_t * task, task_t **queue){
	if (queue == NULL)
		return;

	queue_remove ((queue_t **) queue, (queue_t *) task);

	task->status	= PRONTA;

	queue_append ((queue_t **) &fila_tasks_prontas, (queue_t *) task);
}

int task_create (task_t *task,
                 void (*start_func)(void *),
                 void *arg){
	
	//testa se a task existe
	if (task == NULL){
		fprintf (stderr, "Erro na criacao de task: task inexistente\n");
		return (-1);
	}

	//testa se a funcao existe e se nao é a main
	if ((start_func == NULL) && (current_id > 0)){
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

	// o makecontext só é chamado para funcoes que nao sao a main
	if (current_id > 0)
		makecontext(&new_context, (void *) start_func, 1, arg);


	//coloca as informacoes da task na estrutura
	task->id			= current_id;
	task->context		= new_context;
	task->status		= PRONTA;
	task->prio_e		= 0;
	task->prio_d		= 0;
	task->tempo_exec	= - systime();
	task->tempo_process	= 0;
	task->ativacoes		= 0;

	//caso seja o dispatcher, ela nao é preemptável
	if (task->id == 1)
		task->preemptable	= 0;
	else
		task->preemptable	= 1;

	current_id++;

	//se a tarefa nao for o dispatcher, adiciona na fila de tarefas
	if (task->id != 1){
		user_tasks++;
		queue_append ((queue_t **) &fila_tasks_prontas, (queue_t *) task);
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
	current_task->tempo_exec += systime();

	printf("Task %d exit: execution time %d ms, processor time %d, %d activations\n", 
	current_task->id, current_task->tempo_exec, current_task->tempo_process, current_task->ativacoes);

	#ifdef DEBUG
	printf ("task_exit: a task %d quer tirar elementos de sua fila de espera %d\n", 
			current_task->id, current_task->fila_espera);
	#endif

	current_task->codigo_saida = exit_code;

	//enquanto existirem elementos na fila
	while (current_task->fila_espera != NULL){
		task_t* tarefa_removida = current_task->fila_espera;

		#ifdef DEBUG
		printf ("task_exit: a task %d sera removida da fila de espera da task %d\n", 
				tarefa_removida->id, current_task->id);
		#endif

		task_resume(tarefa_removida, (task_t **) &current_task->fila_espera);
	}

	if (current_task->id != 1){
	//retorno para o dispatcher terminar a tarefa
		current_task->tempo_process += systime();
		user_tasks--;
		task_switch (&dispatcher_task);
	}
	//quando a funcao a dar exit é o próprio dispatcher
	else{
		task_switch (&main_task);
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

	task->ativacoes++;

	//transforma a task que será executada em task atual (current_task)
	task_t* aux_task = current_task;
	current_task = task;

	#ifdef DEBUGSWITCH
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
