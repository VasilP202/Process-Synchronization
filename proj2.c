/*      IOS projekt 2 - Process synchronization
 *      FIT VUT
 * 
 *      Author: Vasil Poposki
 *      Date: May 2021       
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#define shmSIZE sizeof(int)

// Shared variables keys
#define elvesKEY "/shm-elves"
#define rdsKEY "/shm-rds"
#define acntKEY "/shm-acnt"
#define finishKEY "/shm-finish"

sem_t *santaSem, *santaHelp, *santaHelpDone, *reindeerSem, *elfTex, *gateSem, *acntSem;

FILE *f; // Output file

void createResources() {

    int elvesID, rdsID, acntID, finishID;
    int *elves, *rds, *acnt, *finish;
    
    /**** Set shared variables ****/
    elvesID = shm_open(elvesKEY, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(elvesID, shmSIZE);
    elves = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, elvesID, 0);
    close(elvesID);
    *elves = 0;     // Initialize elves counter
    munmap(elves, shmSIZE);

    rdsID = shm_open(rdsKEY, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(rdsID, shmSIZE);
    rds = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, rdsID, 0);
    close(rdsID);
    *rds = 0;       // Initialize reindeer counter
    munmap(rds, shmSIZE);

    acntID = shm_open(acntKEY, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(acntID, shmSIZE);
    acnt = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, acntID, 0);
    close(acntID);
    *acnt = 0;       // Initialize action counter
    munmap(acnt, shmSIZE);

    finishID = shm_open(finishKEY, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(finishID, shmSIZE);
    finish = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, finishID, 0);
    close(finishID);
    *finish = 0;     // Initialize elves counter
    munmap(finish, shmSIZE);

    /**** Set semaphores ****/
    santaSem = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    sem_init(santaSem, 1, 0);
    
    santaHelp = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    sem_init(santaHelp, 1, 0);
    
    santaHelpDone = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    sem_init(santaHelpDone, 1, 0);

    reindeerSem = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    sem_init(reindeerSem, 1, 0);

    elfTex = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    sem_init(elfTex, 1, 3);

    gateSem = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    sem_init(gateSem, 1, 1);

    acntSem = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    sem_init(acntSem, 1, 1);

}

void cleanResources() {
    fclose(f);
    shm_unlink(elvesKEY);
    shm_unlink(rdsKEY);
    shm_unlink(acntKEY);
    shm_unlink(finishKEY);

    sem_destroy(santaSem);
    sem_destroy(santaHelp);
    sem_destroy(santaHelpDone);
    sem_destroy(reindeerSem);
    sem_destroy(elfTex);
    sem_destroy(gateSem);
    sem_destroy(acntSem);
}

void santaproc(int rdsNUM) {

    int elvesID, rdsID, acntID, finishID;
    int *elves, *rds, *acnt, *finish;

    /**** Open shared memory ****/
    elvesID = shm_open(elvesKEY, O_RDWR, S_IRUSR | S_IWUSR);
    elves = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, elvesID, 0);
    close(elvesID);

    rdsID = shm_open(rdsKEY, O_RDWR, S_IRUSR | S_IWUSR);
    rds = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, rdsID, 0);
    close(rdsID);

    acntID = shm_open(acntKEY, O_RDWR, S_IRUSR | S_IWUSR);
    acnt = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, acntID, 0);
    close(acntID);

    finishID = shm_open(finishKEY, O_RDWR, S_IRUSR | S_IWUSR);
    finish = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, finishID, 0);
    close(finishID);

    while(1) {
        sem_wait(acntSem);
        fprintf(f, "%d: Santa: going to sleep\n", ++(*acnt));
        sem_post(acntSem);
        
        sem_wait(santaSem); // Locks semaphore-> waiting for elf/reindeer to signal
        sem_wait(gateSem);
        if(*elves == 3) {
            sem_wait(acntSem);
            fprintf(f, "%d: Santa: helping elves\n", ++(*acnt));
            sem_post(acntSem);

            sem_post(santaHelp);
            sem_post(gateSem);
            sem_wait(santaHelpDone);
        } else if(*rds == rdsNUM) {
            sem_wait(acntSem);
            fprintf(f, "%d: Santa: closing workshop\n", ++(*acnt));
            sem_post(acntSem);
            
            *finish = 1;
            *rds = 0;
            for(int i = 0; i < rdsNUM; i++)
                sem_post(reindeerSem);
            
            sem_wait(santaSem);
            // Hitching reindeers

            sem_wait(acntSem);
            fprintf(f, "%d: Santa: Christmas started\n", ++(*acnt));
            sem_post(acntSem);
            
            sem_post(gateSem);
            sem_post(santaHelp);
            exit(0);
        }

    }

    /**** Unmap memory ****/
    munmap(elves, shmSIZE);
    munmap(rds, shmSIZE);
    munmap(acnt, shmSIZE);

}

