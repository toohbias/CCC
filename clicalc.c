#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include "stack.c"

#define SQRT 1716
#define RT 344
#define ABS 699
#define SIN 780
#define COS 733
#define TAN 768
#define LN 326
#define LOG 757

#define PI 233
#define E 69

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

bool isUnary(char c) {
    switch (c) {
        case '~': return true;
        case '!': return true;
        case 'r': return true;
        case 'a': return true;
        case 's': return true;
        case 'c': return true;
        case 't': return true;
        case 'l': return true;
        default: return false;
    }
}

int getPrecedence(char op) {
    switch(op) {
        case '+': return 1;
        case '-': return 1;
        case '~': return 1;
        case '*': return 2;
        case '/': return 2;
        case '%': return 2;
        case '^': return 3;
        case '!': return 4;
        case 'r': return 5;
        case 'R': return 5;
        case 'a': return 5;
        case 's': return 5;
        case 'c': return 5;
        case 't': return 5;
        case 'l': return 5;
        case 'L': return 5;
        default: return -1;
    }
}

void omitMult(int *resIndex, char d, Stack *stack, Token *result) {
    if(*resIndex != 0) { 
        if(isdigit(d) || d == ')' || d == '!') {
            while(!isEmpty(stack) && getPrecedence('*') <= getPrecedence(peek(stack))) {
                result[*resIndex].op = pop(stack);
                result[*resIndex].isNum = false;
                *resIndex = *resIndex + 1;
            }
            push(stack, '*');
        }
    }
}

int in2postfix(char *infix, Token *result, int size) {
    Stack stack;
    init(&stack, size);

    int resIndex = 0;

    char d = '\0';
    for(int i = 0; i < size; i++) {
        char c = infix[i];

        if(c == ' ' || c == '\t') { continue; }

        if(c == '_') { //multi-char operations starting with _
            omitMult(&resIndex, d, &stack, result);
            int cmdId = 0;
            while(isalpha(infix[++i])) { // weird way not to switch strings
                cmdId *= 2;
                cmdId += infix[i];
            }
            switch (cmdId) {
                case SQRT: push(&stack, 'r'); break;
                case RT: {
                    char *end;
                    result[resIndex].num = strtod(infix + ++i, &end);
                    result[resIndex].isNum = true;
                    resIndex++;
                    i = (int)(end - infix);
                    push(&stack, 'R');
                    break;
                }
                case ABS: push(&stack, 'a'); break;
                case SIN: push(&stack, 's'); break;
                case COS: push(&stack, 'c'); break;
                case TAN: push(&stack, 't'); break;
                case LN: push(&stack, 'l'); break;
                case LOG: {
                    char *end;
                    result[resIndex].num = strtod(infix + ++i, &end);
                    result[resIndex].isNum = true;
                    resIndex++;
                    i = (int)(end - infix);
                    push(&stack, 'L');
                    break;
                }
                case PI: {
                    result[resIndex].num = M_PI;
                    result[resIndex].isNum = true;
                    resIndex++;
                    break;
                }
                case E: {
                    result[resIndex].num = M_E;
                    result[resIndex].isNum = true;
                    resIndex++;
                    break;
                }
                default:
                    printf("syntax error!\n");
            }
            i--;
            d = '_';
            continue;
        }

        if(c == '-') {
            if(resIndex == 0) { 
                c = '~';
            } else if(d == '(' || (isdigit(d) == false && d != ')' && d != '!')) { 
                c = '~'; 
            }
        }

        if(isdigit(c)) {
            char *end;
            result[resIndex].num = strtod(infix + i, &end);
            result[resIndex].isNum = true;
            resIndex++;
            i = (int)(end - infix) - 1;
            d = infix[i];
            continue;
        }

        switch(c) { //basic operations
            case '(':
                omitMult(&resIndex, d, &stack, result);
                push(&stack, c);
                break;
            case ')':
                while(!isEmpty(&stack) && peek(&stack) != '(') {
                    result[resIndex].op = pop(&stack);
                    result[resIndex].isNum = false;
                    resIndex++;
                }
                pop(&stack);
                while(!isEmpty(&stack) && getPrecedence(peek(&stack)) == 5) {
                    result[resIndex].op = pop(&stack);
                    result[resIndex].isNum = false;
                    resIndex++;
                }
                break;
            default: 
                if(!isUnary(c)) {
                    while(!isEmpty(&stack) && getPrecedence(c) <= getPrecedence(peek(&stack))) {
                        result[resIndex].op = pop(&stack);
                        result[resIndex].isNum = false;
                        resIndex++;
                    }              
                }                
                push(&stack, c); 
        }
        d = c;
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
    checkMalloc(root->rightOp);
    
    char op = token->op;
    if(isUnary(op)) {
        root->leftOp = malloc(1);
        return buildOperationTree(postfix, size - 1, root->rightOp);
    } else {
        root->leftOp = malloc(sizeof(Operation));
        checkMalloc(root->leftOp);
        int rightAfter = buildOperationTree(postfix, size - 1, root->rightOp);
        int leftAfter = buildOperationTree(postfix, rightAfter - 1, root->leftOp);
        return leftAfter;
    }
}

double calcTree(Operation *root) {
    if(root->value->isNum) {
        return root->value->num;
    }

    double rightResult = calcTree(root->rightOp);
    double leftResult;

    char op = root->value->op;
    if(!isUnary(op)) {
         leftResult = calcTree(root->leftOp);
    }

    switch(op) {
        case '+': return leftResult + rightResult;
        case '-': return leftResult - rightResult;
        case '~': return -rightResult;
        case '*': return leftResult * rightResult;
        case '/': return leftResult / rightResult;
        case '%': return (int) leftResult % (int) rightResult;
        case '^': return pow(leftResult, rightResult);
        case 'r': return sqrt(rightResult);
        case 'R': return pow(rightResult, (1 / leftResult));
        case '!': {
            double result = 1;
            for(int i = (int) calcTree(root->rightOp); i > 0; i--) {
                result *= i;
            }
            return result;
        }
        case 'a': return fabs(rightResult);
        case 's': return sin(rightResult);
        case 'c': return cos(rightResult);
        case 't': return tan(rightResult);
        case 'l': return log(rightResult);
        case 'L': return log2(rightResult) / log2(leftResult);
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
    free(mainOp.leftOp);
    free(mainOp.rightOp);
    printf("Result: %f\n", result);
}


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
