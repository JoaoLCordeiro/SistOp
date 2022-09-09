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

#define 	READ	1
#define		WRITE	2

semaphore_t sem_disk;	//semáforo do disco

struct sigaction disk_action;	//ação do disco pra quando receber sinal

int sinal_disk;			//sinal do disco

task_t ddriver_task;	//task do driver

//estrutura de um pedido
typedef struct pedido_t{
	task_t *prev, *next;//usado na fila de pedidos
	task_t *tarefa;		//tarefa que fez o pedido
	int		tipo;		//se é escrita ou leitura
	int		block;		//bloco interessado
	void   *buffer;		//buffer do pedido
} pedido_t;

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