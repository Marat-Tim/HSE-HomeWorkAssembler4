#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int max_delay = 5;

// Объявление потоков.
pthread_t thread1, thread2, thread3;
// Мьютекс для консольного вывода.
pthread_mutex_t write_mutex;
// Семафоры для двух буферов, на чтение и запись.
sem_t buffer1_get_sem, buffer1_put_sem, buffer2_get_sem, buffer2_put_sem;

// Сами буферы. Хранят индекс булавки.
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
        // Имитация работы.
        int delay = rand() % max_delay;
        sleep(delay);

        int is_normal_pin = rand() % 2;
        if (is_normal_pin) {
            printfSync("First worker checked pin%d in %d seconds. Verdict: pin is normal\n", i, delay);
            // Ждет когда можно будет положить булавку в первый буфер.
            sem_wait(&buffer1_put_sem);
            // Записывает индекс текущей булавки в первый буфер.
            buffer1 = i;
            // Сообщает, что можно взять булавку из первого буфера.
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
        // Ждет когда можно будет взять булавку из первого буфера.
        sem_wait(&buffer1_get_sem);
        // Сохраняет номер булавки из буфера в локальную переменную.
        int i = buffer1;
        // Очищает буфер.
        buffer1 = 0;
        // Сообщает, что можно положить булавку в первый буфер.
        sem_post(&buffer1_put_sem);
        // Имитация работы.
        int delay = rand() % max_delay;
        sleep(delay);
        printfSync("Second worker sharpened pin%d in %d seconds\n", i, delay);
        // Ждет когда можно будет положить булавку во второй буфер.
        sem_wait(&buffer2_put_sem);
        // Записывает индекс текущей булавки во второй буфер.
        buffer2 = i;
        // Сообщает, что можно взять булавку из второго буфера.
        sem_post(&buffer2_get_sem);
    }
    return NULL;
}

// В общем тут по аналогии.
void *worker3_doWork(void * arg) {
    while (1) {
        sem_wait(&buffer2_get_sem);
        int i = buffer2;
        buffer2 = 0;
        sem_post(&buffer2_put_sem);
        int delay = rand() % max_delay;
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

int main(int argc, char** argv) {
    if (argc == 2) {
        max_delay = atoi(argv[1]);
    }
    pthread_mutex_init(&write_mutex, NULL);
    // Изначально в первый буфер можно положить булавку, но нельзя взять.
    sem_init(&buffer1_put_sem, 0, 1);
    sem_init(&buffer1_get_sem, 0, 0);
    // Аналогично со вторым.
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
