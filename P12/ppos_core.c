//Nome: João Lucas Cordeiro			GRR:20190427

#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"
#include "ppos_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

//#define DEBUG
//#define DEBUGSWITCH
//#define DEBUGSCHEDULER
//#define DEBUGSLEEP
//#define DEBUGSEMAPHORE
//#define DEBUGMSGQUEUE

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

/*-----------FILA-DE-MENSAGENS-------------*/

int mqueue_create (mqueue_t *queue, int max_msgs, int msg_size){
	#ifdef DEBUGMSGQUEUE
	printf ("mqueue_create: entrou na função\n");
	#endif

	if (queue == NULL){
		perror ("mqueue_create:	ponteiro nulo para mqueue");
		return -1;
	}

	queue->buffer = malloc (max_msgs*msg_size);

	queue->vagas_max	= max_msgs;
	queue->tam_mensagem	= msg_size;
	//a cabeca sempre comeca na primeira posicao do buffer
	queue->cabeca		= queue->buffer;
	queue->cauda		= queue->buffer;

	//comeca com todas as vagas livres
	if (sem_create(&(queue->vagas), max_msgs)){
		perror ("mqueue_create:	nao conseguiu criar o semáforo das vagas");
		return -1;
	}

	//comeca 0 itens
	if (sem_create(&(queue->itens), 0)){
		perror ("mqueue_create:	nao conseguiu criar o semáforo dos itens");
		return -1;
	}

	if (sem_create(&(queue->semaforo), 1)){
		perror ("mqueue_create:	nao conseguiu criar o semáforo");
		return -1;
	}

	return 0;
}

int mqueue_send (mqueue_t *queue, void *msg){
	#ifdef DEBUGMSGQUEUE
	printf ("mqueue_send: entrou na função\n");
	#endif

	if (queue == NULL){
		perror ("mqueue_send:	ponteiro nulo para a queue\n");
		return -1;
	}

	if (msg == NULL){
		perror ("mqueue_send:	ponteiro nulo para a msg\n");
		return -1;
	}

	if (sem_down(&(queue->vagas)) == -1)
		return -1;

	//verifica se o buffer está em uso e bloqueia se estiver
	if (sem_down(&(queue->semaforo)) == -1)
		return -1;

	memcpy(queue->cabeca, msg, queue->tam_mensagem);

	queue->cabeca += queue->tam_mensagem;
	//faz o buffer circular
	if (queue->buffer + queue->tam_mensagem * queue->vagas_max == queue->cabeca){
		queue->cabeca = queue->buffer;
	}

	if (sem_up(&(queue->semaforo)) == -1)
		return -1;

	if (sem_up(&(queue->itens)) == -1)
		return -1;

	return 0;
}

int mqueue_recv (mqueue_t *queue, void *msg){
	#ifdef DEBUGMSGQUEUE
	printf ("mqueue_recv: entrou na função\n");
	#endif


	if (queue == NULL){
		perror ("mqueue_recv:	ponteiro nulo para a queue\n");
		return -1;
	}

	if (msg == NULL){
		perror ("mqueue_recv:	ponteiro nulo para a msg\n");
		return -1;
	}

	if (sem_down (&(queue->itens)) == -1)
		return -1;

	//verifica se o buffer está em uso e bloqueia se estiver
	if (sem_down(&(queue->semaforo)) == -1)
		return -1;

	memcpy(msg, queue->cauda, queue->tam_mensagem);

	queue->cauda += queue->tam_mensagem;
	//faz o buffer circular
	if (queue->buffer + queue->tam_mensagem * queue->vagas_max == queue->cauda){
		queue->cauda = queue->buffer;
	}

	if (sem_up(&(queue->semaforo)) == -1)
		return -1;

	if (sem_up(&(queue->vagas)) == -1)
		return -1;

	return 0;
}

int mqueue_destroy (mqueue_t *queue){
	#ifdef DEBUGMSGQUEUE
	printf ("mqueue_destroy: entrou na função\n");
	#endif

	if (queue == NULL){
		perror ("mqueue_destroy:	ponteiro nulo para a queue\n");
		return -1;
	}

	free(queue->buffer);
	sem_destroy(&(queue->itens));
	sem_destroy(&(queue->vagas));
	sem_destroy(&(queue->semaforo));

	return 0;
}

int mqueue_msgs (mqueue_t *queue){
	#ifdef DEBUGMSGQUEUE
	printf ("mqueue_msgs: entrou na função\n");
	#endif

	if (queue == NULL){
		perror ("mqueue_msgs:	ponteiro nulo para a queue");
		return -1;
	}

	int msgs = sem_count (&(queue->itens));
	if (msgs < 0)
		msgs = 0;

	return msgs;
}

/*-----------------------------------------*/

/*---------------SEMÁFOROS-----------------*/

int sem_count (semaphore_t *s){
	if (s == NULL){
		perror ("sem_count:	semáforo nulo");
		return -1;
	}

	return s->contador;
}

int sem_create (semaphore_t *s, int value){
	#ifdef DEBUGSEMAPHORE
	printf ("sem_create: entrou na função\n");
	#endif

	//erro: semáforo nulo
	if (s == NULL){
		perror ("sem_create: semáforo nulo");
		return -1;
	}

	//deu tud certo :)
	s->contador = value;
	s->destruido = 0;
	return 0;
}

