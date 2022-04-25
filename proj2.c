#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>

#define TRUE 1
#define FALSE 0
#define MAX_TIME 1000
#define FROM_MICRO_TO_MILI 1000

#define MMAP(ptr) {(ptr) = mmap(NULL, sizeof(*(ptr)) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);}
#define MUNMAP(ptr) {munmap((ptr), sizeof((ptr)));}
#define PRINT_PROC(proc_func) {sem_wait(write_enable); proc_func; sem_post(write_enable);}

//pouzitie programu
const char usage[] = "%s [OXYGEN AMOUNT] [HYDROGEN AMOUNT] [ATOM MAX WAIT TIME TO JOIN QUEUE (0-1000 ms)] [CREATE MOLECULE MAX WAIT TIME (0-1000 ms)]\n";

//cislo akcie, prvok, id prvku, akcia
const char output_format[] = "%d: %c %d: %s\n";
char * buffer = NULL;

//semafory
sem_t * write_enable = NULL;
sem_t * mutex = NULL;
sem_t * H_queue = NULL;
sem_t * O_queue = NULL;
sem_t * barier = NULL;


//zdielane citace
int * action_num = NULL;
int * no_m = NULL;
int * no_o = NULL;
int * no_h = NULL;

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
    sem_close(write_enable);
    sem_close(mutex);
    sem_close(barier);
    sem_close(O_queue);
    sem_close(H_queue);
    
    //odstranenie semaforov
    sem_unlink("/xponec01.ios.proj2.sem_we");
    sem_unlink("/xponec01.ios.proj2.sem_mutex");
    sem_unlink("/xponec01.ios.proj2.sem_barier");
    sem_unlink("/xponec01.ios.proj2.sem_Hq");
    sem_unlink("/xponec01.ios.proj2.sem_Oq");
    
    exit(exit_type);
}

//chybove ukoncenie s chybovou hlaskou
void error_exit(char * msg){
    fprintf(stderr, "%s\n", msg);
    exit_and_clean(EXIT_FAILURE);
}

