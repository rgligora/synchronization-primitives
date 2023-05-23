#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>

#define N 5
#define M 14
#define X 30


typedef struct{
	sem_t customers;
	sem_t barberChair;
	sem_t barberReady;
	sem_t exclude;
	int customersWaiting;
	int customersIDs[N];
	bool open;
	int toBeWaited;
} shared_mem;


shared_mem *shop; 
int id_shared_mem;
int counter = 1;


void handleAlarm(int signum) {
	printf("Frizerka: Postavljam znak ZATVORENO\n");
    shop->open = false;
}

int getRandomNumberInRange(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}



int main(){
	id_shared_mem = shmget(IPC_PRIVATE, sizeof(shared_mem), 0600);

    if(id_shared_mem == -1){
        exit(1);
    }

    shop = (shared_mem *)shmat(id_shared_mem, NULL, 0);
    shmctl (id_shared_mem, IPC_RMID, NULL); 
    shop->customersWaiting = 0;
    shop->toBeWaited = 0;
    shop->open = true;
    for(int i = 0; i < N; i++){
    	shop->customersIDs[i] = 0;
    }
    sem_init(&(shop->customers),1,0);
    sem_init(&(shop->barberChair),1,0);
    sem_init(&(shop->barberReady),1,0);
    sem_init(&(shop->exclude),1,1);

    pid_t pid = fork();
    if (pid < 0) {
        return 1;
    } else if (pid == 0) {
    	printf("Frizerka: Otvaram salon\n");
    	printf("Frizerka: Postavljam znak OTVORENO\n");
    	struct sigaction sa;

	    sa.sa_handler = handleAlarm;
	    sa.sa_flags = 0;
	    sigemptyset(&sa.sa_mask);

	    sigaction(SIGALRM, &sa, NULL);
    	int seconds = M;
    	alarm(seconds); 
    	
    	
    	while(1){
    		if(!shop->open){
    			while(shop->customersWaiting > 0){
    				int temp = shop->customersIDs[shop->customersWaiting-1];
	    			printf("Frizerka: Idem raditi na klijentu %d\n", temp);
			    	sem_wait(&(shop->exclude));
			    	

			    	sem_post(&(shop->barberChair));
			    	shop->customersWaiting--;
			    	sem_post(&(shop->exclude));
			    	sleep(2);

		            sem_post(&(shop->barberReady)); 
		            printf("Frizerka: Klijent %d gotov\n",temp);
    			}
    			printf("Frizerka: Zatvaram salon\n");
    			exit(0);
    		}
    		if(shop->customersWaiting != 0){
    			
    			int temp = shop->customersIDs[shop->customersWaiting-1];
    			printf("Frizerka: Idem raditi na klijentu %d\n", temp);
		    	sem_wait(&(shop->exclude));
		    	

		    	sem_post(&(shop->barberChair));
		    	shop->customersWaiting--;
		    	sem_post(&(shop->exclude));
		    	sleep(2);

	            sem_post(&(shop->barberReady)); 
	            printf("Frizerka: Klijent %d gotov\n",temp);
    		} else if(shop->open){
    			printf("Frizerka: Spavam dok klijenti ne dođu\n");
    			sem_wait(&(shop->customers));
    		}
    		
    	}
    	
    } else {
    	sleep(1);
    	for(int k = 0; k < X; k++){
    		if(shop->open){
    			
    		pid = fork();
    		if (pid < 0) {
    		    return 1;
            } else if (pid == 0) {
            	shop->toBeWaited++;
	    		printf("\tKlijent(%d): Želim na frizuru\n", counter);
	    		if(shop->open){
	    			sem_wait(&(shop->exclude));
	    			if(shop->customersWaiting < N){
	    				shop->customersIDs[shop->customersWaiting] = counter;
	    				shop->customersWaiting++;
	    				printf("\tKlijent(%d): Ulazim u čekaonicu (%d)\n", shop->customersIDs[shop->customersWaiting-1], shop->customersWaiting);
		    			sem_post(&(shop->exclude));
		    			sem_post(&(shop->customers));
		    			
		    			
		    			sem_wait(&(shop->barberChair));
		    			printf("\tKlijent(%d): frizerka mi radi frizuru\n",shop->customersIDs[shop->customersWaiting]);
		    			sem_wait(&(shop->barberReady));
	    			}else{
	    				printf("\tKlijent(%d): Nema mjesta u čekaoni, vratit ću se sutra\n",counter);
	    				sem_post(&(shop->exclude));
	    			}
	    		}
	    		exit(0);
            }
            counter++;
            sleep(getRandomNumberInRange(0.5,2));
            //1.8,3
    		}
    	}
    }


    for(int j = 0; j < shop->toBeWaited; j++){
    	wait(NULL);
    }


    sem_destroy(&(shop->customers));
    sem_destroy(&(shop->barberChair));
    sem_destroy(&(shop->barberReady));
    sem_destroy(&(shop->exclude));
	shmdt(shop);
	
	return 0;

}