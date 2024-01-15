#include "model.h"
#include "interface.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

// Linked list for dependant cells
struct dependencies {
    char *depName;
    struct dependencies* nextNode;
};


// Cell Structure
struct cell {
    int col;
    int inUse;
    struct cellData { 
        float number;
        char* equation;
        char* string;
    } value;
    struct dependencies dependencies; // Cell dependencies for update task 
};


// Linked list for cells 
struct cellNode {
    struct cell data;
    struct cellNode* nextCol;
};


struct cellNode *array_of_heads[NUM_ROWS]; // Define an array where each block serves as the head of the linked list representing a row 


int isIntValid(const char *str) {
    const char *characters = "0123456789";
    return strspn(str, characters) == strlen(str); // Check if str has valid characters 
}


bool isString(char *text){ // Returns false if its a number, true if its a string 
    int decCounter = 0;
    int i = 0;

    if (strcmp(text,"")==0){
        return true;
    }if (text[0] == '-'){
        i = 1;
    }

    for (; i < strlen(text); i++) {
        if (text[i]=='.'){
            decCounter++;
        }
        if ((!(text[i] >= '0' && text[i] <= '9') && text[i] !='.') || decCounter > 1){
            return true;
        }
    }
    return false;
}


char* customStrtok(char* str, const char* dividers) {
    static char* nextBlock;
    if (str) {
        nextBlock = str;
    }if (!nextBlock) {
        return NULL;
    }
    char* block = nextBlock;
    for (; *nextBlock; ++nextBlock) {
        if (strchr(dividers, *nextBlock)) {
            if (nextBlock != block || *nextBlock != '-' || (*(nextBlock - 1) >= '0' && *(nextBlock - 1) <= '9' && *(nextBlock + 1) != '-')) {
                break;
            }
        }
    }
    if (*nextBlock) {
        *nextBlock++ = '\0';
    } else {
        nextBlock = NULL;
    }
    return block;
}



void string_divider(const char *input, const char *divider, char ***components, char **mathFunc, int *componentCount) {
    *components = NULL;
    *mathFunc = NULL;
    *componentCount= 0;

    // Make a copy of the input string since strtok modifies the string
    char *copy = strdup(input);

    // Use strtok to blockize the input string
    char *block = customStrtok(copy, divider);
    while (block != NULL) {
        // Increase the size of the components and mathFunc arrays
        *components = (char **)realloc(*components, (*componentCount+ 1) * sizeof(char *));
        *mathFunc = (char *)realloc(*mathFunc, (*componentCount+ 1) * sizeof(char));

        // Allocate memory for the block and copy it
        (*components)[*componentCount] = strdup(block);

        // Store the operation associated with this component
        if (*componentCount> 0) {
            // Check if the block is a negative number
            if (block[0] == '-' && (block[1] == '\0' || block[1] == '.' || (block[1] >= '0' && block[1] <= '9'))) {
                // This is a negative number
                (*mathFunc)[*componentCount- 1] = '+';
            } else {
                // This is a normal operation
                (*mathFunc)[*componentCount- 1] = input[block - copy - 1];
            }
        }

        // Increment the number of components
        (*componentCount)++;

        // Get the next block
        block = customStrtok(NULL, divider);
    }

    // Handle the special case of "+-" in the input
    for (int i = 0; i < *componentCount- 1; ++i) {
        if ((*mathFunc)[i] == '+' && (*components)[i + 1][0] == '-') {
            (*mathFunc)[i] = '-';
            memmove(&(*components)[i + 1][0], &(*components)[i + 1][1], strlen((*components)[i + 1]));
        }
    }

    // Free the copied input string
    free(copy);
}


void model_init() {
    for (int i = 0; i < NUM_ROWS; ++i) {
        array_of_heads[i] = (struct cellNode*)malloc(sizeof(struct cellNode)); // Allocate memory

        array_of_heads[i]->data.col=0; 
        array_of_heads[i]->nextCol = NULL; 

        array_of_heads[i]->data.value.number = (float)0.0; // Set blank state of float to 0.0 (cant set a float to NULL)
        array_of_heads[i]->data.value.string = NULL;
        array_of_heads[i]->data.value.equation = NULL;
        array_of_heads[i]->data.dependencies.depName = NULL;
        array_of_heads[i]->data.dependencies.nextNode = NULL;
    }


}

