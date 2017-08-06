#include <stdio.h>
#include <string>

#define ctrl_target 1
#define module void
#define qbit char
#define rkqc void
#define qint char
#define cbit char
#define toffoli qg_Toffoli
char qg_X(char &a){ a = !a; printf("X %d\n",a); return a;}

#if ctrl_target
char qg_Toffoli(char &ctrl, char &ctrl2, char &target){ if (ctrl & ctrl2 == 1) target = (target==0) ? 1:0 ; return 0;}
char qg_CNOT(char &ctrl, char &target){ if (ctrl == 1) target = (target==0) ? 1:0; return 0;}
#else
char qg_Toffoli(char &target, char &ctrl, char &ctrl2){ if (ctrl & ctrl2 == 1) target = (target==0) ? 1:0 ; return 0;}
char qg_CNOT(char &target, char &ctrl){ if (ctrl == 1) target = (target==0) ? 1:0; return 0;}
#endif

void var_display(const char *name, char *a, int n){
  int i;
  printf("%s: ", name);
  for (i = n - 1; i >= 0; i--)
    printf("%d", a[i]);
  printf("\n");
}

void assign_value_of_int_to_a(char &a, char &b, int size = 1 ){
  a = b;   
}
void assign_value_of_b_to_a(char &a, char &b, int size = 1 ){
  a = b;
}
void assign_value_of_b_to_a(char a, std::string binary_constant,  int size = 1 ){
  
}
void a_eq_a_plus_b(char &a, char &b, int size = 1){
  a = (a + b) & 1;
}
void a_swap_b(char &a, char &b, int size = 1){
  char tmp = b;
  b = a;
  a = tmp;
}
void a_eq_a_minus_b(char &a, char &b, int size = 1){
  a = (a - b) & 1;
}
void a_eq_a_plus_b_times_c(char &a, char &b, char &c, int size = 1){
  a = (a + (b * c)) & 1;
}
void a_eq_a_minus_b_times_c(char &a, char b, char c, int size = 1){
  a = (a - (b * c)) & 1;
}

void assign_value_of_1_to_a(char &a, int size = 1){
  a = 1;
}

void assign_value_of_0_to_a(char &a, int size = 1){
  a = 0;
}

void NOT(char &a){
  a = (a==0) ? 1: 0;
}
