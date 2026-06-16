#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define A_INSTRUCTION 1
#define C_INSTRUCTION 2
#define L_INSTRUCTION 3
#define BLANK_INSTRUCTION 0

#define MAX_SYMBOLS 1024

typedef struct {
    char name[64];
    int address;
} Symbol;

Symbol symbolTable[MAX_SYMBOLS];

int symbolCount = 0;

int nextVariableRam = 16;

void error ();
void symbols();
void symbols();
void init_symbol_table();
void int_to_instruction(int address, char *binaryOut);
void add_symbol(const char *name, int address);
int get_symbol_address(const char *name);
void addEntry(char *symbol, int address);
int parse_line(const char *rawLine, char *outBuffer);
void parse_c_instruction(const char *line, char *dest, char *comp, char *jump);
const char *dest_to_binary(const char *dest);
const char *jump_to_binary(const char *jump);
const char *comp_to_binary(const char *comp);

int main (int argc, char *argv[])   {
    FILE *fp, *fp2; 
    char s[256], d[256];
    char *saveptr1, saveptr2;
    char *subtoken;
    int linecount = 0;
    int j = 0;
    if (argc != 2) {
        fprintf(stderr, "Usage : %s filename.asm\n", argv[0]);
        return 1;
    }
    if ((fp = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Error: failed to open file %s \n", argv[1]);
        return 1;
    }

    char outputFileName[256];
    strncpy(outputFileName, argv[1], sizeof(outputFileName) -1);
    outputFileName[sizeof(outputFileName) -1] = '\0';

    char *dot = strchr(outputFileName, '.');
    if (dot != NULL && strcmp(dot, ".asm") == 0) {
        *dot = '\0';
    }
    strcat(outputFileName, ".hack");
    
    if ((fp2 = fopen(outputFileName, "w")) == NULL) {
        fprintf(stderr, "Error failed to open file : %s", outputFileName);
        return 1;
    }

    init_symbol_table();

    int romAddress = 0;

    while (fgets(s, sizeof s, fp) != NULL) {
        linecount++; 

        int type = parse_line(s, d);

        if (type == A_INSTRUCTION || type == C_INSTRUCTION) {
            romAddress++;
        }
        else if (type == L_INSTRUCTION) {

            add_symbol(d, romAddress);
        }
    }

    rewind(fp);

    while(fgets(s, sizeof s, fp) != NULL) {
        int type = parse_line(s, d);

        if (type == A_INSTRUCTION) {
            int finalAddress = 0;

            if (isdigit((unsigned char)d[0])) {
                finalAddress = atoi(d);
            } 
            else {
                finalAddress = get_symbol_address(d);

                if (finalAddress == -1) {
                    add_symbol(d, nextVariableRam);
                    finalAddress = nextVariableRam;
                    nextVariableRam++;
                }

            }
            char binaryOut[17];
            int_to_instruction(finalAddress, binaryOut);
            fprintf(fp2, "%s\n", binaryOut);
        }
        else if (type == C_INSTRUCTION) {
            char comp[16];
            char dest[8];
            char jump[8];

            char binaryOut[17];

            parse_c_instruction(d, dest, comp, jump);
            
            sprintf(binaryOut, "111%s%s%s", comp_to_binary(comp), dest_to_binary(dest), jump_to_binary(jump));
            fprintf(fp2, "%s\n", binaryOut);
        }
    }

    fclose(fp);
    fclose(fp2);
    printf("Assembler successful, generated binary output : %s\n", outputFileName);
    return 0;
}
// bool hasMoreLines() {
    
//     return 
// }

int parse_line (const char *rawLine, char *outBuffer) {

    outBuffer[0] = '\0';
    char lineCopy[256];
    strncpy(lineCopy, rawLine, sizeof(lineCopy) -1);
    lineCopy[sizeof(lineCopy)-1] = '\0';
    
    char *cptr1 = lineCopy;
    while (*cptr1 != '\0') {
        if (*cptr1 == '/' && *(cptr1 +1 ) == '/' ) {
            *cptr1 = '\0';
            break;
        }
        cptr1++;
    }

    char *cptr2 = lineCopy;
    while (*cptr2 != '\0' && isspace((unsigned char)*cptr2)) {
        cptr2++;
    }
    if (*cptr2 == '\0' ) {
        return BLANK_INSTRUCTION;
    }

    if (*cptr2 == '@') {
        char *saveptr;
        char *rawValue = cptr2 + 1; 
        char *cleanValue = strtok_r(rawValue, " \t\r\n", &saveptr);
        if (cleanValue != NULL) {
            strcpy(outBuffer, cleanValue);
            return A_INSTRUCTION;
        }
    }
    else if (*cptr2 == '(') {
        char *closing = strchr(cptr2, ')');
        if (closing != NULL) {
            *closing = '\0';
            strcpy(outBuffer, cptr2 + 1);
            return L_INSTRUCTION;
        }
        // char *rawValue = cptr2 + 1; 
        // char *cleanValue = strtok_r(rawValue, " \t\n", &saveptr);
        // if (cleanValue != NULL) {
        //     strcpy(outBuffer, cleanValue);
        //     return L_INSTRUCTION;
        // }
    }
    else {
        char *saveptr;
        char *cleanValue = strtok_r(cptr2, " \t\r\n", &saveptr);

        if (cleanValue != NULL) {
            strcpy(outBuffer, cleanValue);
            return C_INSTRUCTION;
        }
    }
    return BLANK_INSTRUCTION;
}

void add_symbol(const char *name, int address) {
    strcpy(symbolTable[symbolCount].name, name);
    symbolTable[symbolCount].address = address; 
    symbolCount++;
}

void init_symbol_table () {
    add_symbol("SP", 0);
    add_symbol("LCL", 1);
    add_symbol("ARG", 2);
    add_symbol("THIS", 3);
    add_symbol("THAT", 4);

    char regName[5];
    for(int i=0; i < 16; i++) {
        sprintf(regName, "R%d", i);
        add_symbol(regName, i);
    }
    add_symbol("SCREEN", 16384);
    add_symbol("KBD", 24576);
}

int get_symbol_address(const char *name) {

    for (int i = 0; i < symbolCount ; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return symbolTable[i].address;
        }   
    }

    return -1;
}

