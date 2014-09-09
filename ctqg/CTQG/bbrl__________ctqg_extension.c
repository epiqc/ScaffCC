#include <ctqg_________________defines.h>
#include <stdint.h>



/* Data for processing of _one_ compound signal or constant.   */
uint64_t yQni; /* Number of indexes given on the calling line. */
int64_t  yQir[MAX_NUM_INDEXES]; /* `is a range'   != 0 - yes,    == 0 - no. */
int64_t  yQil[MAX_NUM_INDEXES];
int64_t  yQih[MAX_NUM_INDEXES];
int64_t  yQim[MAX_NUM_INDEXES];
int64_t  yQiee[MAX_NUM_INDEXES]; /* Ranges of the corr. arg. of the callee. */

int64_t  yQa[MAX_NUM_INDEXES];
int64_t  yQincr[MAX_NUM_INDEXES];
int64_t  r_running_index;
int64_t  ee_running_index;



void
print_stderr_sig_name_and_subranges(
char * name) {
  uint64_t i;

  fprintf(stderr, "%s", name);
  for (i = 0; i < yQni; i++) {
    if (yQir[i] != 0) {
      fprintf(stderr, "[%d..%d]", (int32_t) yQil[i], (int32_t) yQih[i]);
    } else {
      fprintf(stderr, "[%d]", (int32_t) yQil[i]);
    }
  }
}



void
print_stderr_caller_sig_name_and_subranges(
char * name) {
  uint64_t i;

  fprintf(stderr, "%s", name);
  for (i = 0; i < yQni; i++) {
    fprintf(stderr, "[0..%d]", (int32_t) (yQim[i] - 1));
  }
}



void
print_stderr_callee_sig_name_and_subranges(
char * name) {
  uint64_t i;
  uint64_t j;

  fprintf(stderr, "%s", name);
  j = 0;
  for (i = 0; i < yQni; i++) if (yQir[i] != 0) {
    fprintf(stderr, "[0..%d]", (int32_t) (yQiee[j] - 1));
    j++;
  }
}



