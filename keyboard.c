/***************************************************************
 * Reads keyboard input and saves it to a buffer               *
 * Author: Shayne Kelly II                                     *
 * Date: June 7, 2017                                          *
 ***************************************************************/

/***************************************************************
 * Imports                                                     *
 ***************************************************************/
#include "keyboard.h"
#include "list.h"
#include <stdio.h>
#include <unistd.h>

/***************************************************************
 * Defines                                                     *
 ***************************************************************/
#define BUF_SIZE 1000
#define TERMINAL_FD 0

/***************************************************************
 * Statics                                                     *
 ***************************************************************/
static char buffer[BUF_SIZE];

/***************************************************************
 * Global Functions                                            *
 ***************************************************************/

/* Infinite loop, read text from keyboard */
int main(void) {
	read(TERMINAL_FD, buffer, BUF_SIZE);
	printf("The buffer contains: %s", buffer);
	return 0;
}

void AcceptKeyboardInput() {
	read(TERMINAL_FD, buffer, BUF_SIZE);
}
