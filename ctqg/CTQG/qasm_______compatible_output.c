#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>



#define SEPARATOR_CHARACTER 'I'

char   signals_filename[4096];
char   gates_filename[4096];
char   ancilla_filename[4096];
char   qasm_filename[4096];
FILE * signals_file = NULL;
FILE * gates_file = NULL;
FILE * ancilla_file = NULL;
FILE * qasm_file = NULL;

int    print_comma = (0 != 0);



void
open_files(
void) {
  signals_file = fopen(signals_filename, "r");
  if (signals_file == NULL) {
    fprintf(
      stderr,
      "Error.\n"
      "`convert_to_qasm' can not open file \"%s\".\n",
      signals_filename
    );
    exit(-1);
  }
  gates_file = fopen(gates_filename, "r");
  if (gates_file == NULL) {
    fprintf(
      stderr,
      "Error.\n"
      "`convert_to_qasm' can not open file \"%s\".\n",
      gates_filename
    );
    exit(-1);
  }
  ancilla_file = fopen(ancilla_filename, "r");
  if (ancilla_file == NULL) {
    fprintf(
      stderr,
      "Error.\n"
      "`convert_to_qasm' can not open file \"%s\".\n",
      ancilla_filename
    );
    exit(-1);
  }
  qasm_file = fopen(qasm_filename, "w");
  if (qasm_file == NULL) {
    fprintf(
      stderr,
      "Error.\n"
      "`convert_to_qasm' can not open file \"%s\".\n",
      qasm_filename
    );
    exit(-1);
  }
}



void
close_files(
void) {
  fclose(signals_file);
  signals_file = NULL;
  fclose(gates_file);
  gates_file = NULL;
  fclose(ancilla_file);
  ancilla_file = NULL;
  fclose(qasm_file);
  qasm_file = NULL;
}



void
create_signals_used_in_main(
void) {
  char     token[4096];
  char     var_name[4096];
  char *   pos_in_var_name;
  char *   pos_in_token;

  while (fscanf(signals_file, "%s", token) == 1) {
    if ( fscanf(signals_file, "%s", token) == 1) {
      /* Do nothing. */
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "`.signals' file for simulation is corrupted: "
              "a variable name is missing.\n");
      exit(-1);
    }

    pos_in_var_name = var_name;
    pos_in_token = token;
    while (*pos_in_token != '\0') {
      *pos_in_var_name = *pos_in_token;
      pos_in_var_name++;
      pos_in_token++;
    }

    while (1 != 0) {
      if (fscanf(signals_file, "%s", token) != 1) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "`.signals' file for simulation is corrupted.\n");
        exit(-1);
      }
      if (token[0] == '[') {
        pos_in_token = &(token[1]);
        *pos_in_var_name = SEPARATOR_CHARACTER;
        pos_in_var_name++;
        while (*pos_in_token != ']') {
          *pos_in_var_name = *pos_in_token;
          pos_in_var_name++;
          pos_in_token++;
        }
      } else
      if (token[0] == '~') {
        *pos_in_var_name = '\0';
        pos_in_var_name++;
        break;
      } else {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "`.signals' file for simulation is corrupted.\n");
        exit(-1);
      }
    }
    if (!print_comma) {
      print_comma = (1 != 0);
      fprintf(qasm_file, "\tqubit %s\n", var_name);
    } else {
      fprintf(qasm_file, "\tqubit %s\n", var_name);
    }
  }
}