void int_to_instruction (int address, char *binaryOut) {
    int totalBits = 16;
    binaryOut[totalBits] = '\0';
    for (int i = totalBits - 1 ; i >=0 ; i--) {
        binaryOut[i] = (address & 1) ? '1' : '0';
        address >>= 1;
    }
}

void parse_c_instruction(const char *cIns, char *dest, char *comp, char *jump) {
    dest[0] = '\0';
    comp[0] = '\0';
    jump[0] = '\0';

    char c[32] = "";
    strcpy(c, cIns); 

    char *equalp = strchr(c, '=');
    char *colonp = strchr(c, ';');

    char *compStart = c;

    if (equalp != NULL) {
        *equalp = '\0';
        strcpy(dest, c);
        compStart = equalp + 1;
    }

    if (colonp != NULL) {
        *colonp = '\0';
        strcpy(comp, compStart);
        strcpy(jump, colonp + 1);
    }
    else {
        strcpy(comp, compStart);
    }

}

const char *comp_to_binary(const char *comp) {
    if (strcmp(comp, "") ==0) return "0000000";
    if (strcmp(comp, "0")==0) return "0101010";
    if (strcmp(comp, "1") ==0) return "0111111";
    if (strcmp(comp, "-1") ==0) return "0111010";
    if (strcmp(comp, "D") ==0) return "0001100";
    if (strcmp(comp, "A") ==0) return "0110000";
    if (strcmp(comp, "M") ==0) return "1110000";
    if (strcmp(comp, "!D") ==0) return "0001101";
    if (strcmp(comp, "!A") ==0) return "0110001";
    if (strcmp(comp, "!M") ==0) return "1110001";
    if (strcmp(comp, "-D") ==0) return "0001111";
    if (strcmp(comp, "-A")==0) return "0110011";
    if (strcmp(comp, "-M")==0) return "1110011";
    if (strcmp(comp, "D+1")==0) return "0011111";
    if (strcmp(comp, "A+1")==0) return "0110111";
    if (strcmp(comp, "M+1")==0) return "1110111";
    if (strcmp(comp, "D-1")==0) return "0001110";
    if (strcmp(comp, "A-1")==0) return "0110010";
    if (strcmp(comp, "M-1")==0) return "1110010";
    if (strcmp(comp, "D+A")==0 || strcmp(comp, "A+D")==0) return "0000010";
    if (strcmp(comp, "D+M")==0 || strcmp(comp, "M+D")==0) return "1000010";
    if (strcmp(comp, "D-A")==0) return "0010011";
    if (strcmp(comp, "D-M")==0) return "1010011";
    if (strcmp(comp, "A-D")==0) return "0000111";
    if (strcmp(comp, "M-D")==0) return "1000111";
    if (strcmp(comp, "D&A")==0 || strcmp(comp, "D&A")==0) return "0000000";
    if (strcmp(comp, "D&M")==0 || strcmp(comp, "M&D")==0) return "1000000";
    if (strcmp(comp, "D|A")==0 || strcmp(comp, "A|D")==0) return "0010101";
    if (strcmp(comp, "D|M")==0 || strcmp(comp, "M|D")==0) return "1010101"; 

    return "0000000";
}

const char *dest_to_binary(const char *dest) {
    if (strcmp(dest, "") == 0) return "000"; 
    if (strcmp(dest, "M") == 0) return "001"; 
    if (strcmp(dest, "D") == 0) return "010"; 
    if (strcmp(dest, "MD") == 0 || strcmp(dest, "DM") == 0) return "011"; 
    if (strcmp(dest, "A") == 0) return "100"; 
    if (strcmp(dest, "AM") == 0) return "101"; 
    if (strcmp(dest, "AD") == 0) return "110"; 
    if (strcmp(dest, "ADM") == 0) return "111";
    
    return "000";
}

const char *jump_to_binary(const char *jump) {
    if (strcmp(jump, "")==0 ) return "000";
    if (strcmp(jump, "JGT")==0 ) return "001";
    if (strcmp(jump, "JEQ")==0 ) return "010";
    if (strcmp(jump, "JGE")==0 ) return "011";
    if (strcmp(jump, "JLT")==0 ) return "100";
    if (strcmp(jump, "JNE")==0 ) return "101";
    if (strcmp(jump, "JLE")==0 ) return "110";
    if (strcmp(jump, "JMP")==0 ) return "111";

    return "000";
}
