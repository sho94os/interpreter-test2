
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "datatypes.h"
#include "input.h"
#include "parser.h"

void* create_section(const int datatype, const char* string){
    struct Section* section = malloc(sizeof(struct Section));
    if (section == NULL)
        goto SECTION_MEMERROR;

    section->datatype = datatype;

    int len = strlen(string);
    section->string = NULL;
    section->string = malloc((len+1)*sizeof(char));
    if (section->string == NULL)
        goto SECTION_MEMERROR;
    strcpy(section->string, string);

    section->prev = NULL;
    section->next = NULL;
    return section;

    SECTION_MEMERROR:
    printf("ERROR: Couldn't allocate memory for a new section\n");
    exit(-1);
}

void free_section(struct Section* section){
    free(section->string);
    free(section);
}

void* parser(char* string){
    printf("# Parsing...\n");

    int len = strlen(string);

    struct Section* section = NULL;
    struct Section* last_section = NULL;

    int datatype = TYPE_NONE;
    int datatype_starti = 0;
    int datatype_endi = 0;
    int prev_datatype = TYPE_NONE;
    for (int i=0; i<=len; i++){
        datatype_endi = i-1;
        // Types with escape chars
        if (datatype == TYPE_STR){
            while ( string[i] != '"' && i < len)
                i++;
            if (string[i] != '"'){
                printf("No end \" for string! Exiting\n");
                exit(-1);
            }
            datatype = TYPE_NONE;
            datatype_starti++;
            datatype_endi = i-1;
        }
        else if (datatype == TYPE_FUNC){
            datatype = TYPE_NONE;
            while ( string[i] != ')' && i < len)
                i++;
            if (string[i] != ')'){
                printf("No end ) for function! Exiting\n");
                exit(-1);
            }
            datatype_endi = i;
        }
        else if (datatype == TYPE_PREPROCESS){
            datatype = TYPE_NONE;
            while ( string[i] != ')' && i < len)
                i++;
            if (string[i] != ')'){
                printf("No end ) for preprocessor! Exiting\n");
                exit(-1);
            }
            datatype_endi = i;
            printf("Preprocessing is not yet fully implemented, exiting\n");
            exit(-1);
        }
        // Alphanumerics
        // A-Z
        else if (string[i] >= 'A' && string[i] <= 'Z'){
            datatype = TYPE_VAR;
        }
        // a-z
        else if (string[i] >= 'a' && string[i] <= 'z'){
            datatype = TYPE_VAR;
        }
        // 0-1
        else if (string[i] >= '0' && string[i] <= '9'){
            if (prev_datatype != TYPE_VAR)
                datatype = TYPE_INT;
        }
        // Symbols
        else {
            switch (string[i]) {
                case ' ': // blank
                case '\0': // end of string
                case '\n': // newline
                    datatype = TYPE_NONE;
                    break;
                case '"':
                    // Start of string
                    if (datatype != TYPE_STR)
                        datatype = TYPE_STR;
                    // End of string
                    else
                        datatype = TYPE_NONE;
                    break;
                case '(':
                    // Function start
                    if (prev_datatype == TYPE_VAR){
                        prev_datatype = TYPE_FUNC;
                        datatype = TYPE_FUNC;
                    }
                    // Preprocess start
                    else {
                        prev_datatype = TYPE_PREPROCESS;
                        datatype = TYPE_PREPROCESS;
                    }
                    break;
                case ')': // Function/Preprocess end
                    datatype = TYPE_NONE;
                    break;
                case '=': // Assign
                    datatype = TYPE_ASSIGNMENT;
                    if (prev_datatype == datatype){
                        datatype = TYPE_COMPARISON;
                        prev_datatype = TYPE_COMPARISON;
                    }
                    break;
                case '+': // +
                    datatype = TYPE_PLUS;
                    break;
                case '-': // -
                    datatype = TYPE_MINUS;
                    break;
                case '*':
                    datatype = TYPE_MULTIPLICATION;
                    break;
                case '/': // /
                    datatype = TYPE_DIVISION;
                    break;
                case '%': // %
                    datatype = TYPE_MODULO;
                    break;
                default:
                    printf("Unknown token %c\n", string[i]);
                    break;
            }
        }
        if (datatype != prev_datatype){
            if (datatype_endi >= 0){
                if (prev_datatype != TYPE_NONE){
                    printf("%s ", DataTypeNames[prev_datatype]);

                    int len = datatype_endi-datatype_starti+2;
                    char* datastring = malloc(len*sizeof(char));
                    strncpy(datastring, string+datatype_starti, len-1);
                    datastring[len-1] = '\0';

                    printf("%s", datastring);
                    printf("(%d,%d,%d), ", datatype_starti, datatype_endi, len-1);

                    section = (struct Section*) create_section(prev_datatype, datastring);
                    free(datastring);
                    if (last_section == NULL)
                        last_section = section;
                    else {
                        last_section->next = section;
                        section->prev = last_section;
                        last_section = section;
                    }
                    printf("\n");
                }
            }
            datatype_starti = i;
        }
        prev_datatype = datatype;
    }
    return last_section;
}