void
map_cs(
char * file, uint32_t line, char * r_name, char * ee_name) {
  int64_t i = 0;
  int64_t j = 0;
  int64_t tail = 0;
  int64_t prod = 1;

  /* We have already made sure that the dimensions of the caller and the */
  /* callee arrays are equal (in `e_process_compound_signal(...)').      */
  while (i < yQni) {
    if (yQir[i] != 0) {
      if (yQil[i] < 0) {
        fprintf(stderr, "Error.\n");
        s_file_and_line_fprint(stderr, file, line);
        fprintf(stderr,
                " : Can not map compound signal  `");
        print_stderr_sig_name_and_subranges(r_name);
        fprintf(stderr,
                "'  because the lower bound of index %u is negative.\n",
                (uint32_t) (i + 1)
        );
        call_stack_fprint(stderr);
        clean_exit(-1);
      }
      if (yQih[i] >= yQim[i]) {
        fprintf(stderr, "Error.\n");
        s_file_and_line_fprint(stderr, file, line);
        fprintf(stderr,
                " : Can not map compound signal  `");
        print_stderr_sig_name_and_subranges(r_name);
        fprintf(stderr,
                "'  because the upper bound of index %u exceeds"
                " the upper bound of index %u in the declaration  `",
                (uint32_t) (i + 1),
                (uint32_t) (i + 1)
        );
        print_stderr_caller_sig_name_and_subranges(r_name);
        fprintf(stderr,
                "'.\n");
        call_stack_fprint(stderr);
        clean_exit(-1);
      }
      if (yQil[i] > yQih[i]) {
        fprintf(stderr, "Error.\n");
        s_file_and_line_fprint(stderr, file, line);
        fprintf(stderr,
                " : Can not map compound signal  `");
        print_stderr_sig_name_and_subranges(r_name);
        fprintf(stderr,
                "'  because the subrange of index %u is empty.\n",
                (uint32_t) (i + 1)
        );
        call_stack_fprint(stderr);
        clean_exit(-1);
      }
      if (yQih[i] - yQil[i] + ((int64_t)1) != yQiee[j]) {
        fprintf(stderr, "Error.\n");
        s_file_and_line_fprint(stderr, file, line);
        fprintf(stderr,
                " : Can not map compound signal  `");
        print_stderr_sig_name_and_subranges(r_name);
        fprintf(stderr,
                "'  because the size of the range of index %u is different "
                "from the size of the range of the corresponding "
                "index %u of the callee signal  `",
                (uint32_t) (i + 1),
                (uint32_t) (j + 1)
        );
        print_stderr_callee_sig_name_and_subranges(ee_name);
        fprintf(stderr,
                "'.\n");
        call_stack_fprint(stderr);
        clean_exit(-1);
      }
      j++;
    } else {
      yQih[i] = yQil[i];
      if (yQil[i] < 0) {
        fprintf(stderr, "Error.\n");
        s_file_and_line_fprint(stderr, file, line);
        fprintf(stderr,
                " : Can not map compound signal  `");
        print_stderr_sig_name_and_subranges(r_name);
        fprintf(stderr,
                "'  because index %u is negative.\n",
                (uint32_t) (i + 1)
        );
        call_stack_fprint(stderr);
        clean_exit(-1);
      }
      if (yQih[i] >= yQim[i]) {
        fprintf(stderr, "Error.\n");
        s_file_and_line_fprint(stderr, file, line);
        fprintf(stderr,
                " : Can not map compound signal  `");
        print_stderr_sig_name_and_subranges(r_name);
        fprintf(stderr,
                "'  because index %u exceeds"
                " the upper bound of index %u in the declaration  `",
                (uint32_t) (i + 1),
                (uint32_t) (i + 1)
        );
        print_stderr_caller_sig_name_and_subranges(r_name);
        fprintf(stderr,
                "'.\n");
        call_stack_fprint(stderr);
        clean_exit(-1);
      }
    }
    i++;
  }

  /* Compute initial `r_running_index' and set positional */
  /* system counter to initial state.                     */
  r_running_index = 0;
  for (i = 0; i < yQni; i++) {
    r_running_index = r_running_index * yQim[i] + yQil[i];
  }
  ee_running_index = 0;

  /* Compute yQincr[...] */
  /* `tail' is initialized to 0; `prod' is initialized to 1. */
  for (i = yQni - 1; i >= 0; i--) {
    yQincr[i] = prod - tail;
    tail += prod * (yQih[i] - yQil[i]);
    prod *= yQim[i];
    yQa[i] = yQil[i];
  }

  while (1 != 0) {
    /*
      printf("Mapping multiindex of caller ");
      for (i = 0; i < yQni; i++) {
        printf("%d ", (int32_t) yQa[i]);
      }
      printf(" [[ %d ]] <-> %d",
             (int32_t) r_running_index,
             (int32_t) ee_running_index);
    */

    map_signal(file, line, ee_name, ee_running_index, r_name, r_running_index);
    for (i = yQni - 1; i >= 0; i--) {
      if (++yQa[i] <= yQih[i]) break; else yQa[i] = yQil[i];
    }
    /* printf(" Changing digit %d.\n", (int32_t) i); */
    if (i < 0) break;
    r_running_index += yQincr[i];
    ee_running_index++;
  }
}



void
updt_anc_set(
uint64_t * yQnztz, uint64_t * yQnoto, char ** yQc, uint64_t * yQc_ee,
uint64_t num_c) {
  uint64_t i;
  uint64_t j;
  char *   pchar;
  uint64_t nztz = 0;
  uint64_t noto = 0;

  for (i = 0; i < num_c; i++) {

    for (pchar = yQc[i], j = 0;
         (*pchar != '\0') && (j < yQc_ee[i]);
         pchar++, j++) {
      if (*pchar == '0') {
        if (nztz >= *yQnztz) {
          zero_to_zero_ancilla("$auto_ancilla_for_const$", 0, "yQztz", nztz);
          (*yQnztz)++;
        }
        nztz++;
      } else
      if (*pchar == '1') {
        if (noto >= *yQnoto) {
          one_to_one_ancilla("$auto_ancilla_for_const$", 0, "yQoto", noto);
          (*yQnoto)++;
        }
        noto++;
      } else {
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "An internal BBRL error occurred: "
                        "a bit string contains characters other "
                        "then '0' and '1'.\n");
        call_stack_fprint(stderr);
        clean_exit(-2);
      }
    }

    pchar--;

    while (j < yQc_ee[i]) {
      if (*pchar == '0') {
        if (nztz >= *yQnztz) {
          zero_to_zero_ancilla("$auto_ancilla_for_const$", 0, "yQztz", nztz);
          (*yQnztz)++;
        }
        nztz++;
      } else
      if (*pchar == '1') {
        if (noto >= *yQnoto) {
          one_to_one_ancilla("$auto_ancilla_for_const$", 0, "yQoto", noto);
          (*yQnoto)++;
        }
        noto++;
      } else {
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "An internal BBRL error occurred: "
                        "a bit string contains characters other "
                        "then '0' and '1'.\n");
        call_stack_fprint(stderr);
        clean_exit(-2);
      }
      j++;
    }

  }
  /*
  printf("nztz_in_this_$b = %3u,  noto_in_this_$b = %3u.\n",
    (uint32_t) nztz, (uint32_t) noto);
  printf("allocated_nztz  = %3u,  allocated_noto  = %3u.\n",
    (uint32_t) *yQnztz, (uint32_t) *yQnoto);
  */
}



