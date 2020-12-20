#ifndef __SPAWN__
#define __SPAWN__

#include "misc.h"

#define BUFFER_SIZE 13

typedef struct missile_code {
  int numprocs;
  char really_important_char;
} missile_code;

typedef struct circularBuffer {
    int head;
    int tail;
    char buf[BUFFER_SIZE];
    lock_t lock;
    sem_t s_fullslots;
    sem_t s_emptyslots;

}cBuf;

#define PRODUCER "producer.dlx.obj"
#define CONSUMER "consumer.dlx.obj"


#endif
