//Nome: João Lucas Cordeiro			GRR:20190427

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto

#define STACKSIZE 64*1024   //tamanho da pilha para cada thread

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t	*prev, *next ;	// ponteiros para usar em filas
  int			id ;			// identificador da tarefa
  ucontext_t	context ;		// contexto armazenado da tarefa
  short			status ;		// pronta, rodando, suspensa, ...
  short			preemptable ;	// pode ser preemptada?
  int			prio_e;         //prioridade estatica
  int			prio_d;         //prioridade dinamica
  int			tempo_exec;     //tempo de execucao
  int			tempo_process;  //tempo de processamento
  int			ativacoes;      //numero de ativacoes
  struct task_t	*fila_espera;	//fila de tarefas que esperam essa terminar
  int			codigo_saida;	//guarda o codigo de saida da tarefa que foi esperada
   // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif

