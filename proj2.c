#include "proj2.h"

//pouzitie programu
const char usage[] = "%s [OXYGEN AMOUNT] [HYDROGEN AMOUNT] [ATOM MAX WAIT TIME TO JOIN QUEUE (0-1000 ms)] [CREATE MOLECULE MAX WAIT TIME (0-1000 ms)]\n";

//cislo akcie, prvok, id prvku, akcia
const char output_format[] = "%d: %c %d: ";
char * buffer = NULL;

//semafory
sem_t * write_enable = NULL;
sem_t * mutex = NULL; //create_molecule process
sem_t * H_queue = NULL; //queues do kriza na execute kyslikov vo vodiku a naopak
sem_t * O_queue = NULL;
sem_t * barier_H = NULL; //create_molecule process
sem_t * barier_O = NULL;
sem_t * bonding_finished = NULL; 
sem_t * bond = NULL;


//zdielane citace
int * action_num = NULL;
int * no_m = NULL;
int * no_o = NULL;
int * no_h = NULL;
int * id_o = NULL;
int * id_h = NULL;

FILE * out_file = NULL; 

//ukoncenie suboru a uvolnenie vsetkeho
void exit_and_clean(int exit_type)
{
    //zatvaranie suborov a uvolnenie zdielanej pamate
    fclose(out_file);
    MUNMAP(action_num);
    MUNMAP(no_m);
    MUNMAP(no_h);
    MUNMAP(no_o);

    //zatvaranie semaforov 
    sem_close(write_enable); //neuvolni sa kvoli interuptu - vyriesit not enough [ELEMENT]
    sem_close(mutex);
    sem_close(barier_H);
    sem_close(barier_O);
    sem_close(O_queue);
    sem_close(H_queue);
    sem_close(bonding_finished);
    sem_close(bond);
    
    //odstranenie semaforov
    sem_unlink(WRITE_ENABLE_SEM);
    sem_unlink(MUTEX_SEM);
    sem_unlink(H_QUEUE);
    sem_unlink(O_QUEUE);
    sem_unlink(BONDING_FINISHED);
    sem_unlink(BOND);
    sem_unlink(BARIER_H);
    sem_unlink(BARIER_O);

    exit(exit_type);
}

//chybove ukoncenie s chybovou hlaskou
void error_exit(char * msg){
    fprintf(stderr, "%s\n", msg);
    exit_and_clean(EXIT_FAILURE);
}

