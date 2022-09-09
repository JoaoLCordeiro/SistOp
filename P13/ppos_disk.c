#include "ppos_disk.h"

//#define DEBUGDISK

void disk_handler(){
	#ifdef DEBUGDISK
	printf ("disk_handler: entrou na função\n");
	#endif

	sinal_disk = 1;
	
	if (ddriver_task.status == SUSPENSA){
		task_resume (&ddriver_task, (task_t **) &fila_tasks_prontas);
	}
}

void disk_driver(){
	#ifdef DEBUGDISK
	printf ("disk_driver: entrou na função\n");
	#endif

	int conta_lacos = 1;

	while (1){
		//#ifdef DEBUGDISK
		//printf ("disk_driver: laço numero %d\n", conta_lacos);
		//#endif

		conta_lacos++;

		//prende o semáforo
		sem_down(&sem_disk);

		//detectar sinal
		if (sinal_disk == 1){
			#ifdef DEBUGDISK
			printf ("disk_driver: (pedido_pronto)\n");
			#endif

			pedido_t *pedido_pronto = (pedido_t *) pedidos_prontos;

			if (queue_remove ((queue_t **) &(pedidos_prontos), (queue_t *) pedido_pronto) < 0){
				perror ("disk_driver: erro no queue_remove do pronto");
				return;
			}

			task_resume (pedido_pronto->tarefa, (task_t **) &pedido_pronto->tarefa);

			sinal_disk = 0;
		}

		if ((disk_cmd(DISK_CMD_STATUS, 0, NULL) == DISK_STATUS_IDLE) && (sem_disk.fila_tasks != NULL)){

			pedido_t *pedido_atual = (pedido_t *) sem_disk.fila_tasks;

			#ifdef DEBUGDISK
			printf ("disk_driver: (pedido_atual) do tipo %s\n", (pedido_atual->tipo == READ ? "READ" : "WRITE"));
			#endif

			if (queue_remove ((queue_t **) &(sem_disk.fila_tasks), (queue_t *) pedido_atual) < 0){
				perror ("disk_driver: erro no queue_remove do atual");
				return;
			}

			if (queue_append (&(pedidos_prontos), (queue_t *) pedido_atual) < 0){
				perror ("disk_driver: erro no queue_append do atual");
				return;
			}

			if (pedido_atual->tipo == READ){

				if (disk_cmd (DISK_CMD_READ, pedido_atual->block, pedido_atual->buffer)){
					perror ("disk_driver: Erro ao ler do arquivo\n");
					return;
				}

			}
			else if (pedido_atual->tipo == WRITE){

				if (disk_cmd (DISK_CMD_WRITE, pedido_atual->block, pedido_atual->buffer)){
					perror ("disk_driver: Erro ao escrever no arquivo\n");
					return;
				}

			}
		}

		//libera o semáforo
		sem_up(&sem_disk);

		//retorna pro dispatcher
		task_yield();
	}
} 

int disk_mgr_init (int *numBlocks, int *blockSize){
	#ifdef DEBUGDISK
	printf ("disk_mgr_init: entrou na função\n");
	#endif

	//cria um semáforo de 1 para o disco
	sem_create (&sem_disk, 1);

	disk_action.sa_handler = disk_handler;
  	sigemptyset (&disk_action.sa_mask);
  	disk_action.sa_flags = 0;

	if (sigaction (SIGUSR1, &disk_action, 0) < 0){
		perror ("disk_mgr_init: sigaction retornou erro\n");
		return -1;
	}

	if (disk_cmd (DISK_CMD_INIT, 0, NULL)){
		perror ("disk_mgr_init: Erro ao iniciar o disco (DISK_CMD_INIT)\n");
		return -1;
	}

	*numBlocks	=	disk_cmd (DISK_CMD_DISKSIZE, 0, NULL);
	if (*numBlocks == -1){
		perror ("disk_mgr_init: Erro ao iniciar o disco (DISK_CMD_DISKSIZE)\n");
		return -1;
	}

	*blockSize	=	disk_cmd (DISK_CMD_BLOCKSIZE, 0, NULL);
	if (*blockSize == -1){
		perror ("disk_mgr_init: Erro ao iniciar o disco (DISK_CMD_BLOCKSIZE)\n");
		return -1;
	}

	task_create (&ddriver_task, (void *) disk_driver, NULL);

	return 0;
}

pedido_t* cria_pedido (int tipo, int block, void *buffer){
	#ifdef DEBUGDISK
	printf ("cria_pedido: entrou na função\n");
	#endif

	pedido_t* pedido = malloc (sizeof(pedido_t));

	pedido->prev	= NULL;
	pedido->next	= NULL;
	pedido->tarefa	= current_task;
	pedido->block	= block;
	pedido->buffer	= buffer;
	pedido->tipo 	= tipo;

	return pedido;
}

int disk_block_read (int block, void *buffer){
	#ifdef DEBUGDISK
	printf ("disk_block_read: entrou na função\n");
	#endif

	sem_down(&sem_disk);

	//agenda um pedido
	pedido_t* pedido;
	pedido = cria_pedido (READ, block, buffer);

	if (pedido == NULL){
		perror ("disk_block_read: erro no cria_pedido");
		return -1;
	}

	if (queue_append ((queue_t **) &(sem_disk.fila_tasks), (queue_t *) pedido) < 0){
		perror ("disk_block_read: erro no queue_append");
		return -1;
	}

	if (ddriver_task.status == SUSPENSA){
		task_resume (&ddriver_task, (task_t **) &fila_tasks_prontas);
	}

	sem_up(&sem_disk);

	//task_yield();

	task_suspend (&current_task);

	return 0;
}

int disk_block_write (int block, void *buffer){
	#ifdef DEBUGDISK
	printf ("disk_block_write: entrou na função\n");
	#endif

	sem_down(&sem_disk);

	//agenda um pedido
	pedido_t* pedido;
	pedido = cria_pedido (WRITE,block, buffer);

	if (pedido == NULL){
		perror ("disk_block_write: erro no cria_pedido");
		return -1;
	}

	if (queue_append ((queue_t **) &(sem_disk.fila_tasks), (queue_t *) pedido) < 0){
		perror ("disk_block_write: erro no queue_append");
		return -1;
	}

	if (ddriver_task.status == SUSPENSA){
		task_resume (&ddriver_task, (task_t **) &fila_tasks_prontas);
	}

	sem_up(&sem_disk);

	//task_yield();

	task_suspend (&current_task);
	
	return 0;
}

