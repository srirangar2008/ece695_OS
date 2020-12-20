#ifndef __FILES_H__
#define __FILES_H__

#include "dfs.h"
#include "files_shared.h"
#include "synch.h"

Lock file_lock;
lock_t file_lock_handle;

#define TRUE	1
#define FALSE   0
#define R 1
#define W 2
#define RW 3
#define OR 4
#define OW 2
#define OX 1
#define UR 32
#define UW 16
#define UX 8
#define OK 1
#define FILE 1
#define DIR 2

#define INVALID_FILE		-1
#define INVALID_INODE		-1
#define FILE_EXISTS		-2
#define DOES_NOT_EXIST		-3
#define DIRECTORY_EXISTS	-4
#define NOT_A_DIRECTORY		-5
#define NOT_A_FILE		-6
#define INVALID_PATH		-7
#define PERMISSION_DENIED	-8
#define DIRECTORY_NOT_EMPTY	-9


//#define FILE_MAX_OPEN_FILES 15
int FileClose(int);
int FileOpen(char*, char*);
int FileWrite(int, char*, int);
int FileDelete(char*);
int FileRead(int, char*, int);
int FileSeek(int, int, int);
int MkDir(char*, int);
int RmDir(char*);


#endif
