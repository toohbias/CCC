#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int size;
    char *arr;
    int top;
} Stack;

void init(Stack *stack, int size) {
    stack->size = size;
    stack->arr = malloc(size * sizeof(char));
    stack->top = -1;
}

bool isEmpty(Stack *stack) {
    return stack->top == -1;
}

bool isFull(Stack *stack) {
    return stack->top == stack->size - 1;
}

void push(Stack *stack, char value) {
    if(isFull(stack)) {
        printf("Stack Overflow\n");
        return;
    }
    stack->arr[++stack->top] = value;
}

char peek(Stack *stack) {
    if(isEmpty(stack)) {
        printf("Stack Underflow\n");
        return -1;
    }
    return stack->arr[stack->top];
}

char pop(Stack *stack) {
    char c = peek(stack); 
    stack->top--;
    return c;
}

void delete(Stack *stack) {
    free(stack->arr);
}