//Search for Cell inputted into function
bool cell_search(ROW row, COL col, struct cellNode **out) {
    *out = array_of_heads[row];
    if (col == 0) {
        return true;
    }if ((*out)->nextCol == NULL) { // Cell does not exist
        return false;
    }
    while ((*out)->nextCol != NULL && (*out)->data.col <= col) {
        if ((*out)->data.col == col) { // Found it, out points to the target cell
            return true;
        } else if ((*out)->nextCol->data.col > col) { // Not found, we went too far
            return false;
        } else if ((*out)->nextCol->data.col == col) { // Check 1 ahead so we dont go too far
            *out = (*out)->nextCol;
            return true;
        } else {
            *out = (*out)->nextCol; // Move out to next col
        }
    }
    return false; // If cell not found, out points to the leftmost cell in the correct row
}



void update_dependents(struct cellNode *node) {
    struct dependencies newDep = node->data.dependencies;

    while (newDep.depName != NULL){
        int foundCol = newDep.depName[0] - 65; 
        int foundRow = (int)strtol(newDep.depName+1,NULL,10);

        struct cellNode *dependant_Node = NULL;
        if(cell_search(foundRow-1,foundCol,&dependant_Node)){
            set_cell_value(foundRow-1,foundCol,strdup(dependant_Node->data.value.equation));
            update_dependents(dependant_Node); // Update the dependencies of the dependent cells 
        } 

        if (newDep.nextNode != NULL){
            newDep = *newDep.nextNode;
        } else {
            break;
        }
    }
}




void set_cell_value(ROW row, COL col, char *text) { 
    // IMPORTANT: By convention, when creating a new node, we link it to the leftmost node in the same row
    // IMPORTANT: By convention, the first node in each row (col A) always exists and is used to link cells inputed into it's row

    struct cellNode *found_node = NULL; 
    struct cellNode *new_node = NULL;

    if (!cell_search(row,col,&found_node)){ // If we did not find the node since it doesnent already exist
        new_node = malloc(sizeof (struct cellNode)); 
        struct cellNode *temp = found_node->nextCol; 

        found_node->nextCol = new_node; // Connect new_node to leftmost node 
        new_node->nextCol = temp; //Connect new_node to the next node to the right in the row

        // Set initial data and dependencies 
        new_node->data.col = col;
        new_node->data.inUse = 0;
        new_node->data.value.string = NULL;
        new_node->data.value.number = (float)0.0;
        new_node->data.value.equation = NULL;
        new_node->data.dependencies.depName = NULL;
        new_node->data.dependencies.nextNode = NULL;
    } else {
        new_node = found_node; // Set foud_node to new_node 
    } 
    
    new_node->data.inUse = 1;

    // Now we must parse the cell input 
    if (text[0] != '='){ // Not an Equation
        if(strcmp(text,"DELETE")==0){ // If we type "DELETE", clear cell
            clear_cell(row,col);
            update_dependents(new_node);
            return;
        }

        if (isString(text)){ //If the user inputed text
            new_node->data.value.string = strdup(text);
            new_node->data.value.number = (float)0.0;
            new_node->data.value.equation = NULL;
        } else { //If the user inputed a number
            new_node->data.value.number = strtof(text,NULL);
            new_node->data.value.string = NULL;
            new_node->data.value.equation = NULL;
        }

        update_dependents(new_node); // Update dependent cells 

    } else { // The user inputed an equation 
        new_node->data.value.equation = strdup(text); // Add Equation to our data structure
        new_node->data.value.string = NULL;
        new_node->data.value.number = (float)0.0;

        char *choptext = text+1; // Removes the '=' sign
        int componentCount= 0;
        char **component;
        char *mathFunc;

        string_divider(choptext, "+-*/", &component, &mathFunc ,&componentCount);


        for (int i = 0; i < componentCount; ++i) {
            found_node = NULL;
            char operation = (i > 0) ? mathFunc[i - 1] : '+';

            if ((component[i][0] >= 'A' && component[i][0] <= 'Z')){ // Check the first character is a letter 
                if (isIntValid(component[i]+1)){ // Check the second character is a number
                    int foundCol = component[i][0] - 65;
                    int foundRow = (int)strtol(component[i]+1,NULL,10); // Compute the row and col being referenced

                    if (foundRow > NUM_ROWS || foundCol > NUM_COLS){ // Invalid case where we go outside of the spreadsheet, Ex: P20
                        char errorText[8] = "INVALID";
                        update_cell_display(row,col,errorText);
                        return;
                    }
                    if (foundRow-1 == row && foundCol == col){ 
                        char errorText[8] = "INVALID";
                        update_cell_display(row,col,errorText);
                        return;
                    }
                    if(cell_search(foundRow-1,foundCol,&found_node)){ // Search for cell being referenced
                        char *cellString = (char*) malloc(3);
                        char currentCol = col + 'A';
                        sprintf(cellString,"%c%d",currentCol,row+1); // Convert current Row and Column to string

                        if (found_node->data.dependencies.depName == NULL){
                            found_node->data.dependencies.depName = strdup(cellString); // Set dependencies
                        }

                        while (found_node->data.dependencies.nextNode != NULL){
                            if (strcmp(found_node->data.dependencies.depName,component[i]) != 0){ // If dependent cell is not already saved in the structure
                                if (found_node->data.dependencies.nextNode == NULL){ 
                                    found_node->data.dependencies.nextNode->depName = strdup(cellString);
                                }
                            }
                        }

                        if (found_node->data.value.string == NULL){ // If found node is not a string, do the math operation 
                            switch (operation) {
                                case '+':
                                    new_node->data.value.number += found_node->data.value.number;
                                    break;
                                case '-':
                                    new_node->data.value.number -= found_node->data.value.number;
                                    break;
                                case '*':
                                    new_node->data.value.number *= found_node->data.value.number;
                                    break;
                                case '/':
                                    new_node->data.value.number /= found_node->data.value.number;
                                    break;
                            }
                        } else { // Display invalid if we trying doing math with a string
                            char errorText[8] = "INVALID";
                            update_cell_display(row,col,errorText);
                            return;
                        }
                    } else{ // If we cant find the cell being referenced 
                        char errorText[8] = "INVALID";
                        update_cell_display(row,col,errorText);
                        return;
                    }
                } else { // Invalid cell input 
                    char errorText[8] = "INVALID";
                    update_cell_display(row,col,errorText);
                    return;
                }

            } else { // Not referencing a cell, must be referencing a number or invalid 
                if (!isString(component[i])){ // If it's a number, do the math
                    switch (operation) {
                        case '+':
                            new_node->data.value.number += strtof(component[i], NULL);
                            break;
                        case '-':
                            new_node->data.value.number -= strtof(component[i], NULL);
                            break;
                        case '*':
                            new_node->data.value.number *= strtof(component[i], NULL);
                            break;
                        case '/':
                            new_node->data.value.number /= strtof(component[i], NULL);
                            break;
                    }
                } else { // Not a number or cell, invalid
                    char errorText[8] = "INVALID";
                    update_cell_display(row,col,errorText);
                    return;
                }
            }
        }
        update_dependents(new_node);

        char *printText = (char*)malloc(30);
        sprintf(printText,"%g",new_node->data.value.number);

        update_cell_display(row, col, printText); // Update text to display results from equation  
        return;
    }
    update_cell_display(row, col, text);
}