int sem_down (semaphore_t *s){
	#ifdef DEBUGSEMAPHORE
	printf ("sem_down: entrou na função\n");
	#endif

	//semáforo nulo
	if (s == NULL)
		return -1;

	//semáforo destruído
	if (s->destruido == 1)
		return -1;

	//desliga o  preemptamento da tarefa atual, fazendo a função ser "atômica"
	current_task->preemptable = 0;

	s->contador--;

	if (s->contador < 0){
		//tira a tarefa da fila de prontas e a coloca na fila do semáforo
		current_task->status = SUSPENSA;
		queue_remove ((queue_t **) &fila_tasks_prontas	, (queue_t *) current_task);
		queue_append ((queue_t **) &(s->fila_tasks)		, (queue_t *) current_task);

		#ifdef DEBUGSEMAPHORE
		printf ("sem_down: colocou a tarefa %d na fila e vai dar yield\n", current_task->id);
		#endif

		current_task->preemptable = 1;

		task_yield();
	}
	else
		current_task->preemptable = 1;

	#ifdef DEBUGSEMAPHORE
	printf ("sem_down: a tarefa %d passou pelo if\n", current_task->id);
	#endif

	//se a tarefa voltu a ser executada por que o semáforo foi destruído, retornamos -1
	if (s->destruido == 1)	
		return -1;

	return 0;
}

int sem_up (semaphore_t *s){
	#ifdef DEBUGSEMAPHORE
	printf ("sem_up: entrou na função\n");
	#endif
	
	//semáforo nulo
	if (s == NULL)
		return -1;

	//semáforo destruído
	if (s->destruido == 1)
		return -1;

	//desliga o  preemptamento da tarefa atual, fazendo a função ser "atômica"
	current_task->preemptable = 0;

	s->contador++;

	if (s->contador <= 0){
		//tira a tarefa da fila do semáforo e coloca na fila de prontas
		task_t* task_aux = s->fila_tasks;
		task_aux->status = PRONTA;
		queue_remove ((queue_t **) &(s->fila_tasks)		, (queue_t *) s->fila_tasks);
		queue_append ((queue_t **) &fila_tasks_prontas	, (queue_t *) task_aux);

		#ifdef DEBUGSEMAPHORE
		printf ("sem_up: tirou a tarefa %d da fila do semáforo\n", task_aux->id);
		#endif
	}

	current_task->preemptable = 1;
	return 0;
}

int sem_destroy (semaphore_t *s){
	#ifdef DEBUGSEMAPHORE
	printf ("sem_destroy: vai destruit um semáforo\n");
	#endif

	//semáforo nulo
	if (s == NULL)
		return -1;

	//desliga o  preemptamento da tarefa atual, fazendo a função ser "atômica"
	current_task->preemptable = 0;

	//passa por todas as tarefas na fila do semáforo
	while (s->fila_tasks != NULL){
		//tira a tarefa da fila do semáforo e coloca ela na fila de prontas
		task_t* task_aux = s->fila_tasks;
		task_aux->status = PRONTA;
		queue_remove ((queue_t **) &(s->fila_tasks)		, (queue_t *) s->fila_tasks);
		queue_append ((queue_t **) &fila_tasks_prontas	, (queue_t *) task_aux);

		#ifdef DEBUGSEMAPHORE
		printf ("sem_up: tirou a tarefa %d da fila do semáforo\n", task_aux->id);
		#endif
	}

	s->destruido = 1;

	return 0;
}

/*-----------------------------------------*/

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

void acorda_tarefas(){
	task_t* aux = (task_t *) fila_tasks_dormindo;

	//se houverem elementos na fila
	if (aux != NULL){
		//caso hajam mais que uma dormindo
		while((aux->next != (task_t *) fila_tasks_dormindo) && (fila_tasks_dormindo != NULL)){
			if (aux->ini_sono + aux->tempo_dormir == systime()){
				task_t* tarefa_remov = aux;
				aux = aux->next;

				task_resume(tarefa_remov, (task_t **) &fila_tasks_dormindo);
			}
			else
				aux = aux->next;
		}

		//caso fique só uma
		if (aux->ini_sono + aux->tempo_dormir == systime()){
			task_t* tarefa_remov = aux;
			aux = aux->next;

			task_resume(tarefa_remov, (task_t **) &fila_tasks_dormindo);
		}
		else
			aux = aux->next;
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
	#ifdef DEBUG
	printf ("task_sleep: a task %d quer mimir %d milisegundos\n", current_task->id, current_task->tempo_dormir);
	#endif

	if (t <= 0){
		task_yield();
		return;
	}

	//coloca os valores na tarefa atual
	current_task->tempo_dormir	= t;
	current_task->ini_sono		= systime();
	current_task->status		= ADORMECIDA;

	//remove da fila de prontas e coloca na fila de tarefas dormindo
	queue_remove ((queue_t **) &fila_tasks_prontas	, (queue_t *) current_task);
	queue_append ((queue_t **) &fila_tasks_dormindo	, (queue_t *) current_task);

	task_yield();
}

int task_join (task_t *task){

	//testa erros
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
	//remove das prontas
	queue_remove ((queue_t **) &fila_tasks_prontas, (queue_t *) current_task);

	current_task->status	= SUSPENSA;

	//coloca na fila desejada
	queue_append ((queue_t **) queue, (queue_t *) current_task);

	#ifdef DEBUG
	printf ("task_suspend: a task %d foi removida da fila de prontas e foi colocada na fila desejada\n", current_task->id);
	#endif

	task_yield();
}

void task_resume (task_t * task, task_t **queue){
	if (queue == NULL)
		return;

	//remove da fila requerida
	queue_remove ((queue_t **) queue, (queue_t *) task);

	task->status	= PRONTA;

	//coloca na fila de prontas
	queue_append ((queue_t **) &fila_tasks_prontas, (queue_t *) task);

	#ifdef DEBUG
	printf ("task_resume: a task %d foi removida da fila desejada e foi colocada na fila de prontas\n", task->id);
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

	//exit_code para o task_join das tarefas retornarem
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
