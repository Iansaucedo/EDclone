#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  STATEMENT_PICKLINE
} StatementType;

typedef struct
{
  StatementType type;

} Statement;

typedef enum
{
  SUCCESS,
  UNRECOGNIZED_STATEMENT
} PrepareResult;

Line *head = NULL;

void append_line(const char *text)
{
  Line *new_line = malloc(sizeof(Line));
  new_line->text = strdup(text);
  new_line->next = NULL;

  if (head == NULL)
  {
    head = new_line;
  }
  else
  {
    Line *current = head;
    while (current->next != NULL)
    {
      current = current->next;
    }
    current->next = new_line;
  }
}

void find_line(int line_number)
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

edit_line(int line_number, const char *new_text)
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

void print_lines()
{
  Line *current = head;
  int line_num = 1;
  while (current != NULL)
  {
    printf("%s\n", current->text);
    current = current->next;
  }
}

void free_lines()
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

InputBuffer *new_input_buffer()
{
  InputBuffer *input_buffer = malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

CommandResult do_meta_command(InputBuffer *input_buffer)
{
  if (strcmp(input_buffer->buffer, ".exit") == 0)
  {
    close_input_buffer(input_buffer);
    exit(EXIT_SUCCESS);
  }
  else
  {
    return COMMAND_UNRECOGNIZED;
  }
}

PrepareResult prepare_statement(InputBuffer *input_buffer,
                                Statement *statement)
{
  if (strncmp(input_buffer->buffer, "-w", 4) == 0)
  {
    statement->type = STATEMENT_SAVE;
    return SUCCESS;
  }
  if (strcmp(input_buffer->buffer, "-p") == 0)
  {
    statement->type = STATEMENT_PICKLINE;
    return SUCCESS;
  }

  return UNRECOGNIZED_STATEMENT;
}

void execute_statement(Statement *statement)
{
  switch (statement->type)
  {
  case (STATEMENT_SAVE):
    printf("This is where we would do a save.\n");
    break;
  case (STATEMENT_PICKLINE):
    print_lines();
    break;
  }
}

void print_prompt() { printf("> "); }

void read_input(InputBuffer *input_buffer)
{
  ssize_t bytes_read =
      getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

  if (bytes_read <= 0)
  {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer *input_buffer)
{
  free(input_buffer->buffer);
  free(input_buffer);
}

int main(int argc, char *argv[])
{
  InputBuffer *input_buffer = new_input_buffer();
  while (true)
  {
    print_prompt();
    read_input(input_buffer);

    if (input_buffer->buffer[0] == '.')
    {
      switch (do_meta_command(input_buffer))
      {
      case (COMMAND_SUCCESS):
        continue;
      case (COMMAND_UNRECOGNIZED):
        printf("Unrecognized command '%s'\n", input_buffer->buffer);
        continue;
      }
    }

    if (input_buffer->buffer[0] == '-')
    {
      Statement statement;
      switch (prepare_statement(input_buffer, &statement))
      {
      case (SUCCESS):
        break;
      case (UNRECOGNIZED_STATEMENT):
        printf("Unrecognized keyword at start of '%s'.\n",
               input_buffer->buffer);
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
