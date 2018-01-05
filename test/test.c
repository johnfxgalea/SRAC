/**
 * test.c
 *
 *  Created on: Jan 5, 2018
 *  Author: John F.x. Galea
 */

#include <stdio.h>
#include <string.h>

void copy_and_print(char *str){

	char str_cpy[20];

	// Perform no boundary checks.
	strcpy(str_cpy, str);

	printf("The copied string is %s\n", str_cpy);
}

int main(int argc, char **argv){

	if (argc == 2){
		// Call buggy function
		copy_and_print((char *) argv[1]);
	}else{
		printf("No param was specified\n");
	}
}
