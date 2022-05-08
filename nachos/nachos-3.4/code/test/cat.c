#include "syscall.h"
#include "copyright.h"
#define maxlen 32

int main(){
	int openFileId, fileSize;
	char c, fileName[maxlen];
	int i;
	OutputConsole("\n\t\t\t-----CAT FILE COMMAND-----\n\n");
	OutputConsole("-> Input file name: ");
	InputConsole(fileName, maxlen);
	openFileId = Open(fileName, 0); // Goi ham Open de mo file 
	if (openFileId != -1) //Kiem tra Open co loi khong
	{
		//Seek den cuoi file de lay duoc do dai noi dung (fileSize)
		fileSize = Seek(-1, openFileId);
		// Seek den dau tap tin de tien hanh Read
		Seek(0, openFileId);
		OutputConsole("\n-> Cat file: \n");
		//OutputChar('c');
		for (i = 0; i < fileSize; i++) // Cho vong lap chay tu 0 - fileSize
		{
			Read(&c, 1, openFileId); // Goi ham Read de doc tung ki tu noi dung file
			OutputChar(c); // Goi ham PrintChar de in tung ki tu ra man hinh
		}
		Close(openFileId); // Goi ham Close de dong file
	}
	else OutputConsole("-> Cannot open file\n\n");
	Halt();
	return 0;
}
