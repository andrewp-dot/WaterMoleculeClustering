#ifndef PROJ2_H
#define PROJ2_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdarg.h>

#define TRUE 1
#define FALSE 0
#define MAX_TIME 1000
#define FROM_MICRO_TO_MILI 1000

#define MMAP(ptr) {(ptr) = mmap(NULL, sizeof(*(ptr)) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);};
#define MUNMAP(ptr) {munmap((ptr), sizeof((ptr)));}
#define USE_SHM(proc_func) {sem_wait(write_enable); proc_func; sem_post(write_enable);}

//semafory
#define WRITE_ENABLE_SEM "/xponec01.ios.proj2.sem_we"
#define MUTEX_SEM "/xponec01.ios.proj2.sem_mutex"
#define BARIER_H "/xponec01.ios.proj2.sem_barh"
#define BARIER_O "/xponec01.ios.proj2.sem_barO"
#define H_QUEUE "/xponec01.ios.proj2.sem_Hq"
#define O_QUEUE "/xponec01.ios.proj2.sem_Oq"
#define BONDING_FINISHED "/xponec01.ios.proj2.sem_bondf"
#define BOND "/xponec01.ios.proj2.sem_bond"

#endif
