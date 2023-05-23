#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_TRAVELERS 7
#define LIJEVO 0
#define DESNO 1
#define MAX_THREADS 100

pthread_mutex_t mutex;
pthread_cond_t condBoat, condTraveler, condReady;
bool obala = DESNO;
pthread_t theCreator;
pthread_t theBoat;
int threadId = 123;
pthread_t misionari_threads[MAX_THREADS];
int ids_misionari[MAX_THREADS];
pthread_t kanibali_threads[MAX_THREADS];
int ids_kanibali[MAX_THREADS];

int K = 1;
int M = 1;
int boat_travellers = 0;
int misionari_broj = 0;
int kanibali_broj = 0;

#define MAX_SIZE 100

typedef struct Node {
  char * data;
  struct Node * next;
}
Node;

typedef struct {
  Node * head;
  Node * tail;
}
LinkedList;

// Function to initialize the linked list
void initializeLinkedList(LinkedList * list) {
  list -> head = NULL;
  list -> tail = NULL;
}

// Function to check if the linked list is empty
bool isLinkedListEmpty(const LinkedList * list) {
  return list -> head == NULL;
}

// Function to create a new node
Node * createNode(const char * data) {
  Node * newNode = (Node * ) malloc(sizeof(Node));
  if (newNode == NULL) {
    printf("Memory allocation failed.\n");
    exit(EXIT_FAILURE);
  }

  newNode -> data = strdup(data);
  newNode -> next = NULL;
  return newNode;
}

// Function to insert a node at the end of the linked list
void insertNode(LinkedList * list,
  const char * data) {
  Node * newNode = createNode(data);

  if (isLinkedListEmpty(list)) {
    list -> head = newNode;
    list -> tail = newNode;
  } else {
    list -> tail -> next = newNode;
    list -> tail = newNode;
  }
}

void removeNode(LinkedList * list,
  const char * data) {
  if (isLinkedListEmpty(list)) {
    printf("List is empty. Cannot remove node.\n");
    return;
  }

  Node * current = list -> head;
  Node * previous = NULL;

  while (current != NULL) {
    if (strcmp(current -> data, data) == 0) {
      if (previous == NULL) {
        // Node to be removed is the head
        list -> head = current -> next;
        if (current == list -> tail) {
          // Node to be removed is also the tail
          list -> tail = NULL;
        }
      } else {
        previous -> next = current -> next;
        if (current == list -> tail) {
          // Node to be removed is the tail
          list -> tail = previous;
        }
      }

      free(current -> data);
      free(current);
      return;
    }

    previous = current;
    current = current -> next;
  }

}

void displayLinkedList(const LinkedList * list) {
  if (isLinkedListEmpty(list)) {
    printf("");
    return;
  }

  Node * current = list -> head;
  while (current != NULL) {
    printf("%s ", current -> data);
    current = current -> next;
  }
  printf("");
}

void freeLinkedList(LinkedList * list) {
  Node * current = list -> head;
  while (current != NULL) {
    Node * next = current -> next;
    free(current -> data);
    free(current);
    current = next;
  }
  list -> head = NULL;
  list -> tail = NULL;
  initializeLinkedList(list);
}

LinkedList LO;
LinkedList DO;
LinkedList C;

int getRandomNumberInRange(int min, int max) {
  return (rand() % (max - min + 1)) + min;
}