void elfproc(int elfID, int sleepMAX) {

    int elvesID, acntID, finishID;
    int *elves, *acnt, *finish;

    /**** Open shared memory ****/
    elvesID = shm_open(elvesKEY, O_RDWR, S_IRUSR | S_IWUSR);
    elves = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, elvesID, 0);
    close(elvesID);

    acntID = shm_open(acntKEY, O_RDWR, S_IRUSR | S_IWUSR);
    acnt = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, acntID, 0);
    close(acntID);

    finishID = shm_open(finishKEY, O_RDWR, S_IRUSR | S_IWUSR);
    finish = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, finishID, 0);
    close(finishID);

    sem_wait(acntSem);
    fprintf(f, "%d: Elf %d: started\n", ++(*acnt), elfID);
    sem_post(acntSem);
    
    while(1) {
        // Find random value in interval
        int sleeptime = (rand() % sleepMAX) * 1000;
        usleep(sleeptime);

        sem_wait(acntSem);
        fprintf(f, "%d: Elf %d: need help\n", ++(*acnt), elfID);
        sem_post(acntSem);
        
        sem_wait(elfTex);
        sem_wait(gateSem);
        if(*finish == 1) {
            sem_post(gateSem);
            break;
        }
        *elves = *elves + 1;
        if(*elves == 3) {
            sem_post(santaSem);
        }
        sem_post(gateSem);
        sem_wait(santaHelp);
        if(*finish == 1)
            break;

        *elves = *elves - 1;
        sem_wait(acntSem);
        fprintf(f, "%d: Elf %d: get help\n", ++(*acnt), elfID);
        sem_post(acntSem);

        if(*elves == 0) {
            sem_post(santaHelpDone);
            sem_post(elfTex);
            sem_post(elfTex);
            sem_post(elfTex);
        } else {
            sem_post(santaHelp);
        }

    }

    sem_wait(acntSem);
    fprintf(f, "%d: Elf %d: taking holidays\n", ++(*acnt), elfID);
    sem_post(acntSem);
    
    sem_post(elfTex);
    sem_post(santaHelp);

    /**** Unmap memory ****/
    munmap(elves, shmSIZE);
    munmap(acnt, shmSIZE);
    munmap(finish, shmSIZE);

}

void rdproc(int rdID, int rdsNUM, int sleepMAX) {

    int rdsID, acntID;
    int *rds, *acnt;

    /**** Open shared memory ****/
    rdsID = shm_open(rdsKEY, O_RDWR, S_IRUSR | S_IWUSR);
    rds = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, rdsID, 0);
    close(rdsID);

    acntID = shm_open(acntKEY, O_RDWR, S_IRUSR | S_IWUSR);
    acnt = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, acntID, 0);
    close(acntID);

    sem_wait(acntSem);
    fprintf(f, "%d: RD %d: rstarted\n", ++(*acnt), rdID);
    sem_post(acntSem);

    // Find random value in interval
    int sleeptime = ((rand() % (sleepMAX/2)) + (sleepMAX/2)) * 1000;

    usleep(sleeptime);
    
    sem_wait(acntSem);
    fprintf(f, "%d: RD %d: return home\n", ++(*acnt), rdID);
    sem_post(acntSem);
    
    *rds = *rds + 1; // New reindeer has arrived

    if(*rds == rdsNUM)
        sem_post(santaSem); // Wake up Santa

    sem_wait(reindeerSem);

    sem_wait(acntSem);
    fprintf(f, "%d: RD %d: get hitched\n", ++(*acnt), rdID);
    sem_post(acntSem);
    
    *rds = *rds + 1; // Reindeer is hitched
    
    if(*rds == rdsNUM)
        // Signal Santa -> reindeers are ready to go
        sem_post(santaSem); 


    /**** Unmap memory ****/
    munmap(rds, shmSIZE);
    munmap(acnt, shmSIZE);
}

int main(int argc, char *argv[]) {

    /**** Parsing input arguments ****/
    int NE, NR, TE, TR;
    if(argc != 5) {
        fprintf(stderr, "Arguments error.\n");
        return 1;
    }
    for(int i = 1; i < argc; i++) {
        for(int j = 0; argv[i][j] != '\0'; j++) {
            if(argv[i][j] < 48 || argv[i][j] > 57) {
                fprintf(stderr, "exit: Arguments error\n");
                    return 1;
            }
        }
    }

    NE = strtol(argv[1], NULL, 10);   // Number of elves
    NR = strtol(argv[2], NULL, 10);   // Number of reindeers
    TE = strtol(argv[3], NULL, 10);   // Elf sleep time
    TR = strtol(argv[4], NULL, 10);   // Reindeer sleep time

    if((NE < 1 || NE >= 1000) || (NR < 1 || NR >= 20) || (TE > 1000 || TR > 1000)) {
        fprintf(stderr, "exit: Arguments error\n");
        return 1;
    }

    f = fopen("proj2.out", "w"); // Open output file for writing 
    if(f == NULL)
        return -1;
    
    setbuf(f, NULL);
    setbuf(stderr, NULL);

    srand(time(NULL));

    createResources();
   
    int pid;
    pid = fork();
    if(pid < 0) {
        fprintf(stderr, "exit: Fork error.");
        cleanResources();
        return 2;
    } else if(pid == 0){ //Child
        santaproc(NR);
        return 0;
    } else { //Parent
        for(int i = 0; i < NE; i++) { 
            // Create NE elf processes
            int elfID = fork();
            if(elfID < 0) {
                fprintf(stderr, "exit: Fork error.");
                cleanResources();
                return 2;
            } else if(elfID == 0) {
                elfproc(i+1, TE);
                return 0;
            }
        }
        for(int i = 0; i < NR; i++) {
            // Create NR reindeer processes
            int rdID = fork();
            if(rdID < 0) {
                fprintf(stderr, "exit: Fork error.");
                cleanResources();
                return 2;
            } else if(rdID == 0) {
                rdproc(i+1, NR, TR);
                return 0;
            }
        }
    }

    while(wait(NULL)) { 
        // Wait for child processes to finish
        if(errno == ECHILD)
            break;
    }

    cleanResources();

    return 0;
}