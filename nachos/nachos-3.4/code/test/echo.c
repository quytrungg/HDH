#include "syscall.h"
#include "copyright.h"

int main(){
	
	char str[50];
	int length = 50;
	OutputConsole("\n\t\t\t-----ECHO COMMAND-----\n\n");
	OutputConsole("-> echo: ");
	InputConsole(str, length);
	OutputConsole("\n-> echo: ");
	OutputConsole(str);
	Halt();
	return 0;
}