void
map_1dba(
char * file, uint32_t line, char * s, char * ee_name,
uint64_t * curr_ztz, uint64_t * curr_oto, uint32_t ee_ind0range) {
  int64_t i = 0;
  int64_t j;
  char *  pchar;

  /* We have already made sure that the dimensions of the           */
  /* callee array is 1 (in `e_process_1d_bit_array_constant(...)'). */

  for (pchar = s, j = 0;
       (*pchar != '\0') && (j < ee_ind0range);
       pchar++, j++) {
    if (*pchar == '0') {
      /*
      printf("MAP_ANCs %s %u %s *curr_ztz = %u\n",
        ee_name, (uint32_t) i, "yQztz", *curr_ztz);
      */
      map_signal(file, line, ee_name, i, "yQztz", *curr_ztz);
      (*curr_ztz)++;
      i++;
    } else
    if (*pchar == '1') {
      /*
      printf("MAP_ANCs %s %u %s *curr_oto = %u\n",
        ee_name, (uint32_t) i, "yQoto", *curr_oto);
      */
      map_signal(file, line, ee_name, i, "yQoto", *curr_oto);
      (*curr_oto)++;
      i++;
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr, "An internal BBRL error occurred: "
                      "a bit string contains characters other "
                      "then '0' and '1'.\n");
      call_stack_fprint(stderr);
      clean_exit(-2);
    }
  }

  pchar--;

  while (j < ee_ind0range) {
    if (*pchar == '0') {
      /*
      printf("MAP_ANCe %s %u %s *curr_ztz = %u\n",
        ee_name, (uint32_t) i, "yQztz", *curr_ztz);
      */
      map_signal(file, line, ee_name, i, "yQztz", *curr_ztz);
      (*curr_ztz)++;
      i++;
    } else
    if (*pchar == '1') {
      /*
      printf("MAP_ANCe %s %u %s *curr_oto = %u\n",
        ee_name, (uint32_t) i, "yQoto", *curr_oto);
      */
      map_signal(file, line, ee_name, i, "yQoto", *curr_oto);
      (*curr_oto)++;
      i++;
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr, "An internal BBRL error occurred: "
                      "a bit string contains characters other "
                      "then '0' and '1'.\n");
      call_stack_fprint(stderr);
      clean_exit(-2);
    }
    j++;
  }
}



void
map_bc(
char * file, uint32_t line, uint32_t v, char * ee_name,
uint64_t * curr_ztz, uint64_t * curr_oto) {
  /* We have already made sure that the dimensions of the           */
  /* callee array is 0 (in `e_process_bit_constant(...)'). */

  if (v == 0) {
    /*
    printf("MAP_ANCb %s %u %s *curr_ztz = %u\n",
      ee_name, 0, "yQztz", *curr_ztz);
    */
    map_signal(file, line, ee_name, 0, "yQztz", *curr_ztz);
    (*curr_ztz)++;
  } else {
    /*
    printf("MAP_ANCb %s %u %s *curr_oto = %u\n",
      ee_name, 0, "yQoto", *curr_oto);
    */
    map_signal(file, line, ee_name, 0, "yQoto", *curr_oto);
    (*curr_oto)++;
  }
}
