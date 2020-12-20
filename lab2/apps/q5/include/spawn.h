#ifndef __SPAWN__
#define __SPAWN__

#include "misc.h"

#define BUFFER_SIZE 13

typedef struct missile_code {
  int numprocs;
  char really_important_char;
} missile_code;

typedef struct Krypton {
    sem_t molecule_n2;
    sem_t molecule_h2o;
    sem_t atom_n;
    sem_t molecule_o2;
    sem_t molecule_h2;
    sem_t molecule_no2;

}krypton;

#define PRODUCER "producer.dlx.obj"
#define CONSUMER "consumer.dlx.obj"


#endif
