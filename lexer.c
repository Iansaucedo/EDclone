#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct
{
  char *buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

typedef struct Line
{
  char *text;
  int line_number;
  struct Line *next;
} Line;

typedef enum
{
  COMMAND_SUCCESS,
  COMMAND_UNRECOGNIZED
} CommandResult;

typedef enum
{
  STATEMENT_SAVE,
  STATEMENT_PICKLINE,
  STATEMENT_FINDLINE,
  STATEMENT_EDITLINE,
  STATEMENT_DELETELINE
} StatementType;

typedef struct
{
  StatementType type;
  int line_number;
  char *edit_text;
  char *filename;
} Statement;

typedef enum
{
  SUCCESS,
  UNRECOGNIZED_STATEMENT
} PrepareResult;

static Line *head = NULL;

// Append a new line to the end of the list
static void append_line(const char *text)
{
  Line *new_line = malloc(sizeof(Line));
  new_line->text = strdup(text);
  new_line->next = NULL;
  Line *current = head;
  if (head == NULL)
  {
    new_line->line_number = 1;
    head = new_line;
  }
  else
  {
    while (current->next != NULL)
    {
      current = current->next;
    }
    new_line->line_number = current->line_number + 1;
    current->next = new_line;
  }
}

// Print a specific line by number
static void find_line(int line_number)
{
  Line *current = head;
  while (current != NULL)
  {
    if (current->line_number == line_number)
    {
      printf("Line %d: %s\n", current->line_number, current->text);
      return;
    }
    current = current->next;
  }
  printf("Line %d not found.\n", line_number);
}

// Edit a line's text by number
static void edit_line(int line_number, const char *new_text)
{
  Line *current = head;
  while (current != NULL)
  {
    if (current->line_number == line_number)
    {
      free(current->text);
      current->text = strdup(new_text);
      return;
    }
    current = current->next;
  }
  printf("Line %d not found for editing.\n", line_number);
}

// Delete a line by number and reindex
static void delete_line(int line_number)
{
  Line *current = head;
  Line *prev = NULL;
  while (current != NULL)
  {
    if (current->line_number == line_number)
    {
      if (prev == NULL)
      {
        head = current->next;
      }
      else
      {
        prev->next = current->next;
      }
      free(current->text);
      free(current);
      printf("Line %d deleted.\n", line_number);
      // Reindex line numbers
      Line *iter = head;
      int idx = 1;
      while (iter != NULL)
      {
        iter->line_number = idx++;
        iter = iter->next;
      }
      return;
    }
    prev = current;
    current = current->next;
  }
  printf("Line %d not found for deletion.\n", line_number);
}

// Print all lines
static void print_lines(void)
{
  Line *current = head;
  while (current != NULL)
  {
    printf("%d: %s\n", current->line_number, current->text);
    current = current->next;
  }
}

// Free all lines
static void free_lines(void)
{
  Line *current = head;
  while (current != NULL)
  {
    Line *tmp = current;
    current = current->next;
    free(tmp->text);
    free(tmp);
  }
  head = NULL;
}

// Create a new input buffer
static InputBuffer *new_input_buffer(void)
{
  InputBuffer *input_buffer = malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;
  return input_buffer;
}

// Handle meta commands (starting with '.')
// Open a document from a file
static void open_document(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file)
  {
    printf("Could not open file: %s\n", filename);
    return;
  }
  free_lines();
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  while ((read = getline(&line, &len, file)) != -1)
  {
    if (read > 0 && line[read - 1] == '\n')
      line[read - 1] = '\0';
    append_line(line);
  }
  free(line);
  fclose(file);
  printf("Opened document: %s\n", filename);
}

static CommandResult do_meta_command(InputBuffer *input_buffer)
{
  if (strcmp(input_buffer->buffer, ".exit") == 0)
  {
    close_input_buffer(input_buffer);
    exit(EXIT_SUCCESS);
  }
  if (strncmp(input_buffer->buffer, ".open ", 6) == 0)
  {
    char *filename = input_buffer->buffer + 6;
    while (*filename == ' ')
      filename++;
    if (*filename)
    {
      open_document(filename);
      return COMMAND_SUCCESS;
    }
    else
    {
      printf("No filename provided to open.\n");
      return COMMAND_SUCCESS;
    }
  }
  return COMMAND_UNRECOGNIZED;
}

