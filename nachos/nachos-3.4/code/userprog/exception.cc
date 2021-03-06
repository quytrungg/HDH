// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
//#include "synchcons.h"
#include "syscall.h"
#include <unistd.h>
#define ThreadsSize 10
extern void StartProcess_2(int id);
extern void StartProcess(char *filename);

//SynchConsole* gSynchConsole;

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void IncreasePC(){
	int counter = machine->ReadRegister(PCReg);
   	machine->WriteRegister(PrevPCReg, counter);
    	counter = machine->ReadRegister(NextPCReg);
    	machine->WriteRegister(PCReg, counter);
   	machine->WriteRegister(NextPCReg, counter + 4);
}

// Input: - User space address (int)
// Limit of buffer (int)
// Output:- Buffer (char*)
// Purpose: Copy buffer from User memory space to System memory space
char* User2System(int virtAddr,int limit){
	int i;// index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit +1];//need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf,0,limit+1);
	//printf("\n Filename u2s:");
	for (i = 0 ; i < limit ;i++){
		machine->ReadMem(virtAddr+i,1,&oneChar);
		kernelBuf[i] = (char)oneChar;
		//printf("%c",kernelBuf[i]);
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

// Input: - User space address (int)
// Limit of buffer (int)
// Buffer (char[])
// Output:- Number of bytes copied (int)
// Purpose: Copy buffer from System memory space to User memory space
int System2User(int virtAddr,int len,char* buffer){
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0 ;
	do{
		oneChar= (int) buffer[i];
		machine->WriteMem(virtAddr+i,1,oneChar);
		i ++;
	}while(i < len && oneChar != 0);
	return i;
}

void ExceptionHandler(ExceptionType which){
    int type = machine->ReadRegister(2);

	switch (which) {
		case NoException:
			return;
		case PageFaultException:
			DEBUG('a', "\nNo valid translation found");
			printf("\n\nNo valid translation found");
			interrupt->Halt();
			break;
		case ReadOnlyException:
			DEBUG('a', "\nWrite attempted to page marked read-only");
			printf("\n\nWrite attempted to page marked read-only");
			interrupt->Halt();
			break;
		case BusErrorException:
			DEBUG('a', "\nTranslation resulted in an invalid physical address");
			printf("\n\nTranslation resulted in an invalid physical address");
			interrupt->Halt();
			break;
		case AddressErrorException:
			DEBUG('a', "\nUnaligned reference or one that was beyond the end of the address space");
			printf("\n\nUnaligned reference or one that was beyond the end of the address space");
			interrupt->Halt();
			break;
		case OverflowException:
			DEBUG('a', "\nInteger overflow in add or sub.");
			printf("\n\nInteger overflow in add or sub.");
			interrupt->Halt();
			break;
		case IllegalInstrException:
			DEBUG('a', "\nUnimplemented or reserved instr.");
			printf("\n\nUnimplemented or reserved instr.");
			interrupt->Halt();
			break;
		case NumExceptionTypes:
			DEBUG('a', "\n Number exception types");
			printf("\n\n Number exception types");
			interrupt->Halt();
			break;
		case SyscallException:
			switch (type){
				case SC_Halt:
					DEBUG('a', "\n Shutdown, initiated by user program.");
					printf ("\n\n Shutdown, initiated by user program.");
					interrupt->Halt();
					break;
				case SC_Create:
				{
					int virtAddr;
					char* filename;
					DEBUG('a',"\n SC_Create call ...");
					DEBUG('a',"\n Reading virtual address of filename");
					virtAddr = machine->ReadRegister(4);
					DEBUG ('a',"\n Reading filename.");
					// MaxFileLength l?? = 32
					filename = User2System(virtAddr,MaxFileLength+1);
					if (strlen(filename) == 0){
						printf("\n Not enough memory in system");
						DEBUG('a',"\n Not enough memory in system");
						machine->WriteRegister(2,-1);
						IncreasePC();
						//delete filename;
						return;
					}

					if (filename == NULL){
						printf("\n Not enough memory in system");
						DEBUG('a',"\n Not enough memory in system");
						machine->WriteRegister(2,-1);
						IncreasePC();
						delete filename;
						return;
					}
					DEBUG('a',"\n Finish reading filename.");
					printf("\nNew file created: %s", filename);
					//DEBUG('a',"\n File name : '"<<filename<<"'");
					// Create file with size = 0
					// D??ng ?????i t?????ng fileSystem c???a l???p OpenFile ????? t???o file,
					// vi???c t???o file n??y l?? s??? d???ng c??c th??? t???c t???o file c???a h??? ??i???u
					// h??nh Linux, ch??ng ta kh??ng qu???n ly tr???c ti???p c??c block tr??n
					// ????a c???ng c???p ph??t cho file, vi???c qu???n ly c??c block c???a file
					// tr??n ??? ????a l?? m???t ????? ??n kh??c
					if (!fileSystem->Create(filename,0)){
						printf("\n Error create file '%s'",filename);
						machine->WriteRegister(2,-1);
						IncreasePC();
						delete filename;
						return;
					}
					machine->WriteRegister(2,0); // tr??? v??? cho ch????ng tr??nh ng?????i d??ng th??nh c??ng
					IncreasePC();
					delete filename;
					break;
				}
				case SC_Open:
				{
					// Output: Tra ve OpenFileID neu thanh cong, -1 neu loi
					//OpenFileID Open(char *name, int type)
					int virtAddr = machine->ReadRegister(4);
					int type = machine->ReadRegister(5);
					char* filename;
					filename = User2System(virtAddr, MaxFileLength);
					int slot = fileSystem->findEmptyPosition();
					//printf("%d\n", type);
					if ((fileSystem->opfile[slot] = fileSystem->Open(filename, type)) == NULL){
						printf("%s: File not exist!\n", filename);
						interrupt->Halt();
					}
					if (slot != -1){
						printf("\nMo file thanh cong: %s", filename);
						printf("\nSlot: %d", slot);
						if (type == 0 || type == 1){
							if ((fileSystem->opfile[slot] = fileSystem->Open(filename, type)) != NULL) //Mo file thanh cong
							{
								machine->WriteRegister(2, slot); //tra ve OpenFileID
								IncreasePC();
							}
						}		
						else if (type == 2) //input file
						{
							machine->WriteRegister(2, 0);
							IncreasePC();
						}
						else //output file
						{
							machine->WriteRegister(2, 1);
							IncreasePC();
						}
						delete[] filename;
						break;
					}
					machine->WriteRegister(2, -1); //Khong mo duoc file return -1
					IncreasePC();
					delete[] filename;
					break;
				}
				case SC_Close:
				{
					// void Close(OpenFileId id);
					// Output: 0: thanh cong, -1 that bai
					int fileid = machine->ReadRegister(4);
					if (fileid >= 0 && fileid <= 9)
					{
						if (fileSystem->opfile[fileid]) //neu mo file thanh cong
						{
							printf("\nClose file");
							delete fileSystem->opfile[fileid]; //Xoa vung nho luu tru file
							fileSystem->opfile[fileid] = NULL; //Gan vung nho NULL
							machine->WriteRegister(2, 0);
							IncreasePC();
							break;
						}
					}
					//printf("\nClose file");
					machine->WriteRegister(2, -1);
					IncreasePC();
					break;
				}

				case SC_Read:
				{
					//int Read(char *buffer, int size, OpenFileId id);
					// Output: -1: Loi, So byte read thuc su: Thanh cong, -2: Thanh cong
					int virtAddr = machine->ReadRegister(4); // Lay dia chi cua tham so buffer tu thanh ghi so 4
					int charcount = machine->ReadRegister(5); // Lay charcount tu thanh ghi so 5
					int id = machine->ReadRegister(6); // Lay id cua file tu thanh ghi so 6 
					int OldPos, NewPos;
					char *buffer;
					// Check id in file board
					if (id < 0 || id > 9)
					{
						printf("\nFile board description out of range!");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					if (fileSystem->opfile[id] == NULL)
					{
						printf("\nFile not exist!");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					if (fileSystem->opfile[id]->type == 3) // Xet truong hop doc file stdout (type quy uoc la 3) thi tra ve -1
					{
						printf("\nCannot read output file");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					OldPos = fileSystem->opfile[id]->getCurPos(); // Kiem tra thanh cong thi lay vi tri OldPos
					buffer = User2System(virtAddr, charcount); // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
					// Xet truong hop doc file stdin (type quy uoc la 2)
					if (fileSystem->opfile[id]->type == 2)
					{
						// Su dung ham Read cua lop SynchConsole de tra ve so byte thuc su doc duoc
						int size = gSynchConsole->Read(buffer, charcount); 
						System2User(virtAddr, size, buffer); // Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer co do dai la so byte thuc su
						machine->WriteRegister(2, size); // Tra ve so byte thuc su doc duoc
						delete[] buffer;
						IncreasePC();
						return;
					}
					// Xet truong hop doc file binh thuong thi tra ve so byte thuc su
					if ((fileSystem->opfile[id]->Read(buffer, charcount)) > 0){
						// So byte thuc su = NewPos - OldPos
						NewPos = fileSystem->opfile[id]->getCurPos();
						// Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer co do dai la so byte thuc su 
						System2User(virtAddr, NewPos - OldPos, buffer); 
						machine->WriteRegister(2, NewPos - OldPos);
						//printf("%s", buffer);
						IncreasePC();
					}
					else{
						// Truong hop con lai la doc file co noi dung la NULL tra ve -2
						printf("\nEmpty file");
						machine->WriteRegister(2, -2);
						IncreasePC();
					}
					delete[] buffer;
					//IncreasePC();
					return;
				}
				case SC_Write:
				{
					//void Write(char *buffer, int size, OpenFileId id);
					// Input: buffer(char*), so ky tu(int), id cua file(OpenFileID)
					// Output: -1: Loi, So byte write thuc su: Thanh cong, -2: Thanh cong
					int virtAddr = machine->ReadRegister(4); // Lay dia chi cua tham so buffer tu thanh ghi so 4
					int charcount = machine->ReadRegister(5); // Lay charcount tu thanh ghi so 5
					int id = machine->ReadRegister(6); // Lay id cua file tu thanh ghi so 6
					int OldPos, NewPos;
					char *buffer;
					if (id < 0 || id > 9)
					{
						printf("\nFile board description out of range!");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					// Kiem tra file co ton tai khong
					if (fileSystem->opfile[id] == NULL)
					{
						printf("\nFile not exist!");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					// Xet truong hop ghi file only read (type quy uoc la 1) hoac file stdin (type quy uoc la 2) thi tra ve -1
					if (fileSystem->opfile[id]->type == 1 || fileSystem->opfile[id]->type == 2)
					{
						printf("\nCannot write output file");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					OldPos = fileSystem->opfile[id]->getCurPos(); // Kiem tra thanh cong thi lay vi tri OldPos
					buffer = User2System(virtAddr, charcount);  // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
					// Xet truong hop ghi file read & write (type quy uoc la 0) thi tra ve so byte thuc su
					if (fileSystem->opfile[id]->type == 0)
					{
						if ((fileSystem->opfile[id]->Write(buffer, charcount)) > 0)
						{
							// So byte thuc su = NewPos - OldPos
							NewPos = fileSystem->opfile[id]->getCurPos();
							machine->WriteRegister(2, NewPos - OldPos);
							delete[] buffer;
							IncreasePC();
							return;
						}
					}
					if (fileSystem->opfile[id]->type == 3) // Xet truong hop con lai ghi file stdout (type quy uoc la 3)
					{
						int i = 0;
						while (buffer[i] != 0 && buffer[i] != '\n') // Vong lap de write den khi gap ky tu '\n'
						{
							gSynchConsole->Write(buffer + i, 1); // Su dung ham Write cua lop SynchConsole 
							i++;
						}
						buffer[i] = '\n';
						gSynchConsole->Write(buffer + i, 1); // Write ky tu '\n'
						machine->WriteRegister(2, i - 1); // Tra ve so byte thuc su write duoc
						delete[] buffer;
						IncreasePC();
						return;
					}
				}

				case SC_Seek:
				{
					//int Seek(int pos, OpenFileId id);
					// Input: Vi tri(int), id cua file(OpenFileID)
					// Output: -1: Loi, Vi tri thuc su: Thanh cong
					int pos = machine->ReadRegister(4);
					int id = machine->ReadRegister(5);
					if (id < 0 || id > 9){
						printf("\nKhong the seek vi id nam ngoai bang mo ta file.");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					// Kiem tra file co ton tai khong
					if (fileSystem->opfile[id] == NULL){
						printf("\nKhong the seek vi file nay khong ton tai.");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					// Kiem tra co goi Seek tren console khong
					if (id == 0 || id == 2){
						printf("\nKhong the seek tren file console.");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					// Neu pos = -1 thi gan pos = Length nguoc lai thi giu nguyen pos
					pos = ((pos == -1) ? fileSystem->opfile[id]->Length() : pos);
					//printf("\n%d %d %d", pos, id, fileSystem->opfile[id]->Length());
					if (pos > fileSystem->opfile[id]->Length() || pos < 0) // Kiem tra lai vi tri pos co hop le khong
					{
						printf("\nKhong the seek file den vi tri nay.");
						machine->WriteRegister(2, -1);
						IncreasePC();
					}
					else{
						// Neu hop le thi tra ve vi tri di chuyen thuc su trong file
						fileSystem->opfile[id]->Seek(pos);
						printf("\nVi tri: %d",pos);
						machine->WriteRegister(2, pos);
						IncreasePC();
					}
					//IncreasePC();
					return;
				}
				case SC_InputConsole:
				{
					// void InputConsole(char buffer[], int length);
					// Input: Buffer(char*), do dai toi da cua chuoi nhap vao(int)
					int virtAddr = machine->ReadRegister(4);
					int length = machine->ReadRegister(5);
					int len = 0, i;
					char* buffer = User2System(virtAddr, length);
					fgets(buffer, length, stdin);
					//printf("%s", buffer);
					while(buffer[len] != NULL and buffer[len+1] != NULL) len++;
					//printf("%d\n", len);
					//printf("%s", newbuf);
					//gSynchConsole->Read(buffer, length); // Goi ham Read cua SynchConsole de doc chuoi
					System2User(virtAddr, len, buffer); // Copy chuoi tu vung nho System Space sang vung nho User Space
					delete[] buffer; 
					IncreasePC(); 
					return;
				}
				case SC_OutputConsole:
				{
					// void OutputConsole(char buffer[]);
					// Input: Buffer(char*)
					int virtAddr = machine->ReadRegister(4);
					char* buffer = User2System(virtAddr, 256); 
					int length = 0;
					while (buffer[length] != '\0') length++; // Dem do dai that cua chuoi
					printf("%s", buffer);
					//gSynchConsole->Write(buffer, length+1); // Goi ham Write cua SynchConsole de in chuoi
					delete[] buffer; 
					IncreasePC(); 
					break;
				}
				case SC_InputChar:
				{
					//Output: Duy nhat 1 ky tu (char)
					int maxBytes = 255;
					char* buffer = new char[255];
					int numBytes = gSynchConsole->Read(buffer, maxBytes);
					if(numBytes > 1) //Neu nhap nhieu hon 1 ky tu thi khong hop le
					{
						printf("Chi duoc nhap duy nhat 1 ky tu!");
						DEBUG('a', "\nERROR: Chi duoc nhap duy nhat 1 ky tu!");
						machine->WriteRegister(2, 0);
					}
					else if(numBytes == 0) //Ky tu rong
					{
						printf("Ky tu rong!");
						DEBUG('a', "\nERROR: Ky tu rong!");
						machine->WriteRegister(2, 0);
					}
					else{
						//Chuoi vua lay co dung 1 ky tu, lay ky tu o index = 0, return vao thanh ghi R2
						char c = buffer[0];
						machine->WriteRegister(2, c);
					}

					delete buffer;
					IncreasePC(); // error system
					break;
				}

				case SC_OutputChar:
				{
					// Input: Ki tu(char)
					// Output: Ki tu(char)
					char c = (char) machine->ReadRegister(4); // Doc ki tu tu thanh ghi r4
					printf("%c", c);
					//gSynchConsole->Write(&c, 1); // In ky tu tu bien c, 1 byte
					IncreasePC();
					break;

				}
				case SC_Delete:
				{
					// int Delete(char* name);
					int virtAddr = machine->ReadRegister(4);
					char* fileName = User2System(virtAddr, MaxFileLength + 1);
					//printf("test");	
					
					if(fileSystem->isOpen(fileName)) {
						printf("File is opened. Can't delete\n");
						machine->WriteRegister(2, -1);
						
					}
					else {
						if(fileSystem->Remove(fileName)) {
							machine->WriteRegister(2, 0);
							printf("File deleted\n");
						}
						else {
							printf("File is not existed\n");
							machine->WriteRegister(2, -1);
						}
					}
					delete[] fileName;
					IncreasePC();

					break;
				}

				case SC_Exec:
				{
					// SpaceId Exec(char *name);
					int virtAddr = machine->ReadRegister(4);
					char* filename = User2System(virtAddr, MaxFileLength);

					if(filename == NULL)
					{
						printf("He thong khong du bo nho\n");
						machine->WriteRegister(2, -1);
						break;
					}

					

					//delete fileSystem->table[freeSlot];

					// T??m ?? c??n tr???ng c???a threads
					int threadID = -1;
					for(int i = 0; i < ThreadsSize; i++)
					{
						if(mythreads[i] == NULL)
						{
							threadID = i;
							break;
						}
					}
					//printf("test1\n");
					if(threadID != -1)
					{
						mythreads[threadID] = new Thread(filename);
						if(mythreads[threadID] == NULL) printf("NULL\n");
						//printf("test2\n");
						mythreads[threadID]->Fork(StartProcess_2, threadID);
					}
					IncreasePC();
					machine->WriteRegister(2, threadID);
					delete[] filename;
					break;
				}
				case SC_CreateSemaphore:
				{
					// int CreateSemaphore(char* name, int semval).
					int virtAddr = machine->ReadRegister(4);
					int semval = machine->ReadRegister(5);

					char *name = User2System(virtAddr, MaxFileLength + 1);
					if(name == NULL)
					{
						DEBUG('a', "\n Not enough memory in System");
						printf("\n Not enough memory in System");
						machine->WriteRegister(2, -1);
						delete[] name;
						IncreasePC();
						return;
					}
					int res;
					//int res = semTab->Create(name, semval);

					if(res == -1)
					{
						DEBUG('a', "\n Khong the khoi tao semaphore");
						printf("\n Khong the khoi tao semaphore");
						machine->WriteRegister(2, -1);
						delete[] name;
						IncreasePC();
						return;				
					}
			
					delete[] name;
					machine->WriteRegister(2, res);
					IncreasePC();
					return;
				}

				case SC_Sleep:
				{
					// void Sleep(int time);
					int s = machine->ReadRegister(4);

					semjoin->V();	//down
					semexit->P();	//up
					semjoin->P();	//down
					semexit->V();	//up
				
					//sleep(s);
					machine->WriteRegister(2, s);
					break;
				}
				case SC_Exit:
				{
					//void Exit(int status);
					int exitStatus = machine->ReadRegister(4);

               				if (exitStatus != 0) {
                    				IncreasePC();
                    				break;
                			}

                			currentThread->FreeSpace();
                			currentThread->Finish();
                			IncreasePC();
               				break;
				}
				case SC_P:
				{
					//addrLock->P();
					IncreasePC();
					break;
				}
				default:
					printf("\n Unexpected user mode exception (%d %d)", which, type);
					interrupt->Halt();
			}
	}
}
/*
void StartProcess_2(int id)
{
    char* filename = mythreads[id]->getName();
	printf("%s\n", filename);
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
    space = new AddrSpace(executable);    
    currentThread->space = space;

    delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}
*/
