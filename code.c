#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_t thread1, thread2, thread3;
pthread_mutex_t write_mutex;
sem_t buffer1_get_sem, buffer1_put_sem, buffer2_get_sem, buffer2_put_sem;

int buffer1 = 0, buffer2 = 0;

void printfSync(char* fstring, int index, int delay) {
    pthread_mutex_lock(&write_mutex);
    printf(fstring, index, delay);
    pthread_mutex_unlock(&write_mutex);
}

void putsSync(char* string) {
    pthread_mutex_lock(&write_mutex);
    puts(string);
    pthread_mutex_unlock(&write_mutex);
}

void *worker1_doWork(void *arg) {
    int pin_count = *(int *) arg;
    int i = 1;
    while (i <= pin_count) {
        int delay = rand() % 5;
        sleep(delay);
        int is_normal_pin = rand() % 2;
        if (is_normal_pin) {
            printfSync("First worker checked pin%d in %d seconds. Verdict: pin is normal\n", i, delay);
            sem_wait(&buffer1_put_sem);
            buffer1 = i;
            sem_post(&buffer1_get_sem);
        } else {
            printfSync("First worker checked pin%d in %d seconds. Verdict: pin is crooked\n", i, delay);
        }
        ++i;
    }
    putsSync("First worker processed all the pins\n");
    return NULL;
}

void *worker2_doWork(void *arg) {
    while (1) {
        sem_wait(&buffer1_get_sem);
        int i = buffer1;
        buffer1 = 0;
        sem_post(&buffer1_put_sem);
        int delay = rand() % 5;
        sleep(delay);
        printfSync("Second worker sharpened pin%d in %d seconds\n", i, delay);
        sem_wait(&buffer2_put_sem);
        buffer2 = i;
        sem_post(&buffer2_get_sem);
    }
    return NULL;
}

void *worker3_doWork(void * arg) {
    while (1) {
        sem_wait(&buffer2_get_sem);
        int i = buffer2;
        buffer2 = 0;
        sem_post(&buffer2_put_sem);
        int delay = rand() % 5;
        sleep(delay);
        int is_normal_pin = rand() % 2;
        if (is_normal_pin) {
            printfSync("Third worker checked pin%d in %d seconds. Verdict: pin is qualitative\n", i, delay);
        } else {
            printfSync("Third worker checked pin%d in %d seconds. Verdict: pin isn't qualitative\n", i, delay);
        }
    }
    return NULL;
}

int main() {
    pthread_mutex_init(&write_mutex, NULL);
    sem_init(&buffer1_put_sem, 0, 1);
    sem_init(&buffer1_get_sem, 0, 0);
    sem_init(&buffer2_put_sem, 0, 1);
    sem_init(&buffer2_get_sem, 0, 0);
    int pin_count;
    puts("Enter the number of pins:");
    scanf("%d", &pin_count);
    pthread_create(&thread1, NULL, worker1_doWork, (void*)&pin_count);
    pthread_create(&thread2, NULL, worker2_doWork, NULL);
    pthread_create(&thread3, NULL, worker3_doWork, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
}
