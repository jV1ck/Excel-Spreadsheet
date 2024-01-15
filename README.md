# Excel-Project-278

Overview:
This repository contains a simple spreadsheet model implemented in C, providing a foundation for creating and manipulating spreadsheet-like structures. The project aims to offer flexibility for an unlimited number of rows and columns while ensuring efficient memory usage. The implementation follows a dynamic memory allocation approach and utilizes an array of linked lists to represent each row.

Features:
Cell Types: Support for different cell types, including numbers, text, and formulas.
Equation Processing: Ability to interpret formulas starting with an equals sign (=), allowing for basic arithmetic operations and cell references.
Dependency Management: Dependency tracking ensures that changes in one cell propagate to dependent cells.
Memory Management: Emphasis on proper memory allocation and deallocation to meet non-functional requirements.
User Interface Integration: Interface functions like set_cell_value, clear_cell, and get_textual_value for user-friendly interaction.
Additional Features: Support for subtraction (-), multiplication (*), and division (/) operations.
Implementation Details
Data Structures
Cell Structure:

Attributes: Column number, type, dependencies, and a union for number, text, or equation.
Cell Node Structure:

Represents a linked list node for each row in the spreadsheet.
Contains a cell structure and a pointer to the next column.
Dependencies Structure:

Manages dependencies of a cell through a linked list.
Functions
model_init():

Initializes the spreadsheet by creating an initial linked list structure for each row.
cell_search(row, col, &out):

Searches for a cell by row and column, providing a pointer to the found or preceding cell.
clear_cell(row, col):

Clears the content of a cell, frees memory, and updates the display.
set_cell_value(row, col, text):

Sets the value of a cell based on user input (number, text, or formula).
update_dependents(node):

Updates dependent cells recursively when a cell changes.
get_textual_value(row, col):

Retrieves the textual value of a cell for display.
isString(text):

Checks if the given text is a number or text.
customStrtok(str, delimiters):

Custom string tokenization function.
isIntValid(str):

Checks if the input string has valid integers.

Testing:
Extensive testing has been conducted to ensure the functionality of the spreadsheet model, covering various scenarios, including equation computation, circular references, and additional arithmetic operations.
