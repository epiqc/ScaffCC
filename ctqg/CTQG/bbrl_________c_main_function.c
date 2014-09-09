extern sgt_ATOM name_to_index_in_main_var_decl_table;



static void
init_name_to_index_in_main_var_decl_table(
void) {
  #ifdef MAIN_VAR_DECL_TABLE_IS_PRESENT
  ptr_to_entry_of_sgt_ATOM ptr_to_new_entry;
  uint64_t                 i;
  for (i = 0; i < NUM_MAIN_VAR_DECL; i++) {
    new_entry_for_sgt_ATOM(ptr_to_new_entry)
    h_ATOM_alloc_strcpy_hs(
      &(ptr_to_new_entry->e.body.key_s),
      main_var_decl_table[i].name + 13 /* 13 = strlen("main_module::") */
    );
    ptr_to_new_entry->e.body.key_i = 0;
    ptr_to_new_entry->e.body.value_s = NULL;
    ptr_to_new_entry->e.body.value_i = i;

    if (insert__if_conflicts_bounce__sgt_ATOM(
          ptr_to_new_entry,
          &name_to_index_in_main_var_decl_table)) {
      /* Do nothing. */
    } else {
      delete_ATOM(&(ptr_to_new_entry->e.body));
      delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
      fprintf(stderr, "Error.\n");
      fprintf(stderr, "Duplicate name in main variable declarations table.\n");
      clean_exit(-1);
    }
  }
  #endif
}



int /* Ret. (1 != 0) on success, (0 != 0) if not found. */
search_main_var_decl_table(
int * signal_type, uint64_t * num_indexes, uint64_t ** ranges,
str_on_heap_ATOM key) {
  #ifdef MAIN_VAR_DECL_TABLE_IS_PRESENT
  ATOM * ptr_to_search_result;
  ATOM   search_key;
  int    rv;

  search_key.key_s = key; /* `search_key.key_s' does not own memory. */
  search_key.key_i = 0;
  find__if_many_give_eps_smallest__sgt_ATOM(
    ptr_to_search_result, search_key, name_to_index_in_main_var_decl_table)
  if (ptr_to_search_result != NULL) {
    *signal_type =
      main_var_decl_table[ptr_to_search_result->value_i].signal_type;
    *num_indexes =
      main_var_decl_table[ptr_to_search_result->value_i].num_indexes;
    *ranges =
      main_var_decl_table[ptr_to_search_result->value_i].ranges;
    rv = (1 != 0);
  } else {
    *signal_type = -1;
    *num_indexes = -1;
    *ranges = NULL;
    rv = (0 != 0);
  }
  return rv;
  #endif

  #ifndef MAIN_VAR_DECL_TABLE_IS_PRESENT
  *signal_type = -1;
  *num_indexes = -1;
  *ranges = NULL;
  return (0 != 0);
  #endif
}



