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
int cached[MAX_MEMORY_SIZE] = {0};

// Result trackers
int instructions_executed = 0;
int ld_st_executed = 0;
int local_mem_hits = 0;
int clock_cycles = 0;

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
    instruction_offset = instruction_num[last_line_num] - last_line_num;
}

int getRegNum(char* reg) {
    // Since always 1 through 6, convert to 0 through 5
    int reg_num;
    sscanf(reg, "R%d", &reg_num);
    return reg_num - 1;
}

void mov(int reg_num, int value) {
    registers[reg_num] = value;
    clock_cycles += 1;
}

void addReg(int reg_num_n, int reg_num_m) {
    registers[reg_num_n] += registers[reg_num_m];
    clock_cycles += 1;
}

void addVal(int reg_num_n, int value) {
    registers[reg_num_n] += value;
    clock_cycles += 1;
}

int cmp(int reg_num_n, int reg_num_m) {
    clock_cycles += 1;
    if (registers[reg_num_n] == registers[reg_num_m]) {
        return 1;
    }
    else {
        return 0;
    }
}

void ld(int reg_num_n, int reg_num_m) {
    unsigned char memory_idx = registers[reg_num_m];
    if (cached[memory_idx] == 0) {
        cached[memory_idx] = 1;
        clock_cycles += 45;
    }
    else {
        clock_cycles += 2;
        local_mem_hits++;
    }
    registers[reg_num_n] = memory[memory_idx];
}

void st(int reg_num_n, int reg_num_m) {
    unsigned char memory_idx = registers[reg_num_n];
    if (cached[memory_idx] == 0) {
        cached[memory_idx] = 1;
        clock_cycles += 45;
    }
    else {
        clock_cycles += 2;
        local_mem_hits++;
    }
    memory[memory_idx] = registers[reg_num_m];
}

void executeInstructions() {
    /**
     * delim: Used for strtok; delimit string by tabs, commas, and spaces
     * line: Used as a counter/tracker for current line
     * conditional: Holds value from cmp (-1 for not in use, 0 for false, 1 for true) 
     */
    char* delim = "\t, ";
    int line = 0;
    int conditional = -1;

    while (line <= last_line_num) {
        char* instruction_copy = strdup(instruction_arr[line]);
        char* token = strtok(instruction_copy, delim);
        if (strcmp(token, "MOV") == 0) {
            // Get register number
            token = strtok(NULL, delim);
            int reg_num = getRegNum(token);

            // Get value
            token = strtok(NULL, delim);
            int value = atoi(token);

            // Execute
            mov(reg_num, value);
            // printf("Value %d moved into %d\n", registers[reg_num], reg_num);
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
            // printf("Value of register %d after add: %d\n", reg_num_n, registers[reg_num_n]);
            line++;
        }
        else if (strcmp(token, "LD") == 0) {
            // Get register number n
            token = strtok(NULL, delim);
            int reg_num_n = getRegNum(token);

            // Get register number m
            token = strtok(NULL, delim);
            char reg[3];
            reg[0] = token[1];
            reg[1] = token[2];
            reg[2] = '\0';
            int reg_num_m = getRegNum(reg);

            // Execute
            ld(reg_num_n, reg_num_m);
            // printf("Value at register %d: %d\n", reg_num_n, registers[reg_num_n]);
            line++;
            ld_st_executed++;
        }
        else if (strcmp(token, "ST") == 0) {
            // Get register number m
            token = strtok(NULL, delim);
            char reg[3];
            reg[0] = token[1];
            reg[1] = token[2];
            reg[2] = '\0';
            int reg_num_n = getRegNum(reg);

            // Get register number n
            token = strtok(NULL, delim);
            int reg_num_m = getRegNum(token);

            // Execute
            st(reg_num_n, reg_num_m);
            // printf("Value in memory: %d\n", memory[registers[reg_num_n]]);
            line++;
            ld_st_executed++;
        }

        else if (strcmp(token, "CMP") == 0) {
            // Get register number n
            token = strtok(NULL, delim);
            int reg_num_n = getRegNum(token);

            // Get register number m
            token = strtok(NULL, delim);
            int reg_num_m = getRegNum(token);
            conditional = cmp(reg_num_n, reg_num_m);
            // printf("Conditional result: %d\n", conditional);
            line++;
        }
        else if (strcmp(token, "JE") == 0) {
            // Get line number
            if (conditional == 1){
                token = strtok(NULL, delim);
                int line_to_jump = atoi(token);
                // printf("Instruction offset: %d\n", instruction_offset);
                line = line_to_jump - instruction_offset;
                // printf("Jump to line: %d\n", line);
            }
            else if (conditional == 0){
                line++;
            }
            conditional = -1;
            clock_cycles += 1;
        }
        else if (strcmp(token, "JMP") == 0) {
            // Get line number
            token = strtok(NULL, delim);
            int line_to_jump = atoi(token);
            line = line_to_jump - instruction_offset;
            clock_cycles += 1;
        }
        else {
            printf("Unknown instruction: %s\n", instruction_copy);
            free(instruction_copy);
            break;
        }
        free(instruction_copy);
        instructions_executed++;
    }
}

void printInstructions() {
    for (int i = 0; i < MAX_LINES; i++) {
        if (instruction_arr[i][0] != '\0') {
            printf("%s\n", instruction_arr[i]);
        }
    }
}

void printResults() {
    printf("Total number of executed instructions: %d\n", instructions_executed);
    printf("Total number of clock cycles: %d\n", clock_cycles);
    printf("Number of hits to local memory: %d\n", local_mem_hits);
    printf("Total number of executed LD/ST instructions: %d\n", ld_st_executed);
}
int main(int argc, char* argv[]) {
    FILE *assembly_code = fopen(argv[1], "r");
    if (assembly_code == NULL) {
        printf("Could not open the file.\n");
        return 1;
    }
    readInstructions(assembly_code);
    executeInstructions();
    printResults();
    fclose(assembly_code);
}