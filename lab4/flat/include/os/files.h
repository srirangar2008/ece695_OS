#ifndef __FILES_H__
#define __FILES_H__

#include "dfs.h"
#include "files_shared.h"
#include "synch.h"

Lock file_lock;
lock_t file_lock_handle;


//#define FILE_MAX_OPEN_FILES 15
int FileClose(int);
int FileOpen(char*, char*);
int FileWrite(int, char*, int);
int FileDelete(char*);
int FileRead(int, char*, int);
int FileSeek(int, int, int);


#endif
