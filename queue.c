#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    int capacity;
    char **arr;
    int front;
    int size;
} Queue;

void initQueue(Queue *queue, int size, int max_size) {
    queue->capacity = size;
    queue->arr = malloc(size * sizeof(char *));
    for(int i = 0; i < size; i++) {
        queue->arr[i] = malloc(max_size * sizeof(char));
    }
    queue->front = 0;
    queue->size = 0;
}

void dequeue(Queue *queue) {
    if(queue->size == 0) { return; }
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
}

void enqueue(Queue *queue, char *value) {
    if(queue->size == queue->capacity) {
        dequeue(queue);
    }
    strcpy(queue->arr[(queue->front + queue->size) % queue->capacity], value);
    queue->size++;
}

void printQueue(Queue *queue) {
    if(queue->size != 0) {
        for(int i = 0; i < queue->size; i++) {
            printf("%s\n", queue->arr[(queue->front + i) % queue->capacity]);
        }
    }
}