void
create_ancilla_signals(
void) {
  char     sink[4][4096];
  uint32_t num_00;
  uint32_t num_11;
  uint32_t num_0g;
  uint32_t num_1g;
  uint32_t i;

  if (fscanf(
        ancilla_file, "%s%u%s%u%s%u%s%u",
        sink[0], &num_00,
        sink[1], &num_11,
        sink[2], &num_0g,
        sink[3], &num_1g) != 8) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Bad `.ancilla' file.\n");
    exit(-1);
  }

  for (i = 0; i < num_00; i++) {
    if (!print_comma) {
      print_comma = (1 != 0);
      fprintf(qasm_file, "\tqubit %c00%c%u\n",
        SEPARATOR_CHARACTER, SEPARATOR_CHARACTER, i);
    } else {
      fprintf(qasm_file, "\tqubit %c00%c%u\n",
        SEPARATOR_CHARACTER, SEPARATOR_CHARACTER, i);
    }
  }
  for (i = 0; i < num_11; i++) {
    if (!print_comma) {
      print_comma = (1 != 0);
      fprintf(qasm_file, "\tqubit %c11%c%u\n",
        SEPARATOR_CHARACTER, SEPARATOR_CHARACTER, i);
    } else {
      fprintf(qasm_file, "\tqubit %c11%c%u\n",
        SEPARATOR_CHARACTER, SEPARATOR_CHARACTER, i);
    }
  }
  for (i = 0; i < num_0g; i++) {
    if (!print_comma) {
      print_comma = (1 != 0);
      fprintf(qasm_file, "\tqubit %c0g%c%u\n",
        SEPARATOR_CHARACTER, SEPARATOR_CHARACTER, i);
    } else {
      fprintf(qasm_file, "\tqubit %c0g%c%u\n",
        SEPARATOR_CHARACTER, SEPARATOR_CHARACTER, i);
    }
  }
  for (i = 0; i < num_1g; i++) {
    if (!print_comma) {
      print_comma = (1 != 0);
      fprintf(qasm_file, "\tqubit %c1g%c%u\n",
        SEPARATOR_CHARACTER, SEPARATOR_CHARACTER, i);
    } else {
      fprintf(qasm_file, "\tqubit %c1g%c%u\n",
        SEPARATOR_CHARACTER, SEPARATOR_CHARACTER, i);
    }
  }
}



void
read_and_convert_signal_name_from_gates_file(
char * target) {
  uint32_t i, j;

  if ( fscanf(gates_file, "%s", target) == 1 ) {
    /* Do nothing. */
  } else {
    fprintf(stderr, "Error.\n");
    fprintf(stderr, "Can not read signal name from `.gates' file.\n");
    exit(-1);
  }
  if (target[0] == '@') {
    target[0] = SEPARATOR_CHARACTER;
    target[3] = SEPARATOR_CHARACTER;
    for (i = 4; target[i] != ']'; i++);
    target[i] = '\0';
  } else {
    for (i = 0, j = 0; target[j] != '\0'; j++) {
      if (target[j] == '[') {
        target[i++] = SEPARATOR_CHARACTER;
      } else
      if (target[j] == ']') {
        /* Do nothing, i.e. delete this character. */
      } else {
        target[i++] = target[j];
      }
    }
    target[i] = '\0';
  }
}



void
create_gates(
void) {
  char     token[4096];
  uint32_t num_control_signals;
  uint32_t i;

  while (fscanf(gates_file, "%s", token) == 1) {
    if (strcmp(token, "TOF") == 0) {
      if ( fscanf(gates_file, "%u", &num_control_signals) == 1 ) {
        /* Do nothing. */
      } else {
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "Bad `.gates' file.\n");
        exit(-1);
      }
      if (num_control_signals == 0) {
        fprintf(qasm_file, "\tX ");
        read_and_convert_signal_name_from_gates_file(token);
        fprintf(qasm_file, "%s\n", token);
      } else
      if (num_control_signals == 1) {
        fprintf(qasm_file, "\tcnot ");
        read_and_convert_signal_name_from_gates_file(token);
        fprintf(qasm_file, "%s,", token);
        read_and_convert_signal_name_from_gates_file(token);
        fprintf(qasm_file, "%s\n", token);
      } else
      {
        fprintf(qasm_file, "\ttoffoli ");
        for (i = 0; i < num_control_signals; i++) {
          read_and_convert_signal_name_from_gates_file(token);
          fprintf(qasm_file, "%s,", token);
        }
        read_and_convert_signal_name_from_gates_file(token);
        fprintf(qasm_file, "%s\n", token);
      }
    } else {
      fprintf(stderr,
        "A non-classical quantum gate has encountered. "
        "Not implemented yet.\n"
      );
      exit(-1);
    }
  }
}



int
main(
int argc, char ** argv) {
  if (argc < 5) {
    fprintf(
      stderr,
      "Error.\n"
      "`convert_to_qasm' executable must be given 4 arguments.\n"
    );
    exit(-1);
  }

  strcpy(signals_filename, argv[1]);
  strcpy(gates_filename, argv[2]);
  strcpy(ancilla_filename, argv[3]);
  strcpy(qasm_filename, argv[4]);

  open_files();

  create_signals_used_in_main();
  create_ancilla_signals();
  create_gates();

  close_files();

  return 0;
}
