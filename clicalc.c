#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include "stack.c"
#include "queue.c"
#include "plot.c"

#define MAX_LENGTH 50

#define EXIT 1614
#define X 120
#define PRECISION 56248
#define P 112
#define RESET 3410
#define HIST 1598
#define MODE 1617
#define M 109
#define HELP 1564
#define H 104

#define SIMPLE 6997
#define S 115
#define FUNCTION 27328
#define F 102

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
    bool isX;
    union {
        double *x;
        double num;
        char op;
    };
} Token;

typedef struct Operation {
    Token *value;
    struct Operation *leftOp;
    struct Operation *rightOp;
} Operation;

typedef struct Command {
    int value;
    struct Command *next;
} Command;

double global_x = 1; // has to be global, for the pointers

void checkMalloc(void *ptr) {
    if(!ptr) {
        printf("MEMORY ALLOCATION FAILED!\n");
        exit(1);
    }
}

void checkTokens(Token* ptr, int size) {
    for(int i = 0; i < size; i++) {
        if(ptr[i].isNum) {
            if(!ptr[i].isX) {
                printf("%.3f", ptr[i].num);
            } else {
                printf("x");
            }
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
        case '+': return 1; // plus
        case '-': return 1; // minus
        case '~': return 1; // unary minus
        case '*': return 2; // multiplied by
        case '/': return 2; // divided by
        case '%': return 2; // modulo
        case '^': return 3; // exponent
        case '!': return 4; // factorial
        case 'r': return 5; // square root
        case 'R': return 5; // root
        case 'a': return 5; // absolute value
        case 's': return 5; // sine
        case 'c': return 5; // cosine
        case 't': return 5; // tangent
        case 'l': return 5; // natural logarithm
        case 'L': return 5; // logarithm
        default: return -1;
    }
}

void omitMult(int *resIndex, char d, Stack *stack, Token *result) {
    if(*resIndex != 0) { 
        if(isdigit(d) || d == ')' || d == '!' || d == 'x' || d == '.') {
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
    initStack(&stack, size);

    int resIndex = 0;

    char d = '\0';
    for(int i = 0; i < size; i++) {
        char c = infix[i];

        if(c == ' ' || c == '\t') { continue; }

        if(c == 'x') {
            omitMult(&resIndex, d, &stack, result);
            result[resIndex].x = &global_x;
            result[resIndex].isX = true;
            result[resIndex].isNum = true;
            resIndex++;
            d = 'x';
            continue;
        }

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
        root->rightOp = malloc(1); // make free() not throw segfault
        root->leftOp = malloc(1);
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
        if(!root->value->isX) {
            return root->value->num;
        } else {
            return *root->value->x;
        }
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
            printf("Something went wrong here...\n");
            exit(1);
    }
}

void tokenizeCmd(Command *cmd, char *input) {
    char c = input[0];
    if(isdigit(c)) {
        char *end;
        cmd->value = (int) strtod(input, &end);
        if(*end != '\0') {
            cmd->next = malloc(sizeof(Command));
            checkMalloc(cmd->next);
            tokenizeCmd(cmd->next, end);
        }
    } else {
        int i = -1;
        int cmdId = 0;
        while(isalpha(input[++i])) { // weird way not to switch strings
            c = input[i];
            cmdId *= 2;
            cmdId += c;
        }
        cmd->value = cmdId;
        if(c != '\0') {
            cmd->next = malloc(sizeof(Command));
            checkMalloc(cmd->next);
            tokenizeCmd(cmd->next, &input[i] + 1);
        }
    }
}


int main(void) {
    char input[MAX_LENGTH];

    Queue history;
    initQueue(&history, 5, MAX_LENGTH);
    Queue results;
    initQueue(&results, 5, 25); // how long can doubles get?

    unsigned current_mode = SIMPLE;

    int precision = 3;

    while(1) {
        if(current_mode == FUNCTION) {
            printf("f(x) = ");
        }
        fgets(input, MAX_LENGTH, stdin);
        if(input[0] == '\n') { continue; }

        int len = strlen(input);
        input[len - 1] = '\0';

        if(input[0] == ':') {
            Command cmd;
            tokenizeCmd(&cmd, input + 1);
            switch (cmd.value) {
                // exit 
                case X:
                case EXIT: return 0;
                // set decimal point precision
                case P: 
                case PRECISION: {
                    int cmdId = cmd.next->value;
                    if(cmdId == RESET) {
                        precision = 3;
                    } else {
                        precision = cmdId;
                    }
                    continue;
                }
                // see last 5 searches
                case HIST: {
                    if(history.size != 0) {
                        for(int i = 0; i < history.size; i++) {
                            printf("%s= %s\n", 
                                   results.arr[(results.front + i) % results.capacity],
                                   history.arr[(history.front + i) % history.capacity]);
                        }
                    }
                    continue;
                }
                // select mode (more to be implemented)
                case M: 
                case MODE: {
                    int cmdId = cmd.next->value;
                    switch (cmdId) {
                        case S:
                        case SIMPLE: current_mode = SIMPLE; continue;
                        case F:
                        case FUNCTION: current_mode = FUNCTION; continue;
                        default: printf("mode doesn't exist!\n"); continue;
                    }
                } 
                case H:
                case HELP: {
                    printf("available commands:\n");
                    printf("  :x, :exit       exit the program\n");
                    printf("  :p, :precision  set the decimal precision of results\n");
                    printf("      :hist       view the last 5 calculations\n");
                    printf("  :m, :mode       change the calculation mode\n\n");
                    printf("available modes:\n");
                    printf("  s,  simple      simple evaluation of expressions\n");
                    printf("  f,  function    solving a function for x\n\n");
                    continue;
                }
                default: printf("command doesn't exist!\n"); continue;
            }
        }

        switch (current_mode) {
            case SIMPLE: {
                // tokenize input
                Token tokens[len + 5]; // buffer for e.g. omitted mult
                int trueSize = in2postfix(input, tokens, len - 1);

                // create syntax tree
                Operation mainOp;
                buildOperationTree(tokens, trueSize, &mainOp);

                // evaluate tree
                double result = calcTree(&mainOp);
                free(mainOp.leftOp);
                free(mainOp.rightOp);

                // history stuff
                char *resultStr = malloc(25 * sizeof(char));
                checkMalloc(resultStr);
                sprintf(resultStr, "%.*f\t", precision, result);
                enqueue(&history, input);
                enqueue(&results, resultStr);

                printf("Result: %s\n", resultStr);
                break;
            }
            case FUNCTION: {
                // tokenize input
                Token tokens[len + 5]; // buffer for e.g. omitted mult
                int trueSize = in2postfix(input, tokens, len - 1);

                // create syntax tree
                Operation mainOp;
                buildOperationTree(tokens, trueSize, &mainOp);

                // ask user for domain and increment of x
                Table t;
                char info[10];

                printf("min x? ");
                fgets(info, 10, stdin);
                t.min_x = strtod(info, NULL);

                printf("max x? ");
                fgets(info, 10, stdin);
                t.max_x = strtod(info, NULL);

                printf("step? ");
                fgets(info, 10, stdin);
                t.step = strtod(info, NULL);

                // make a table
                t.size_x = round((t.max_x - t.min_x) / t.step + 1);
                t.values = malloc(t.size_x * sizeof(double));
                checkMalloc(t.values);

                printf("\n x\t│ y\n────────┼────────\n");
                int a = 0;
                for(double i = t.min_x; i <= t.max_x; i += t.step) {
                    global_x = i;
                    double y = calcTree(&mainOp);
                    t.values[a] = y;
                    printf(" %.*f\t│ %.*f\n", precision, i, precision, y);
                    a++;
                    if(isnan(y) || isinf(y)) { continue; }
                    if(a == 1) {
                        t.min_y = y;
                        t.max_y = y;
                    } else {
                        t.min_y = y < t.min_y ? y : t.min_y;
                        t.max_y = y > t.max_y ? y : t.max_y;
                    }
                }
                printf("\n");
                free(mainOp.leftOp);
                free(mainOp.rightOp);

                // ask for plot
                printf("plot? (y/n) ");
                fgets(info, 10, stdin);
                if(info[0] == 'y') {
                    plot(&t);
                }

                // history stuff
                char *resultStr = malloc(25 * sizeof(char));
                checkMalloc(resultStr);
                sprintf(resultStr, "%.3f < x < %.3f  x+=%.3f  f(x) ", t.min_x, t.max_x, t.step);
                enqueue(&history, input);
                enqueue(&results, resultStr);

                break;
            }
            default: printf("Something has gone horribly wrong!\n");
        }
    }
    return 0;
}