void clear_cell(ROW row, COL col) {
    struct cellNode *found_node = NULL;

    if (col == 0){ // If the node to clear is in col A
        array_of_heads[row]->data.value.number = (float)0.0;
        array_of_heads[row]->data.value.string = NULL;
        array_of_heads[row]->data.value.equation = NULL;
        array_of_heads[row]->data.inUse=0;

    } else if (cell_search(row,col-1,&found_node)){ // If the previous node is the previous cell, Ex: A3 linked to A4
        struct cellNode *old_node = NULL; // Found_node is the previous node

        old_node = found_node->nextCol;
        found_node->nextCol = old_node->nextCol;
        free(old_node);
    } else { // If the previous node is not the previous cell, Ex: A3 linked to A6
        found_node = array_of_heads[row];
        struct cellNode *old_node = NULL;  // Found_node is the previous node

        while (found_node->nextCol != NULL){
            if (found_node->nextCol->data.col == col){
                old_node = found_node->nextCol;
                found_node->nextCol = old_node->nextCol;
                free(old_node);
            } else{
                found_node = found_node->nextCol;
            }
        }
    }
    update_cell_display(row, col, ""); // Update display to show nothing 
}

char *get_textual_value(ROW row, COL col) { // Get textual value to be displayed in the interface
    struct cellNode *found_node = NULL;
    char *text = (char*)malloc(30);

    if(cell_search(row,col,&found_node)){ // Check if there is a node
        if (found_node->data.inUse == 0){
            text = strdup(" ");
            return text;
        }
        if (found_node->data.value.equation != NULL){ // If there is an equation
            return strdup(found_node->data.value.equation);
        } else {
            if (found_node->data.value.string !=NULL){  //If there is text
                return strdup(found_node->data.value.string);
            } else { // If theres a number 
                sprintf(text,"%g",found_node->data.value.number); 
                return text;
            }
        }
    }
    return NULL;
}






