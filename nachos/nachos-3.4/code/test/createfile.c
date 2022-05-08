#include "syscall.h"
#include "copyright.h"
#define maxlen 32

int main(){
	int fileId;
	char filename[maxlen];
	OutputConsole("\n\t\t\t-----CREATE FILE COMMAND-----\n\n");
	OutputConsole("-> Input new filename: ");
	InputConsole(filename, maxlen);
	Create(filename);
	fileId = Open(filename, 0);
	Close(fileId);
	Halt();
	return 0;
}