int
main(
int argc, char ** argv) {
  char     out_file_name[4096];
  char     out_signals_file_name[4096];
  char     ancilla_file_name[4096];
  char     sim_in_file_name[4096];
  char     sim_out_file_name[4096];
  uint64_t length_of_out_file_name;
  uint64_t length_of_out_signals_file_name;
  uint64_t length_of_ancilla_file_name;
  uint64_t length_of_sim_in_file_name;
  uint64_t length_of_sim_out_file_name;

  if (argc < 1) {
    fprintf(stderr,
            "Error.\n"
            "Something is wrong with main(...) : argc < 1.\n");
    clean_exit(-1);
  }

  memory_manager_message_level = 0;
  init_name_to_index_in_main_var_decl_table();

  length_of_out_file_name = strlen(argv[0]) + 32;
  if (length_of_out_file_name >= 4096) {
    fprintf(stderr,
            "Error.\n"
            "The name of the output file is too long.\n");
    clean_exit(-1);
  }
  sprintf(out_file_name, "%s.gates", argv[0]);

  length_of_ancilla_file_name = strlen(argv[0]) + 32;
  if (length_of_ancilla_file_name >= 4096) {
    fprintf(stderr,
            "Error.\n"
            "The name of the ancilla file is too long.\n");
    clean_exit(-1);
  }
  sprintf(ancilla_file_name, "%s.ancilla", argv[0]);

  if (argc == 1) {
    /* Write circuit to file `out_file_name'. */
    out_file = fopen(out_file_name, "w");
    if (out_file == NULL) {
      fprintf(stderr,
              "Error.\n"
              "Can not open file \"%s\" for writing.\n",
              out_file_name);
      clean_exit(-1);
    }
    memory_manager_message_level = 0;
    main_module("N/A", 0, 0);
    fclose(out_file);
    out_file = NULL;
    if (0) {
      printf("The sequence of gates is written to file `%s'.\n", out_file_name);
    }
    printf("\n");
    statistics_fprint(stdout);
    printf("\n");
  } else if (argc == 2) {
    /* Create the i/o signals file. */
    length_of_out_signals_file_name = strlen(argv[1]) + 32;
    if (length_of_out_signals_file_name >= 4096) {
      fprintf(stderr,
              "Error.\n"
              "The name of the output signals file is too long.\n");
      clean_exit(-1);
    }
    sprintf(out_signals_file_name, "%s", argv[1]);
    out_signals_file = fopen(out_signals_file_name, "w");
    if (out_signals_file == NULL) {
      fprintf(stderr,
              "Error.\n"
              "Can not open file \"%s\" for writing.\n",
              out_signals_file_name);
      clean_exit(-1);
    }
    ancilla_file = fopen(ancilla_file_name, "w");
    if (ancilla_file == NULL) {
      fprintf(stderr,
              "Error.\n"
              "Can not open file \"%s\" for writing.\n",
              ancilla_file_name);
      clean_exit(-1);
    }
    memory_manager_message_level = 0; /* 1 */
    main_module("N/A", 0, 0);
    create_signals_file();
    fclose(out_signals_file);
    out_signals_file = NULL;
    create_ancilla_file_for_qasm();
    fclose(ancilla_file);
    ancilla_file = NULL;
    if (0) {
      printf("\n");
      statistics_fprint(stdout);
      printf("\n");
      printf("The i/o signals are written to file `%s'.\n"
             "Replace `.'s with `0's and `1's for simulation.\n"
             "To simulate the circuit:\n"
             "`my_circuit <IN i/o signals with values file> "
             "<OUT i/o signals with values file>'.\n",
             out_signals_file_name);
    }
  } else if (argc == 3) {
    /* Simulate. */
    length_of_sim_out_file_name = strlen(argv[1]) + 32;
    if (length_of_sim_out_file_name >= 4096) {
      fprintf(stderr,
              "Error.\n"
              "The name of the sim_out file is too long.\n");
      clean_exit(-1);
    }
    sprintf(sim_out_file_name, "%s", argv[2]);
    sim_out_file = fopen(sim_out_file_name, "w");
    if (sim_out_file == NULL) {
      fprintf(stderr,
              "Error.\n"
              "Can not open file \"%s\" for writing.\n",
              sim_out_file_name);
      clean_exit(-1);
    }
    length_of_sim_in_file_name = strlen(argv[1]) + 32;
    if (length_of_sim_in_file_name >= 4096) {
      fprintf(stderr,
              "Error.\n"
              "The name of the sim_in file is too long.\n");
      clean_exit(-1);
    }
    sprintf(sim_in_file_name, "%s", argv[1]);
    sim_in_file = fopen(sim_in_file_name, "r");
    if (sim_in_file == NULL) {
      fprintf(stderr,
              "Error.\n"
              "Can not open file \"%s\" for reading.\n",
              sim_in_file_name);
      clean_exit(-1);
    }
    memory_manager_message_level = 0;
    read_sim_in_file__to__set_of_signals_used_in_main();
    main_module("N/A", 0, 0);
    create_sim_out_file();
    fclose(sim_in_file);
    sim_in_file = NULL;
    fclose(sim_out_file);
    sim_out_file = NULL;
  } else {
    fprintf(stderr,
            "Error.\n"
            "Too many arguments.\n");
    clean_exit(-1);
  }

  clean_exit(0);
  return 0; /* Make compiler happy. */
}
