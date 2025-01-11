#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


extern int yyparse(void);
extern void yy_scan_string(const char* str);
extern FILE* yyin;
extern void initializeGlobalScope(void);
extern void printVariables(void);
extern void clearAllVariables(void);

#define MAX_INPUT_SIZE 1024
#define PROMPT "> "


extern void yyrestart(FILE *input_file);


void run_script(const char* filename, int repeat_count) {
        FILE* file = fopen(filename, "r");
        if (!file) {
            printf("Error: Could not open file '%s'\n", filename);
            return;
        }
               
       // clearAllVariables();
        initializeGlobalScope();
        yyrestart(file);
        yyin = file;
        yyparse();
       // printVariables();
        fclose(file);
        clearAllVariables();
}


bool process_line(const char* input) {
    if (strlen(input) == 0) {
        return true;
    }

    if (strncmp(input, "exit", 4) == 0 || strncmp(input, "quit", 4) == 0) {
        return false;
    }
   
    if (strcmp(input, "clear") == 0) {
        clearAllVariables();
        initializeGlobalScope();
        printf("Memoria a fost curatata!\n");
        return true;
    }

   
    if (strncmp(input, "run ", 4) == 0) {
        char filename[MAX_INPUT_SIZE];
        int repeat = 1;
        
        char* token = strtok((char*)input + 4, " ");
        if (token == NULL) {
            printf("Error: Missing filename\n");
            return true;
        }
        strcpy(filename, token);
        
        token = strtok(NULL, " ");
        if (token != NULL && strcmp(token, "repeat") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                repeat = atoi(token);
                if (repeat <= 0) {
                    printf("Error: Invalid repeat count\n");
                    return true;
                }
            }
        }
        
        run_script(filename, repeat);
        return true;
    }

    yy_scan_string(input);
    yyparse();
    return true;
}


char* read_line(void) {
    static char buffer[MAX_INPUT_SIZE];
    printf(PROMPT);
    fflush(stdout);
    
    if (fgets(buffer, MAX_INPUT_SIZE, stdin) == NULL) {
        return NULL;
    }
    
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
    }
    
    return buffer;
}


void run_interactive_mode(void) {
    while (1) {
        char* input = read_line();
        if (input == NULL) {
            break;
        }
        
        if (!process_line(input)) {
            break;
        }
    }
}

int main(int argc, char** argv) {
    initializeGlobalScope();

    if (argc > 1) {
        run_script(argv[1], 1);
    } else {
        run_interactive_mode();
    }

    return 0;
}
int yyerror(const char* msg) {
     printf("Eroare: %s\n", msg);
     exit(1);
}
