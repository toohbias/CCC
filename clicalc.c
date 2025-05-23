#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#define SQRT 1716

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

void checkTokens(Token* ptr, int size) {
    for(int i = 0; i < size; i++) {
        if(ptr[i].isNum) {
            printf("%f", ptr[i].num);
        } else {
            printf("%c", ptr[i].op);
        }
    }
    printf("\n");
}

int getPrecedence(char op) {
    switch(op) {
        case '+': return 1;
        case '-': return 1;
        case '*': return 2;
        case '/': return 2;
        case '%': return 2;
        case '_': return 3; // unary minus
        case '^': return 4;
        case 'r': return 4;
        case '~': return 5; // unary minus, but directly after exponent to not mess up postfix
        case '!': return 6;
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

        if(c == '-') { //messed up unary minus
            if(resIndex == 0) { 
                c = '_';
            } else {
                char d = ' ';
                for(int j = i - 1; j >= 0; j--) {
                    if(infix[j] != ' ') {
                        d = infix[j];
                        break;
                    }
                }
                if(d == '^') { 
                    c = '~';
                } else if(d == '(' || (isdigit(d) == false && d != ')' && d != '!')) { 
                    c = '_'; 
                }
            }
        }

        if(c == '_') {
            if(resIndex != 0) { //omit *
                char d = ' ';
                for(int j = i - 1; j >= 0; j--) {
                    if(infix[j] != ' ') {
                        d = infix[j];
                        break;
                    }
                }
                if(isdigit(d) || d == ')' || d == '!') {
                    while(!isEmpty(&stack) && getPrecedence('*') <= getPrecedence(peek(&stack))) {
                        result[resIndex].op = pop(&stack);
                        result[resIndex].isNum = false;
                        resIndex++;
                    }
                    push(&stack, '*');
                }
            }
            int cmdId = 0;
            while(infix[++i] != '(' && i < size) { // weird way not to switch strings
                cmdId *= 2;
                cmdId += infix[i];
            }
            switch (cmdId) {
                case SQRT: 
                    while(!isEmpty(&stack) && getPrecedence('r') <= getPrecedence(peek(&stack))) {
                        result[resIndex].op = pop(&stack);
                        result[resIndex].isNum = false;
                        resIndex++;
                    }
                    push(&stack, 'r');
                    break;
                default:
                    printf("syntax error!\n");
            }
            i--;
            continue;
        }

        if(isdigit(c)) {
            char *end;
            result[resIndex].num = strtod(infix + i, &end);
            result[resIndex].isNum = true;
            resIndex++;
            i = (int)(end - infix) - 1;
            continue;
        }
        switch(c) { //basic operations
            case '(':
                if(resIndex != 0) { //omit *
                    char d = ' ';
                    for(int j = i - 1; j >= 0; j--) {
                        if(infix[j] != ' ') {
                            d = infix[j];
                            break;
                        }
                    }
                    if(isdigit(d) || d == ')' || d == '!') {
                        while(!isEmpty(&stack) && getPrecedence('*') <= getPrecedence(peek(&stack))) {
                            result[resIndex].op = pop(&stack);
                            result[resIndex].isNum = false;
                            resIndex++;
                        }
                        push(&stack, '*');
                    }
                }
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
    
    char op = token->op;
    if(op == '~' || op == '_' || op == '!' || op == 'r') {
        root->rightOp = malloc(sizeof(Operation));
        checkMalloc(root->rightOp);
        root->leftOp = malloc(1); //dummy malloc so I dont have to check unaries before freeing in calc
        return buildOperationTree(postfix, size - 1, root->rightOp);
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

    char op = root->value->op;
    if(op == '~' || op == '_') {
        return -calcTree(root->rightOp);
    }

    if(op == '!') {
        double result = 1;
        for(int i = (int) calcTree(root->rightOp); i > 0; i--) {
            result *= i;
        }
        return result;
    }

    if(op == 'r') {
        return sqrt(calcTree(root->rightOp));
    }

    double leftResult = calcTree(root->leftOp);
    double rightResult = calcTree(root->rightOp);

    switch(root->value->op) {
        case '+': return leftResult + rightResult;
        case '-': return leftResult - rightResult;
        case '*': return leftResult * rightResult;
        case '/': return leftResult / rightResult;
        case '%': return (int) leftResult % (int) rightResult;
        case '^': return pow(leftResult, rightResult);
        default: 
            printf("Something went wrong here...");
            exit(1);
    }
}

void calculate(char *input) {
    int size = strlen(input);
    Token tokens[size];
    int trueSize = in2postfix(input, tokens, size);
//   checkTokens(tokens, trueSize);
    
    Operation mainOp;
    buildOperationTree(tokens, trueSize, &mainOp);

    double result = calcTree(&mainOp);
    char op = mainOp.value->op;
    free(mainOp.leftOp);
    free(mainOp.rightOp);
    printf("Result: %f\n", result);

}

