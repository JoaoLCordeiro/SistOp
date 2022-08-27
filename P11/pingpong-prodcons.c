#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"
#include "ppos_core.h"

#include <stdlib.h>

#define NPRODUTOS 40

semaphore_t s_vaga;
semaphore_t s_buffer;
semaphore_t s_item;

void produtor (){
	for (int i = 1 ; i <= NPRODUTOS ; i++){
		task_sleep (1000);

		int item = rand() % 100;

		sem_down (s_vaga);

		sem_down (s_buffer);

		//implementar

		sem_up (s_buffer);
		sem_up (s_item);
	}
}

int main(){
	sem_create(&s_vaga	, 1);
	sem_create(&s_buffer, 5);
	sem_create(&s_item	, 1);

	return 0;
}