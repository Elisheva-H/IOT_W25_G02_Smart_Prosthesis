
#include <stdio.h>
#include <string.h>

void function_a(void) { printf("Function A\n"); }
void function_b(void) { printf("Function B\n"); }
void function_c(void) { printf("Function C\n"); }
void function_d(void) { printf("Function D\n"); }
void function_e(void) { printf("Function E\n"); }

const static struct {
  const char *name;
  void (*func)(void);
  } 
  
  function_map [] = {
  { "function_a", function_a },
  { "function_b", function_b },
  { "function_c", function_c },
  { "function_d", function_d },
  { "function_e", function_e },
};

int call_function(const char *name)
{
  int i;
  for (i = 0; i < (sizeof(function_map) / sizeof(function_map[0])); i++) {
    if (!strcmp(function_map[i].name, name) && function_map[i].func) {
      function_map[i].func();
      return 0;
    }
  }

  return -1;
}

