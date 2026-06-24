#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define BLANK_INSTRUCTION 0
#define C_ARITHMETIC 1
#define C_PUSH 2
#define C_POP 3
#define C_LABEL 4
#define C_GOTO 5
#define C_IF 6
#define C_FUNCTION 7
#define C_RETURN 8
#define C_CALL 9

char currentLineCopy[256];
char *tokenNavigator;


int commandType(const char *inBuffer);

int main() {
    FILE *fp1;
    char s[256],d[256];

    if ((fp1 = fopen("asmtest.vm", "r")) == NULL) {
        fprintf(stderr, "Usage : filename.asm\n");
        return 1;
    }
    while(fgets(s, sizeof s, fp1) != NULL) {
        int type = commandType(s);
        printf("this line is a : %d \n", type);
        if (type == C_PUSH || type == C_POP) {
            arg1(d);
        }
    }


    return 0;
}

int commandType (const char *inBuffer) {

    char lineCopy[256];
    strncpy(lineCopy, inBuffer, sizeof(lineCopy) -1);
    lineCopy[sizeof(lineCopy) - 1] = '\0';

    char *cptr1 = lineCopy;

    while (*cptr1 != '\0' ) {
        if (*cptr1 == '/' && *(cptr1 + 1) == '/') {
            cptr1 = '\0';
            break;
        }
        cptr1++;
    }
    char *cptr2 = lineCopy;

    while (*cptr2 != '\0' && isspace((unsigned char)*cptr2)) {
        cptr2++;
    }

    if (*cptr2 == '\0') {
        return BLANK_INSTRUCTION;
    }

    strncpy(currentLineCopy, lineCopy, sizezof(lineCopy) -1);
    currentLineCopy[sifeof(currentLineCopy) - 1] = '\0';

    char *firstWord = strtok_r(currentLineCopy, " \t\r\n", &tokenNavigator);
    if (firstWord != NULL) {
        if (strcmp(firstWord, "push") == 0)  {
            return C_PUSH;
        }
        else if (strcmp(firstWord, "pop") == 0 ) {
            return C_POP;
        }
        else if (strcmp(firstWord, "add") == 0 || strcmp(firstWord, "sub") == 0) {
            return C_ARITHMETIC;
        }

    }
}
void *arg1 (char *outBuffer) {
    char *segment = strtok_r(NULL, " \t\r\n", &tokenNavigator);
    if (segment != NULL) {
        strcpy(outBuffer, segment);
    }
    else {
        outBuffer[0] = '\0';
    }
}