//inicializacia zdielanej pamate a vystupneho suboru
int init()
{
    if((write_enable = sem_open(WRITE_ENABLE_SEM, O_CREAT | O_EXCL , 0660, 1)) == SEM_FAILED)
    {
        fprintf(stderr,"%s failed.\n", WRITE_ENABLE_SEM);
        return EXIT_FAILURE;
    }
    if((mutex = sem_open(MUTEX_SEM, O_CREAT | O_EXCL, 0660, 1)) == SEM_FAILED)
    {
        fprintf(stderr,"%s failed.\n",MUTEX_SEM);
        return EXIT_FAILURE;
    }
    if((H_queue = sem_open(H_QUEUE, O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED)
    {
        fprintf(stderr,"%s failed.\n",H_QUEUE);
        return EXIT_FAILURE;
    }
    if((O_queue = sem_open(O_QUEUE, O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED)
    {
        fprintf(stderr,"%s failed.\n",O_QUEUE);
        return EXIT_FAILURE;
    }
    if((barier_H = sem_open(BARIER_H, O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED)
    {
        fprintf(stderr,"%s failed.\n", BARIER_H);
        return EXIT_FAILURE;
    }
    if((barier_O = sem_open(BARIER_O, O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED)
    {
        fprintf(stderr,"%s failed.\n", BARIER_O);
        return EXIT_FAILURE;
    }
    if((bonding_finished = sem_open(BONDING_FINISHED, O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED)
    {
        fprintf(stderr,"%s failed.\n", BONDING_FINISHED);
        return EXIT_FAILURE;
    }
    if((bond = sem_open(BOND, O_CREAT | O_EXCL, 0660, 1)) == SEM_FAILED)
    {
        fprintf(stderr,"%s failed.\n", BOND);
        return EXIT_FAILURE;
    }

    
    MMAP(action_num);
    if(action_num == MAP_FAILED) return EXIT_FAILURE;
    
    MMAP(no_m);
    if(no_m == MAP_FAILED) return EXIT_FAILURE;
  
    MMAP(no_h);
    if(no_h == MAP_FAILED) return EXIT_FAILURE;

    MMAP(no_o);
    if(no_o == MAP_FAILED) return EXIT_FAILURE;

    MMAP(id_o);
    if(id_o == MAP_FAILED) return EXIT_FAILURE;

    MMAP(id_h);
    if(id_h == MAP_FAILED) return EXIT_FAILURE;
    
    out_file = fopen("proj2.out", "w");
    *action_num = 1;
    *no_m = 1;
    *no_h = 0;
    *no_o = 0;
    *id_h = 1;
    *id_o = 1;
    return EXIT_SUCCESS;
}

//print do suboru
void my_fprintf(FILE * file,char elem, int local_id, const char * fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    
    fprintf(file,output_format,(*action_num)++,elem,local_id);
    vfprintf(file, fmt, args);

    va_end(args);
}

//overenie vstupneho stringu 
int is_positive_number(char * num)
{
    int i = 0;
    while (num[i] != '\0')
    {
        if(num[i] > '9' || num [i] < '0')
        {
            return FALSE;
        }
        i++;
    }
    return TRUE;
}

void process_NO(int NH, int NO,int TI, int TB, FILE * file)
{
    //inicializacia ID kysliku
    int local_ido = *id_o;
    int was_bond = FALSE;

    //semafor na pristup do pamate - aby sa inkrementovala premenna v jeden cas iba raz
    
    USE_SHM(*id_o += 1);
    
    srand(time(NULL));

    USE_SHM(my_fprintf(file,'O',local_ido,"started\n"));
    
    usleep((rand() % TI +1)*FROM_MICRO_TO_MILI); //TI + 1 kvoli intervalu <0,TI>

    sem_post(O_queue);
    USE_SHM(my_fprintf(file,'O',local_ido,"going to queue \n"));
    
    sem_wait(mutex);
    
    int local_mol = *no_m;
    USE_SHM((*no_o)++);

    if(*no_h >= 2) 
    {
        sem_wait(bond);

        sem_post(barier_H);
        sem_post(barier_H);

        USE_SHM((*no_h) -= 2);

        sem_post(barier_O);
        USE_SHM((*no_o)--);

        was_bond = TRUE;

        sem_post(bond);

    }   
    else
    {
        sem_post(mutex);
    }

    sem_wait(barier_O);


    if( *no_m > NO || *no_m*2 > NH) 
    {
        USE_SHM(my_fprintf(file,'O',local_ido,"Not enough H\n"));
        sem_post(barier_H);
        sem_post(barier_O);
        exit(EXIT_SUCCESS);
    }
        
    USE_SHM(my_fprintf(file,'O',local_ido,"creating molecule %d \n",  local_mol));
    usleep((rand() % TB +1)*FROM_MICRO_TO_MILI);
        
    USE_SHM(my_fprintf(file,'O',local_ido,"molecule %d created\n",  local_mol));

    if(was_bond)
    {
        sem_wait(O_queue);
    }

    if(*no_m*2 < NH && *no_m == NO)
    {
        sem_post(barier_H);
    }
  
    sem_post(bonding_finished); //informovat dva kysliky o ukonceni procesu zlucovania
    sem_post(bonding_finished);

    (*no_m)++;

    sem_post(mutex);

    exit(EXIT_SUCCESS);
}

void process_NH(int NO, int NH, int TI, FILE * file)
{
    //inicializacia ID vodiku
    int local_idh = *id_h;
    int was_bond = FALSE;

    USE_SHM(*id_h += 1;);

    // printf("A %d\n", local_idh);
    srand(time(NULL));

    USE_SHM(my_fprintf(file,'H',local_idh,"started\n"));

    usleep((rand() % TI +1)*FROM_MICRO_TO_MILI); //TI + 1 kvoli intervalu <0,TI>

    sem_post(H_queue);
    USE_SHM(my_fprintf(file,'H',local_idh,"going to queue\n")); 

    sem_wait(mutex);

    USE_SHM((*no_h)++;);

    if(*no_h >= 2 && *no_o >= 1) 
    {
        sem_wait(bond);

        sem_post(barier_H);
        sem_post(barier_H);
        
        USE_SHM((*no_h) -= 2;);

        sem_post(barier_O);
        USE_SHM((*no_o)--;);

        was_bond = TRUE;

        sem_post(bond);
    }
    else
    {
        sem_post(mutex);
    }

    sem_wait(barier_H);

    int local_mol = *no_m;
    if( *no_m > NO || *no_m*2 > NH ) 
    {
        USE_SHM(my_fprintf(file,'H',local_idh,"Not enough O or H\n"));
        sem_post(barier_H);
        sem_post(barier_O);
        exit(EXIT_SUCCESS);
    }

    USE_SHM(my_fprintf(file,'H',local_idh,"creating molecule %d\n",  local_mol));

    sem_wait(bonding_finished);

    USE_SHM(my_fprintf(file,'H',local_idh,"molecule %d created\n",  local_mol));

    if(was_bond)
    {
        sem_wait(H_queue);
    }

    if(*no_m < NO && *no_m * 2 == NH)
    {
        sem_post(barier_O);
    }

    exit(EXIT_SUCCESS);
}


void process_main(unsigned int NO, unsigned int NH, unsigned int TI, unsigned int TB) //po ukonceni oboch procesov sa proces ukonci
{
    for(unsigned int i = 0; i < NH; i++)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            process_NH(NO,NH,TI,stdout);  
        }
        else if(pid < 0)
        {
            error_exit("Fork error.");
        }
    }

    for(unsigned int i = 0; i < NO; i++)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            process_NO(NH,NO,TI,TB,stdout);   
        }
        else if(pid < 0)
        {
            error_exit("Fork error.");
        }
    }
    (void)TB;
    (void)NO;
}

// M           M
// M M       M M 
// M   M   M   M 
// M     M     M
// M           M   

int main(int argc, char * argv[])
{
    //inicializacia zdielanej pamate
    if(init() == EXIT_FAILURE) exit_and_clean(EXIT_FAILURE);

    //overenie poctu argumentov
    if(argc != 5)  error_exit("Zlý počet argumentov.");
 
    //overenie format vstupnych dat - musi to byt kladne cislo
    for(int idx = 1; idx < argc; idx++)
    {
        if(!is_positive_number(argv[idx]))
        {
            error_exit("Zlý formát vstupých dát.");
        };
    }

    //nacitanie a pretypovanie vstupnych dat na cisla
    unsigned int NO = atoi(argv[1]); 
    unsigned int NH = atoi(argv[2]); 
    unsigned int TI = atoi(argv[3]); 
    unsigned int TB = atoi(argv[4]); 

    //overenie maximalnej dlzky casov
    if(TI > MAX_TIME || TB > MAX_TIME) error_exit("Zadaný príliš dlhý časový rozsah.");
       
    process_main(NO,NH,TI,TB);
    while (wait(NULL) > 0);//cyklí dokial su child procesy aktivne
    
    exit_and_clean(EXIT_SUCCESS);
    return 0;
}

//ak je pocet molekul mensi ako pocet kyslikov -> spusti proces kyslik
//ak je pocet molekul * 2 mensi ako pocet vodikov -> spusti proces vodík 
//check, ci je toho dostatok 
//check ci je to koniec a toho dostatok
