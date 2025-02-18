#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 100
#define MAX_INSTR_LEN 15
#define MAX_REGISTERS 6
#define MAX_MEMORY_SIZE 256

// Set up registers, memory, instruction array
signed char registers[MAX_REGISTERS] = {0};
unsigned char memory[MAX_MEMORY_SIZE] = {0};
char instruction_arr[MAX_LINES][MAX_INSTR_LEN];
int instruction_num[MAX_LINES];
int instruction_offset;
int last_line_num;


void readInstructions(FILE *file) {
    int line = 0;
    int instruction_number;
    
    // Read instructions and instruction nums into their own arrays
    while (fscanf(file, "%d %[^\n]", &instruction_number, instruction_arr[line]) == 2) {
        instruction_num[line] = instruction_number;
        line++;
    }

    // Calculate instruction offset
    // Ex: We are fed instruction at line 10, but it is instruction 0 for us.
    // Should only really need this for jmp and je
    last_line_num = line - 1;
    instruction_offset = instruction_number - instruction_num[last_line_num];
}

int getRegNum(char* reg) {
    // Since always 1 through 6, convert to 0 through 5
    int reg_num;
    sscanf(reg, "R%d", &reg_num);
    return reg_num - 1;
}

void mov(int reg_num, int value) {
    registers[reg_num] = value;
}

void addReg(int reg_num_n, int reg_num_m) {
    registers[reg_num_n] += registers[reg_num_m];
}

void addVal(int reg_num_n, int value) {
    registers[reg_num_n] += value;
}

int cmp(int reg_num_n, int reg_num_m) {
    if (registers[reg_num_n] == registers[reg_num_m]){
        return 0;
    }
    else {
        return 1;
    }
}

void executeInstructions() {
    char* delim = ", ";
    int line = 0;
    while (line <= last_line_num) {
        char *token = strtok(instruction_arr[line], delim);
        if (strcmp(token, "MOV") == 0) {
            // Get register number
            token = strtok(NULL, delim);
            int reg_num = getRegNum(token);

            // Get value
            token = strtok(NULL, delim);
            int value = atoi(token);

            // Execute
            mov(reg_num, value);
            printf("Value %d moved into %d\n", registers[reg_num], reg_num);
            line++;
        }
    
        else if (strcmp(token, "ADD") == 0) {
            // Get register number
            token = strtok(NULL, delim);
            int reg_num_n = getRegNum(token);
            
            // Decide whether last input is another register number or a value
            token = strtok(NULL, delim);
            if (token[0] == 'R') {
                int reg_num_m = getRegNum(token);
                addReg(reg_num_n, reg_num_m);
            }
            else {
                int value = atoi(token);
                addVal(reg_num_n, value);
            }
            printf("Value of register %d after add: %d\n", reg_num_n, registers[reg_num_n]);
            line++;
        }
        else if (strcmp(token, "CMP") == 0) {
            // Get register number n
            token = strtok(NULL, delim);
            int reg_num_n = getRegNum(token);

            // Get register number m
            token = strtok(NULL, delim);
            int reg_num_m = getRegNum(token);

            int result = cmp(reg_num_n, reg_num_m);
        }
        else {
            printf("Not implemented\n");
        }
    }
}

void printInstructions() {
    for (int i = 0; i < MAX_LINES; i++) {
        if (instruction_arr[i][0] != '\0') {
            printf("%s\n", instruction_arr[i]);
        }
    }
}


int main(int argc, char* argv[]) {
    FILE *assembly_code = fopen(argv[1], "r");
    if (assembly_code == NULL) {
        printf("Could not open the file.\n");
        return 1;
    }
    readInstructions(assembly_code);
    executeInstructions();
    // printInstructions();
    fclose(assembly_code);
}