//inicializacia zdielanej pamate a vystupneho suboru
void init()
{
    if((write_enable = sem_open("/xponec01.ios.proj2.sem_we", O_CREAT | O_EXCL , 0660, 0)) == SEM_FAILED)
    {
        fprintf(stderr,"write_enable failed.\n");
        exit_and_clean(EXIT_FAILURE);
    }
    if((mutex = sem_open("/xponec01.ios.proj2.sem_mutex", O_CREAT | O_EXCL, 0660, 1)) == SEM_FAILED)
    {
        fprintf(stderr,"mutex failed.\n");
        exit_and_clean(EXIT_FAILURE);
    }
    if((H_queue = sem_open("/xponec01.ios.proj2.sem_Hq", O_CREAT | O_EXCL, 0660, 1)) == SEM_FAILED)
    {
        fprintf(stderr,"H_queue failed.\n");
        exit_and_clean(EXIT_FAILURE);
    }
    if((O_queue = sem_open("/xponec01.ios.proj2.sem_Oq", O_CREAT | O_EXCL, 0660, 1)) == SEM_FAILED)
    {
        fprintf(stderr,"O_queue failed.\n");
        exit_and_clean(EXIT_FAILURE);
    }
    if((barier = sem_open("/xponec01.ios.proj2.sem_barier", O_CREAT | O_EXCL, 0660, 1)) == SEM_FAILED)
    {
        fprintf(stderr,"barier failed.\n");
        exit_and_clean(EXIT_FAILURE);
    }
    
    MMAP(action_num);
    if(action_num == MAP_FAILED)
    {
        exit_and_clean(EXIT_FAILURE);
    }
    MMAP(no_m);
    if(no_m == MAP_FAILED)
    {
        exit_and_clean(EXIT_FAILURE);
    }
    MMAP(no_h);
    if(no_h == MAP_FAILED)
    {
        exit_and_clean(EXIT_FAILURE);
    }
    MMAP(no_o);
    if(no_o == MAP_FAILED)
    {
        exit_and_clean(EXIT_FAILURE);
    }

    out_file = fopen("proj2.out", "w");
    *action_num = 1;
    *no_m = 1;
    *no_h = 1;
    *no_o = 1;
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

    //PROCES NO
    // • Každý kyslík je jednoznačně identifikován číslem idO, 0<idO<=NO
    // • Po spuštění vypíše: A: O idO: started
    // • Vypíše: A: O idO: going to queue a zařadí se do fronty kyslíků na vytváření molekul.

    // • Ve chvíli, kdy není vytvářena žádná molekula, jsou z čela front uvolněny kyslík a dva vodíky.
    // Příslušný proces po uvolnění vypíše: A: O idO: creating molecule noM (noM je číslo molekuly,
    // ty jsou číslovány postupně od 1).
    // • Pomocí usleep na náhodný čas v intervalu <0,TB> simuluje dobu vytváření molekuly.
    // • Po uplynutí času vytváření molekuly informuje vodíky ze stejné molekuly, že je molekula
    // dokončena.

    // • Vypíše: A: O idO: molecule noM created a proces končí.
    // • Pokud již není k dispozici dostatek vodíků (ani nebudou žádné další vytvořeny/zařazeny do (queue_O prazdna && idO == NO)
    // fronty) vypisuje: A: O idO: not enough H a proces končí.

void process_NO(int TI, int TB, FILE * file)
{
    sem_post(write_enable);
    int id_o = *no_o;
    *no_o += 1;
    srand(time(NULL));

    PRINT_PROC(fprintf(file,output_format,(*action_num)++,'O',id_o,"started"));
    
    usleep((rand() % TI +1)*FROM_MICRO_TO_MILI); //TI + 1 kvoli intervalu <0,TI>

    PRINT_PROC(fprintf(file,output_format,(*action_num)++,'O',id_o,"going to queue"));

    PRINT_PROC(fprintf(file,output_format,(*action_num)++,'O',id_o,"creating molecule"));

    usleep((rand() % TB +1)*FROM_MICRO_TO_MILI);

    PRINT_PROC(fprintf(file,output_format,(*action_num)++,'O',id_o,"molecule created"));//potom end process 


    exit(EXIT_SUCCESS);
}

    //PROCES NH
    // Každý vodík je jednoznačně identifikován číslem idH, 0<idH<=NO
    // • Po spuštění vypíše: A: H idH: started
    // • Následně čeká pomocí volání usleep náhodný čas v intervalu <0,TI>
    // • Vypíše: A: H idH: going to queue a zařadí se do fronty vodíků na vytváření molekul.

    // • Ve chvíli, kdy není vytvářena žádná molekula, jsou z čela front uvolněny kyslík a dva vodíky.
    // Příslušný proces po uvolnění vypíše: A: H idH: creating molecule noM (noM je číslo molekuly,
    // ty jsou číslovány postupně od 1).

    // • Následně čeká na zprávu od kyslíku, že je tvorba molekuly dokončena.
    // • Vypíše: A: H idH: molecule noM created a proces končí.
    // • Pokud již není k dispozici dostatek vodíků (ani nebudou žádné další vytvořeny/zařazeny do
    // fronty) vypisuje: A: H idH: not enough O or H a process končí.

void process_NH(int TI, FILE * file)
{
    
    int idH = *no_h;
    *no_h += 1;
    srand(time(NULL));

    PRINT_PROC(fprintf(file,output_format,(*action_num)++,'H',idH,"started"));

    usleep((rand() % TI +1)*FROM_MICRO_TO_MILI); //TI + 1 kvoli intervalu <0,TI>

    PRINT_PROC(fprintf(file,output_format,(*action_num)++,'H',idH,"going to queue")); 

    exit(EXIT_SUCCESS);
}

    //PROCES HL - az po predoslych dvoch
    // Hlavní proces vytváří ihned po spuštění NO procesů kyslíku a NH procesů vodíku.
    // • Poté čeká na ukončení všech procesů, které aplikace vytváří. Jakmile jsou tyto procesy
    // ukončeny, ukončí se i hlavní proces s kódem (exit code) 0.

void process_main(unsigned int NO, unsigned int NH, unsigned int TI, unsigned int TB) //po ukonceni oboch procesov sa proces ukonci
{
    for(unsigned int i = 0; i < NH; i++)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            process_NH(TI,stdout);  
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
            process_NO(TI,TB,stdout);   
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
    init();

    //overenie poctu argumentov
    if(argc != 5)
    {   
        error_exit("Zlý počet argumentov.");
    }

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
    if(TI > MAX_TIME || TB > MAX_TIME)
    {
        error_exit("Zadaný príliš dlhý časový rozsah.");
    }

    process_main(NO,NH,TI,TB);
    while (wait(NULL) > 0);//cyklí dokial su child procesy aktivne
    
    exit_and_clean(EXIT_SUCCESS);
    return 0;
}


// PARSING ZADANIA

// Tri procesy:
// 0 - hl proces 
// 1 - kyslik
// 2 - vodik

// Dve rady: 
// a) pre kysliky 
// b) pre vodiky

// naraz je mozne vytvarat iba jednu molekulu. procesy uvolnia miesto dalsim atomov na vytvorenie dalsej molekuly a skoncia.
// ak nie je k dispozici dostatok atomov na vytvorenie vodika (ziadne dalsie moleuly nebudu procesom 0 vytvorene),
//  vsetky atomy uvolnit a skoncit proces. 

// • Použijte sdílenou paměť pro implementaci čítače akcí a sdílených proměnných nutných pro
// synchronizaci.
// • Použijte semafory pro synchronizaci procesů.
// • Nepoužívejte aktivní čekání (včetně cyklického časového uspání procesu) pro účely
// synchronizace.

// actions:
// started 
// going to queue
// creating molecule
// molecule created 
// not enough <element>

