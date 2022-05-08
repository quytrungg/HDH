#include "syscall.h"
#include "copyright.h"
#define maxlen 32

int main(){
	int srcId, destId, fileSize, i;
	char c, source[maxlen], dest[maxlen];
	
	OutputConsole("\n\t\t\t-----COPY FILE COMMAND-----\n\n");
	OutputConsole("-> Input src file: ");
	InputConsole(source, maxlen); // Goi ham ReadString de doc vao ten file nguon
	
	OutputConsole("-> Input dest file: ");
	InputConsole(dest, maxlen); // Goi ham ReadString de doc vao ten file dich
	srcId = Open(source, 1); // Goi ham Open de mo file nguon
	
	if (srcId != -1) //Kiem tra mo file thanh cong
	{
		//Tao file moi voi ten la chuoi luu trong "dest"
		destId = Create(dest);
		
		destId = Open(dest, 0); // Goi ham Open de mo file dich
		if (destId != -1) //Kiem tra mo file thanh cong
		{
			// Seek den cuoi file nguon de lay duoc do dai noi dung file nguon (fileSize)
			fileSize = Seek(-1, srcId);
			
			//Chuan bi sao chep
			Seek(0, srcId); // Seek den dau file nguon
			Seek(0, destId); // Seek den dau file dich
			
			// Vong lap chay tu dau file nguon den het file nguon
			for(i = 0; i < fileSize; i++) {
				Read(&c, 1, srcId); //Doc tung ki tu cua file nguon
				Write(&c, 1, destId); //Ghi vao file dich
			}
			OutputConsole("\n-> Copy file successfully!.\n\n");
			Close(destId); // Goi ham Close de dong file dich
		}
		else OutputConsole("-> Cannot create dest file\n\n");
		Close(srcId); // Goi ham Close de dong file nguon
	}
	else OutputConsole("->Error open file!");
	Halt();
	return 0;
}
