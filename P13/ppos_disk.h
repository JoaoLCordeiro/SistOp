// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__

#include "ppos_core.h"

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

// estrutura que representa um disco no sistema operacional
//typedef struct
//{
//  // completar com os campos necessarios
//} disk_t ;

//#define DEBUGPPOSDISK

#define 	READ	1
#define		WRITE	2

typedef struct pedido_t{
	struct pedido_t	*prev, *next;
	int		block;
	void*	buffer;
	int		tipo;
	task_t*	task;
} pedido_t;

semaphore_t sem_disk;

pedido_t* pedidos_queue;

task_t task_driver;

int sinal_disco;

struct sigaction action_disk;

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer) ;

#endif