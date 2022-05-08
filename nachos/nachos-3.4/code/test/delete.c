#include "syscall.h"
#include "copyright.h"
#define maxlen 32

int main(){
	int openFileId, fileSize;
	char c, fileName[maxlen];
	int i;
	OutputConsole("\n\t\t\t-----DELETE FILE COMMAND-----\n\n");
	OutputConsole("-> Input file name: ");
	InputConsole(fileName, maxlen);
	Delete(fileName);
	Halt();
}
