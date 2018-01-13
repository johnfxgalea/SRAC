/**
 * test.c
 *
 *  Created on: Jan 5, 2018
 *  Author: John F.x. Galea
 */

#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER)
# define EXPORT __declspec(dllexport)
#else  // _MSC_VER
# define EXPORT __attribute__ ((visibility ("default")))
#endif

void copy_and_print(char *str){

  char str_cpy[20];

  // Perform no boundary checks.
  strcpy(str_cpy, str);

  printf("The copied string is %s\n", str_cpy);
}

EXPORT int prog_main(int argc, char *arg){

  if (argc == 2){
    // Call buggy function
    copy_and_print((char *) arg);
  }else{
    printf("No param was specified\n");
  }
}

int main(int argc, char **argv){

  return  prog_main(argc, argv[1]);
}
