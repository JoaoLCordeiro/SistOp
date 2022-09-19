#include "ppos_disk.h"

void trataSigUsr (){
	#ifdef DEBUGPPOSDISK
	printf ("trataSigUsr: entrou na função\n");
	#endif

	sinal_disco = 1;

	queue_append ((queue_t **) &fila_tasks_prontas, (queue_t *) &task_driver);
	task_yield();
}

void diskDriverBody (){
	#ifdef DEBUGPPOSDISK
	printf ("diskDriverBody: entrou na função\n");
	#endif

	while (1) 
	{
		#ifdef DEBUGPPOSDISK
		printf ("diskDriverBody: começou o laço\n");
		#endif

		// obtém o semáforo de acesso ao disco
		sem_down(&sem_disk);	

		// se foi acordado devido a um sinal do disco
		if (sinal_disco == 1)
		{
			#ifdef DEBUGPPOSDISK
			printf ("diskDriverBody: entrou no sinal_disco == 1\n");
			#endif

			// acorda a tarefa cujo pedido foi atendido
			pedido_t* pedido = pedidos_queue;
			queue_remove ((queue_t **) &pedidos_queue, (queue_t *) pedido);
			queue_append ((queue_t **) &fila_tasks_prontas, (queue_t *) pedido->task);

			sinal_disco = 0;
		}

		// se o disco estiver livre e houver pedidos de E/S na fila
		if ((disk_cmd (DISK_CMD_STATUS, 0, NULL) == DISK_STATUS_IDLE) && (pedidos_queue != NULL))
		{
			#ifdef DEBUGPPOSDISK
			printf ("diskDriverBody: entrou no segundo if\n");
			#endif

			// escolhe na fila o pedido a ser atendido, usando FCFS
			// solicita ao disco a operação de E/S, usando disk_cmd()
			int		block	= pedidos_queue->block;
			void*	buffer	= pedidos_queue->buffer;
			int		tipo	= pedidos_queue->tipo;

			if (tipo == READ){
				#ifdef DEBUGPPOSDISK
				printf ("diskDriverBody: entrou no tipo == READ\n");
				#endif

				if (disk_cmd (DISK_CMD_READ, block, buffer) == -1){
					perror ("diskDriverBody: Erro no disk_cmd com read\n");
					exit(1);
				}
			}
			else if (tipo == WRITE){
				#ifdef DEBUGPPOSDISK
				printf ("diskDriverBody: entrou no tipo == WRITE\n");
				#endif

				if (disk_cmd (DISK_CMD_WRITE, block, buffer) == -1){
					perror ("diskDriverBody: Erro no disk_cmd com write\n");
					exit(1);
				}
			}
			else{
				#ifdef DEBUGPPOSDISK
				printf ("diskDriverBody: tipo inesperado\n");
				#endif
			}
		}	
		// libera o semáforo de acesso ao disco
		sem_up(&sem_disk);
		// suspende a tarefa corrente (retorna ao dispatcher)
		#ifdef DEBUGPPOSDISK
		printf ("diskDriverBody: deu yield\n");
		#endif
		queue_remove ((queue_t **) &fila_tasks_prontas, (queue_t *) &task_driver);
		task_yield();
	}
}

int disk_mgr_init (int *numBlocks, int *blockSize){
	#ifdef DEBUGPPOSDISK
	printf ("disk_mgr_init: entrou na função\n");
	#endif

	sinal_disco = 0;

	//cria o semáforo do disco
	sem_create (&sem_disk, 1);
	//pedidos_queue = malloc (sizeof(pedido_t));
	
	//task que sera chamada a cada sinal
	action_disk.sa_handler = trataSigUsr; 
	sigemptyset (&action_disk.sa_mask);
	action_disk.sa_flags = 0 ;

	if (sigaction (SIGUSR1, &action_disk, 0) < 0)
	{
		fprintf (stderr, "Erro no disk_mgr_init: sigaction retornou erro\n");
		return (-1) ;
	}

	//inicia o disco
	if (disk_cmd (DISK_CMD_INIT, 0, NULL) == -1){
		perror("disk_mgr_init: Erro no disk_init\n");
		return -1;
	}

	//guarda o número de blocos do disco em numBlocks
	*numBlocks = disk_cmd (DISK_CMD_DISKSIZE, 0, NULL);
	if ((*numBlocks) == -1){
		perror("disk_mgr_init: Erro no disk_size\n");
		return -1;
	}

	//guarda o tamanho dos blocos do disco em blockSize
	*blockSize = disk_cmd (DISK_CMD_BLOCKSIZE, 0, NULL);
	if ((*blockSize) == -1){
		perror("disk_mgr_init: Erro no block_size\n");
		return -1;
	}
	
	task_create(&task_driver, diskDriverBody, NULL);
	queue_remove ((queue_t **) &fila_tasks_prontas, (queue_t *) &task_driver);

	return 0;
}

int disk_block_read  (int block, void* buffer){
	#ifdef DEBUGPPOSDISK
	printf ("disk_block_read: entrou na função\n");
	#endif

	sem_down(&sem_disk);

	pedido_t *pedido = malloc (sizeof(pedido_t));

	pedido->block	= block;
	pedido->buffer	= buffer; 
	pedido->tipo	= READ;
	pedido->task	= current_task;

	queue_remove ((queue_t **) &fila_tasks_prontas, (queue_t *) current_task);
	queue_append ((queue_t **) &pedidos_queue, (queue_t *) pedido);
	queue_append ((queue_t **) &fila_tasks_prontas, (queue_t *) &task_driver);

	#ifdef DEBUGPPOSDISK
	printf ("disk_block_read: antes do yield\n");
	#endif

	sem_up(&sem_disk);

	task_yield();

	#ifdef DEBUGPPOSDISK
	printf ("disk_block_read: depois do yield\n");
	#endif

	free (pedido);

	return 0;
}

int disk_block_write (int block, void* buffer){
	#ifdef DEBUGPPOSDISK
	printf ("disk_block_write: entrou na função\n");
	#endif

	sem_down(&sem_disk);

	pedido_t *pedido = malloc (sizeof(pedido_t));

	pedido->block	= block;
	pedido->buffer	= buffer;
	pedido->tipo	= WRITE;
	pedido->task	= current_task;

	queue_remove ((queue_t **) &fila_tasks_prontas, (queue_t *) current_task);
	queue_append ((queue_t **) &pedidos_queue, (queue_t *) pedido);
	queue_append ((queue_t **) &fila_tasks_prontas, (queue_t *) &task_driver);

	#ifdef DEBUGPPOSDISK
	printf ("disk_block_write: antes do yield\n");
	#endif

	sem_up(&sem_disk);

	task_yield();

	#ifdef DEBUGPPOSDISK
	printf ("disk_block_write: depois do yield\n");
	#endif

	free (pedido);

	return 0;
}