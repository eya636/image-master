#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include "image_io.h"

#define NUM_PROCESSES 4
#define NUM_THREADS_PER_PROCESS 4

typedef struct {
    unsigned char* image_data;
    int width;
    int height;
    int num_components;
    int start_row;
    int end_row;
    sem_t* sem_start;
    sem_t* sem_done;
} ThreadData;

void* process_image(void* thread_arg) {
    ThreadData* data = (ThreadData*) thread_arg;
    sem_wait(data->sem_start);

    for (int iteration = 0; iteration < 10000; iteration++) {
        for (int y = data->start_row; y < data->end_row; y++) {
            for (int x = 0; x < data->width; x++) {
                int index = (y * data->width + x) * data->num_components;
                unsigned char gray = 0.299 * data->image_data[index] + 0.587 * data->image_data[index + 1] + 0.114 * data->image_data[index + 2];
                data->image_data[index] = gray;
                data->image_data[index + 1] = gray;
                data->image_data[index + 2] = gray;
            }
        }
    }

    sem_post(data->sem_done);
    pthread_exit(NULL);
}

int main() {
    unsigned char* image_data = NULL;
    int width, height, num_components;
    const char* input_filename = "input.jpg";
    const char* output_filename = "output_parallel_process_thread_optimized.jpg";
    struct timeval start_time, end_time;
    double execution_time;

    image_data = read_jpeg_file(input_filename, &width, &height, &num_components);
    if (!image_data) {
        fprintf(stderr, "Erreur lors de la lecture du fichier JPEG.\n");
        return 1;
    }

    sem_t sem_start[NUM_PROCESSES][NUM_THREADS_PER_PROCESS];
    sem_t sem_done[NUM_PROCESSES][NUM_THREADS_PER_PROCESS];
    pthread_t threads[NUM_PROCESSES][NUM_THREADS_PER_PROCESS];
    ThreadData thread_data[NUM_PROCESSES][NUM_THREADS_PER_PROCESS];

    int rows_per_thread = height / (NUM_PROCESSES * NUM_THREADS_PER_PROCESS);

    for (int i = 0; i < NUM_PROCESSES; ++i) {
        for (int j = 0; j < NUM_THREADS_PER_PROCESS; ++j) {
            sem_init(&sem_start[i][j], 0, 0);
            sem_init(&sem_done[i][j], 0, 0);

            int start_row = i * NUM_THREADS_PER_PROCESS * rows_per_thread + j * rows_per_thread;
            int end_row = start_row + rows_per_thread;

            thread_data[i][j].image_data = image_data;
            thread_data[i][j].width = width;
            thread_data[i][j].height = height;
            thread_data[i][j].num_components = num_components;
            thread_data[i][j].start_row = start_row;
            thread_data[i][j].end_row = end_row > height ? height : end_row;
            thread_data[i][j].sem_start = &sem_start[i][j];
            thread_data[i][j].sem_done = &sem_done[i][j];

            if (pthread_create(&threads[i][j], NULL, process_image, (void*) &thread_data[i][j]) != 0) {
                fprintf(stderr, "Erreur lors de la création du thread %d du processus %d.\n", j, i);
                return 1;
            }
        }
    }

    gettimeofday(&start_time, NULL);

    for (int i = 0; i < NUM_PROCESSES; ++i) {
        for (int j = 0; j < NUM_THREADS_PER_PROCESS; ++j) {
            sem_post(&sem_start[i][j]);
        }
    }

    for (int i = 0; i < NUM_PROCESSES; ++i) {
        for (int j = 0; j < NUM_THREADS_PER_PROCESS; ++j) {
            sem_wait(&sem_done[i][j]);
        }
    }

    gettimeofday(&end_time, NULL);
    execution_time = (double)(end_time.tv_sec - start_time.tv_sec) + (double)(end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    printf("Temps d'exécution parallèle (processus + threads optimisés) : %.6f secondes\n", execution_time);

    write_jpeg_file(output_filename, image_data, width, height, num_components);

    for (int i = 0; i < NUM_PROCESSES; ++i) {
        for (int j = 0; j < NUM_THREADS_PER_PROCESS; ++j) {
            sem_destroy(&sem_start[i][j]);
            sem_destroy(&sem_done[i][j]);
        }
    }

    free(image_data);

    return 0;
}
