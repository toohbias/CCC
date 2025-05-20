#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "stack.c"

typedef struct { 
    bool isNum;
    union {
        double num;
        char op;
    };
} Token;

typedef struct Operation {
    Token *value;
    struct Operation *leftOp;
    struct Operation *rightOp;
} Operation;

void calculate(char *input);


int main(void) {
    char input[50];

    bool running = true;
    while(running) {
        fgets(input, 50, stdin);
        if(input[0] == '\n') { continue; }
        input[strlen(input) - 1] = '\0';
        calculate(input);
    }
    return 0;
}


void checkMalloc(void *ptr) {
    if(!ptr) {
        printf("MEMORY ALLOCATION FAILED!");
        exit(1);
    }
}

int getPrecedence(char op) {
    switch(op) {
        case '+': return 1;
        case '-': return 1;
        case '*': return 2;
        case '/': return 2;
        default: return -1;
    }
}

int in2postfix(char *infix, Token *result, int size) {
    Stack stack;
    init(&stack, size);

    int resIndex = 0;

    for(int i = 0; i < size; i++) {
        char c = infix[i];

        if(c == ' ' || c == '\t') { continue; }

        if(isdigit(c) || c == '.') {
            char *end;
            result[resIndex].num = strtod(infix + i, &end);
            result[resIndex].isNum = true;
            resIndex++;
            i = (int)(end - infix) - 1;
            continue;
        }
         switch(c) {
             case '(':
                 push(&stack, c);
                 break;
             case ')':
                 while(!isEmpty(&stack) && peek(&stack) != '(') {
                     result[resIndex].op = pop(&stack);
                     result[resIndex].isNum = false;
                     resIndex++;
                 }
                 pop(&stack);
                 break;
             default: 
                 while(!isEmpty(&stack) && getPrecedence(c) <= getPrecedence(peek(&stack))) {
                     result[resIndex].op = pop(&stack);
                     result[resIndex].isNum = false;
                     resIndex++;
                 }
                 push(&stack, c);
         }
    }

    while (!isEmpty(&stack)) {
        result[resIndex].op = pop(&stack);
        result[resIndex].isNum = false;
        resIndex++;
    }

    delete(&stack);
    return resIndex;
}

int buildOperationTree(Token *postfix, int size, Operation *root) {
    if(size == 0) { return size; }
    Token *token = &postfix[size - 1];

    root->value = token;
    if(token->isNum) { 
        return size; 
    }
    
    root->rightOp = malloc(sizeof(Operation));
    root->leftOp = malloc(sizeof(Operation));
    checkMalloc(root->rightOp);
    checkMalloc(root->leftOp);

    int rightAfter = buildOperationTree(postfix, size - 1, root->rightOp);
    int leftAfter = buildOperationTree(postfix, rightAfter - 1, root->leftOp);

    return leftAfter;
}

double calcTree(Operation *root) {
    if(root->value->isNum) {
        return root->value->num;
    }

    double leftResult = calcTree(root->leftOp);
    double rightResult = calcTree(root->rightOp);
//   free(root->leftOp);
//   free(root->rightOp);

    switch(root->value->op) {
        case '+': return leftResult + rightResult;
        case '-': return leftResult - rightResult;
        case '*': return leftResult * rightResult;
        case '/': return leftResult / rightResult;
        default: 
            printf("Something went wrong here...");
            exit(1);
    }
}

void calculate(char *input) {
    int size = strlen(input);
    Token tokens[size];
    int trueSize = in2postfix(input, tokens, size);
    
    Operation mainOp;
    buildOperationTree(tokens, trueSize, &mainOp);

    double result = calcTree(&mainOp);
    free(mainOp.leftOp);
    free(mainOp.rightOp);
    printf("Result: %f\n", result);

}