void * kanibali(void * arg) {
  int side;
  pthread_mutex_lock( & mutex);
  int temp = rand() % 2;
  if (temp > 0) {
    side = 1;
    insertNode( & DO, arg);
    printf("%s: dosao na desnu obalu\n", arg);
  } else {
    side = 0;
    insertNode( & LO, arg);
    printf("%s: dosao na lijevu obalu\n", arg);
  }

  if (obala == 0) {
    printf("C[L]={");
    displayLinkedList( & C);
    printf("} LO={");
    displayLinkedList( & LO);
    printf("} DO={");
    displayLinkedList( & DO);
    printf("}\n");
  } else {
    printf("C[D]={");
    displayLinkedList( & C);
    printf("} LO={");
    displayLinkedList( & LO);
    printf("} DO={");
    displayLinkedList( & DO);
    printf("}\n");
  }
  printf("\n");

  while (!((obala && side == 1) || (!obala && side == 0)) || (misionari_broj == kanibali_broj) && (misionari_broj != 0)) {
    pthread_cond_wait( & condReady, & mutex);
  }

  printf("%s: ušao u čamac\n", arg);
  insertNode( & C, arg);
  if (obala == 0) {
    removeNode( & LO, arg);
    printf("C[L]={");
    displayLinkedList( & C);
    printf("} LO={");
    displayLinkedList( & LO);
    printf("} DO={");
    displayLinkedList( & DO);
    printf("}\n");

  } else {
    removeNode( & DO, arg);
    printf("C[D]={");
    displayLinkedList( & C);
    printf("} LO={");
    displayLinkedList( & LO);
    printf("} DO={");
    displayLinkedList( & DO);
    printf("}\n");

  }
  kanibali_broj++;

  boat_travellers++;

  printf("\n");
  if (boat_travellers >= 3) {
    pthread_cond_signal( & condBoat);
  }
  pthread_mutex_unlock( & mutex);
  return 0;
}

void * misionari(void * arg) {
  int side;
  pthread_mutex_lock( & mutex);
  int temp = rand() % 2;
  if (temp > 0) {
    side = 1;
    insertNode( & DO, arg);
    printf("%s: dosao na desnu obalu\n", arg);
  } else {
    side = 0;
    insertNode( & LO, arg);
    printf("%s: dosao na lijevu obalu\n", arg);
  }

  if (obala == 0) {
    printf("C[L]={");
    displayLinkedList( & C);
    printf("} LO={");
    displayLinkedList( & LO);
    printf("} DO={");
    displayLinkedList( & DO);
    printf("}\n");
  } else {
    printf("C[D]={");
    displayLinkedList( & C);
    printf("} LO={");
    displayLinkedList( & LO);
    printf("} DO={");
    displayLinkedList( & DO);
    printf("}\n");
  }
  printf("\n");

  while (!((obala && side == 1) || (!obala && side == 0)) || (misionari_broj + 1 < kanibali_broj) || boat_travellers >= MAX_TRAVELERS) {
    pthread_cond_wait( & condReady, & mutex);
  }

  printf("%s: ušao u čamac\n", arg);
  insertNode( & C, arg);
  if (obala == 0) {
    removeNode( & LO, arg);
    printf("C[L]={");
    displayLinkedList( & C);
    printf("} LO={");
    displayLinkedList( & LO);
    printf("} DO={");
    displayLinkedList( & DO);
    printf("}\n");

  } else {
    removeNode( & DO, arg);
    printf("C[D]={");
    displayLinkedList( & C);
    printf("} LO={");
    displayLinkedList( & LO);
    printf("} DO={");
    displayLinkedList( & DO);
    printf("}\n");

  }
  misionari_broj++;

  boat_travellers++;

  printf("\n");
  if (boat_travellers >= 3) {
    pthread_cond_signal( & condBoat);
  }
  pthread_mutex_unlock( & mutex);
  return 0;

}

