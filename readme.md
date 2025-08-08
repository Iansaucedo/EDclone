# EDclone - Simple Command-Line Text Editor

## Overview

EDclone is a simple command-line text editor written in C. It allows you to add, edit, delete, print, save, and open lines of text interactively. All data is stored in memory and can be saved to or loaded from a text file.

## Features

- Add lines of text interactively
- Print all lines with line numbers
- Edit or delete specific lines by number
- Save the current document to a file
- Open a document from a file (replaces current lines)
- Exit the program safely

## Usage

Run the program:

```sh
gcc lexer.c -o lexer
./lexer
```

### Commands

- Add a line: Type any text and press Enter
- Print all lines: `-p`
- Find a line: `-f <number>`
- Edit a line: `-e <number> <new text>`
- Delete a line: `-d <number>`
- Save to file: `-w <filename>` (default: `document.txt` if no filename given)
- Open a file: `.open <filename>`
- Exit: `.exit`

### Example Session

```
> Hello world
Line added.
> This is a test
Line added.
> -p
1: Hello world
2: This is a test
Executed.
> -e 2 Goodbye
Executed.
> -p
1: Hello world
2: Goodbye
Executed.
> -w myfile.txt
Saved document to: myfile.txt
Executed.
> .open myfile.txt
Opened document: myfile.txt
> .exit
```

## Notes

- Saving or opening a file will overwrite the current in-memory document.
- All line numbers are automatically reindexed after deletion.
- The program is intended for educational/demo purposes and does not support advanced text editing features.
  Taken inspiration of lexer implementation by the project of https://cstack.github.io/db_tutorial/