// Parse user input into a statement
static PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement)
{
  statement->filename = NULL;
  if (strncmp(input_buffer->buffer, "-w", 2) == 0)
  {
    char *rest = input_buffer->buffer + 2;
    while (*rest == ' ')
      rest++;
    if (*rest)
    {
      statement->filename = strdup(rest);
    }
    else
    {
      statement->filename = strdup("document.txt");
    }
    statement->type = STATEMENT_SAVE;
    return SUCCESS;
  }
  if (strcmp(input_buffer->buffer, "-p") == 0)
  {
    statement->type = STATEMENT_PICKLINE;
    return SUCCESS;
  }
  if (strncmp(input_buffer->buffer, "-f ", 3) == 0)
  {
    int num;
    if (sscanf(input_buffer->buffer, "-f %d", &num) == 1)
    {
      statement->type = STATEMENT_FINDLINE;
      statement->line_number = num;
      return SUCCESS;
    }
  }
  if (strncmp(input_buffer->buffer, "-e ", 3) == 0)
  {
    int num;
    char *rest = strchr(input_buffer->buffer + 3, ' ');
    if (rest && sscanf(input_buffer->buffer, "-e %d", &num) == 1)
    {
      statement->type = STATEMENT_EDITLINE;
      statement->line_number = num;
      rest = strchr(input_buffer->buffer + 3, ' ');
      if (rest)
      {
        while (*rest == ' ')
          rest++;
        statement->edit_text = strdup(rest);
        return SUCCESS;
      }
    }
  }
  if (strncmp(input_buffer->buffer, "-d ", 3) == 0)
  {
    int num;
    if (sscanf(input_buffer->buffer, "-d %d", &num) == 1)
    {
      statement->type = STATEMENT_DELETELINE;
      statement->line_number = num;
      return SUCCESS;
    }
  }
  return UNRECOGNIZED_STATEMENT;
}

// Execute a parsed statement
// Save all lines to a file
static void save_document(const char *filename)
{
  FILE *file = fopen(filename, "w");
  if (!file)
  {
    printf("Could not open file for writing: %s\n", filename);
    return;
  }
  Line *current = head;
  while (current != NULL)
  {
    fprintf(file, "%s\n", current->text);
    current = current->next;
  }
  fclose(file);
  printf("Saved document to: %s\n", filename);
}

static void execute_statement(Statement *statement)
{
  switch (statement->type)
  {
  case STATEMENT_SAVE:
    if (statement->filename)
    {
      save_document(statement->filename);
      free(statement->filename);
    }
    break;
  case STATEMENT_PICKLINE:
    print_lines();
    break;
  case STATEMENT_FINDLINE:
    find_line(statement->line_number);
    break;
  case STATEMENT_EDITLINE:
    edit_line(statement->line_number, statement->edit_text);
    free(statement->edit_text);
    break;
  case STATEMENT_DELETELINE:
    delete_line(statement->line_number);
    break;
  }
}

static void print_prompt(void) { printf("> "); }

// Read a line of input from stdin
static void read_input(InputBuffer *input_buffer)
{
  ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
  if (bytes_read <= 0)
  {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }
  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

// Free the input buffer
static void close_input_buffer(InputBuffer *input_buffer)
{
  free(input_buffer->buffer);
  free(input_buffer);
}

int main(void)
{
  InputBuffer *input_buffer = new_input_buffer();
  while (1)
  {
    print_prompt();
    read_input(input_buffer);

    if (input_buffer->buffer[0] == '.')
    {
      switch (do_meta_command(input_buffer))
      {
      case COMMAND_SUCCESS:
        continue;
      case COMMAND_UNRECOGNIZED:
        printf("Unrecognized command '%s'\n", input_buffer->buffer);
        continue;
      }
    }

    if (input_buffer->buffer[0] == '-')
    {
      Statement statement;
      switch (prepare_statement(input_buffer, &statement))
      {
      case SUCCESS:
        break;
      case UNRECOGNIZED_STATEMENT:
        printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
        continue;
      }
      execute_statement(&statement);
      printf("Executed.\n");
    }
    else
    {
      append_line(input_buffer->buffer);
      printf("Line added.\n");
    }
  }
}