void * boat(void * arg) {
  // kad promjeni stranu mora signalizirati cond_promjenio_stranu

  if (obala) {
    printf("C: prazan na desnoj obali\n");
  } else {
    printf("C: prazan na lijevoj obali\n");
  }
  if (obala == 0) {
    printf("C[L]={");
    displayLinkedList( & C);
    printf("} LO={");
    displayLinkedList( & LO);
    printf("} DO={");
    displayLinkedList( & DO);
    printf("}\n");
  } else {
    printf("C[D]={");
    displayLinkedList( & C);
    printf("} LO={");
    displayLinkedList( & LO);
    printf("} DO={");
    displayLinkedList( & DO);
    printf("}\n");
  }
  printf("\n");
  while (1) {
    pthread_mutex_lock( & mutex);

    while (boat_travellers < 3)
      pthread_cond_wait( & condBoat, & mutex);

    printf("C: tri putnika ukrcana, polazim za jednu sekundu\n");
    if (obala == 0) {
      printf("C[L]={");
      displayLinkedList( & C);
      printf("} LO={");
      displayLinkedList( & LO);
      printf("} DO={");
      displayLinkedList( & DO);
      printf("}\n");
    } else {
      printf("C[D]={");
      displayLinkedList( & C);
      printf("} LO={");
      displayLinkedList( & LO);
      printf("} DO={");
      displayLinkedList( & DO);
      printf("}\n");
    }
    printf("\n");
    pthread_mutex_unlock( & mutex);
    sleep(1);

    // Isprintaj koga vozi s koje strane na koju
    pthread_mutex_lock( & mutex);
    if (obala == 1) {
      printf("C: prevozim s desne na lijevu obalu:");
    } else {
      printf("C: prevozim s lijeve na desnu obalu:");
    }

    displayLinkedList( & C);
    printf("\n\n");

    pthread_mutex_unlock( & mutex);
    sleep(2);
    pthread_mutex_lock( & mutex);

    // isprintaj koga si preveo s koje na koju stranu
    // Isprintaj koga vozi s koje strane na koju
    if (obala) {
      printf("C: preveo s desne na lijevu obalu:");
    } else {
      printf("C: preveo s lijeve na desnu obalu:");
    }
    displayLinkedList( & C);
    printf("\n");

    freeLinkedList( & C);

    if (obala == 1) {
      obala = 0;
    } else {
      obala = 1;
    }

    if (obala) {
      printf("C: prazan na desnoj obali\n");
    } else {
      printf("C: prazan na lijevoj obali\n");
    }
    if (obala == 0) {
      printf("C[L]={");
      displayLinkedList( & C);
      printf("} LO={");
      displayLinkedList( & LO);
      printf("} DO={");
      displayLinkedList( & DO);
      printf("}\n");
    } else {
      printf("C[D]={");
      displayLinkedList( & C);
      printf("} LO={");
      displayLinkedList( & LO);
      printf("} DO={");
      displayLinkedList( & DO);
      printf("}\n");
    }
    printf("\n");

    // update variable
    boat_travellers = 0;
    misionari_broj = 0;
    kanibali_broj = 0;

    pthread_cond_signal( & condReady);
    pthread_mutex_unlock( & mutex);
  }
}

void * create(void * arg) {
  char str[4];
  char numbStr[3];
  while (1) {
    sleep(1);
    sprintf(str, "K%d", K);
    pthread_attr_t attr;
    pthread_attr_init( & attr);
    pthread_attr_setdetachstate( & attr, PTHREAD_CREATE_DETACHED);
    pthread_create( & kanibali_threads[K - 1], & attr, kanibali, & str);

    sleep(1);
    sprintf(str, "M%d", M);
    pthread_create( & misionari_threads[M - 1], & attr, misionari, & str);

    K++;
    M++;
  }
  return 0;

}

int main() {
  srand((unsigned) time(0));
  initializeLinkedList( & LO);
  initializeLinkedList( & DO);
  initializeLinkedList( & C);

  pthread_mutex_init( & mutex, NULL);
  pthread_cond_init( & condBoat, NULL);
  pthread_cond_init( & condTraveler, NULL);
  pthread_cond_init( & condReady, NULL);

  pthread_create( & theBoat, NULL, boat, NULL);
  pthread_create( & theCreator, NULL, create, & threadId);


  pthread_join(theCreator, NULL);
  freeLinkedList( & LO);
  freeLinkedList( & DO);
  freeLinkedList( & C);
  printf("Oslobodio sam oba Queuea i dretvu stvoritelja\n");


  pthread_mutex_destroy( & mutex);
  pthread_cond_destroy( & condBoat);
  pthread_cond_destroy( & condTraveler);
  pthread_cond_destroy( & condReady);

  return 0;
}