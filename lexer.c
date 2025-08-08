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
  if (strncmp(input_buffer->buffer, "w", 4) == 0)
  {
    statement->type = STATEMENT_SAVE;
    return SUCCESS;
  }
  if (strcmp(input_buffer->buffer, "p") == 0)
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
    printf("This is where we would do a pickline.\n");
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
}
