//Nome: João Lucas Cordeiro
//GRR: 20190427

#include <stdio.h>
#include "queue.h"

int queue_size (queue_t *queue){
	//caso nao exista fila
	if (queue == NULL)
		return (0);

	int cont = 1;
	queue_t *aux = queue;
	//passa pelos elementos da fila, contando-os
	while (aux->next != queue){
		aux = aux->next;
		cont++;
	}

	return (cont);
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ){
	printf ("%s: [", name);

	//passa pelos elementos, os imprimindo
	if (queue != NULL){
		queue_t *aux = queue;
		print_elem(aux);
		while (aux->next != queue){
			aux = aux->next;
			printf(" ");
			print_elem(aux);
		}
	}

	printf ("]\n");
}

int queue_append (queue_t **queue, queue_t *elem){
	//verifica se a fila existe
	if (queue == NULL){
		fprintf(stderr, "Erro: fila inexistente\n");
		return (-1);
	}

	//verifica se o elemento existe
	if (elem == NULL){
		fprintf(stderr, "Erro: elemento inexistente\n");
		return (-2);
	}

	//verifica se o elemento nao esta em outro lugar
	if ((elem->prev != NULL) || (elem->next != NULL)){
		fprintf(stderr, "Erro: tentativa de inserção de elemento que já possui fila\n");
		return (-3);
	}

	//caso: fila sem elementos
	if (*queue == NULL){
		(*queue)		= elem;
		(*queue)->prev	= (*queue);
		(*queue)->next	= (*queue);

		return (0);
	}

	//insere o elemento no final da fila
	elem->prev 			= (*queue)->prev;
	(*queue)->prev		= elem;
	elem->prev->next	= elem;
	elem->next			= (*queue);

	return (0);
}

int queue_remove (queue_t **queue, queue_t *elem){
	//verifica se a fila existe
	if (queue == NULL){
		fprintf(stderr, "Erro: fila inexistente\n");
		return (-1);
	}

	//verifica se o elemento existe
	if (elem == NULL){
		fprintf(stderr, "Erro: elemento inexistente\n");
		return (-2);
	}

	//verifica se a fila é vazia
	if (((*queue)->prev == NULL) || ((*queue)->next == NULL)){
		fprintf(stderr, "Erro: tentativa de remoção em fila vazia\n");
		return (-3);
	}

	//primeiro elemento eh o requerido
	if (elem == (*queue)){
		//fila única
		if (elem->next == elem){
			(*queue) = NULL;
			//agora, a fila está vazia
		}
		//fila maior
		else{
			(*queue) = elem->next;

			elem->next->prev = elem->prev;
			elem->prev->next = elem->next;
		}
		elem->next = NULL;
		elem->prev = NULL;
		return (0);
	}
	//caso nao seja o primeiro elemento
	else{
		queue_t *aux = (*queue);
		while (aux->next != (*queue)){
			aux = aux->next;
			if (aux == elem){
				//remove o elemento e retorna
				elem->next->prev = elem->prev;
				elem->prev->next = elem->next;
				elem->next = NULL;
				elem->prev = NULL;
				return (0);
			}
		}

		fprintf(stderr, "Erro: tentativa de remoção de elemento nao encontrado\n");
		return (-4);
		//nao achou o elemento
	}
}