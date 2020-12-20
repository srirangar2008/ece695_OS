#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"


// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.
Lock file_lock;
lock_t file_lock_handle;


file_descriptor openFileTable[MAX_FILE_DESCRIPTORS];

void PrintFileDescriptor(file_descriptor* fd);

// STUDENT: put your file-level functions here

int FileOpen(char* filename, char* mode)
{
    //To keep things simple, returning the handle as the inode handle.
    int i;
    int fileMode;
    int fileHandle;
    file_lock_handle = LockInit(&file_lock);

    
    
    dbprintf('r',"Entered the fileopnen function now.\n");
    if(dstrncmp(mode,"rw",dstrlen("rw")) == 0)
    {
        dbprintf('r',"If read_write The mode = ");
        dbprintf('r',mode);
        fileMode = READ_WRITE;
    }
    else if(dstrncmp(mode,"r",dstrlen("r")) == 0)
    {
        dbprintf('r',"If read_only The mode = ");
        dbprintf('r',mode);

        fileMode = READ_ONLY;
    }
    else if(dstrncmp(mode,"w",dstrlen("w")) == 0)
    {
        dbprintf('r',"If write only The mode = ");
        dbprintf('r',mode);
        fileMode = WRITE_ONLY;
    }
    
    else
    {
        dbprintf('r',"OPening the file in read only mode by default.\n");
        fileMode = READ_ONLY;
    }
    dbprintf('r',"fileMode = 0x%x\n",fileMode);
    
    dbprintf('r',"Opening the Inode now.\n");
    fileHandle = DfsInodeOpen(filename);
    
    if(openFileTable[fileHandle].inUse == 1)
    {
        printf("The file is laready open by another process. Cannot open. Open Failed.\n");
        return FILE_FAIL;
    }
    
    LockAcquire(&file_lock);
    openFileTable[fileHandle].inUse = 1;
    openFileTable[fileHandle].inodeHandle = fileHandle;
    LockRelease(&file_lock);
    dstrncpy(openFileTable[fileHandle].filename,filename,dstrlen(filename));
    openFileTable[fileHandle].curPos = 0;
    openFileTable[fileHandle].mode = fileMode;
    openFileTable[fileHandle].endOfFileFlag = -1;
    openFileTable[fileHandle].filesize = 0;

    dbprintf('r',"The file with filename = ");
    dbprintf('r',filename);
    dbprintf('r'," opened with handle = %d\n",fileHandle);
    PrintFileDescriptor(&openFileTable[fileHandle]);
    return fileHandle;
}

int FileClose(int handle)
{
    if(openFileTable[handle].inUse == 0)
    {
        printf("ERROR : File not opened.\n");
        return FILE_FAIL;
    }

    dbprintf('r',"Entered the fileclose function.\n");
    LockAcquire(&file_lock);
    openFileTable[handle].inUse = 0;
    //openFileTable[handle].inodeHandle = 0;
    LockRelease(&file_lock);
    //openFileTable[handle].curPos = 0;
    //openFileTable[handle].mode = 0;
    //openFileTable[handle].endOfFileFlag = 0;
    //openFileTable[handle].filesize = 0;
    //bzero(openFileTable[handle].filename,FILE_MAX_FILENAME_LENGTH);

    dbprintf('r',"FileClose() : Closing the fd = %d\n",handle);

    return FILE_SUCCESS;
}

void PrintFileDescriptor(file_descriptor* fd)
{
    dbprintf('r',"PrintFileDescriptor() : Filename = ");
    dbprintf('r',fd->filename);
    dbprintf('r',"\n");
    dbprintf('r',"PrintFileDescriptor() : Inode Handle = %d\n",fd->inodeHandle);
    dbprintf('r',"PrintFileDescriptor() : Inuse = %d\n", fd->inUse);
    dbprintf('r',"PrintFileDescriptor() : Current Pos = %d\n",fd->curPos);
    dbprintf('r',"PrintFileDescriptor() : End of file flag = %d\n",fd->endOfFileFlag);
    dbprintf('r',"PrintFileDescriptor() : Filemode = 0x%02x\n",fd->mode);
    dbprintf('r',"PrintFileDescriptor() : Filesize = %d\n",fd->filesize);
}

int FileWrite(int handle, char* buffer, int len)
{
    int bytesWritten;
    if(len > 4096)
    {
        printf("Write Error : Max bytes to be written at once is 4096.\n");
        return FILE_FAIL;
    }
    dbprintf('r',"Entered the file write function now.\n");
    if(openFileTable[handle].inUse == 0)
    {
        printf("FileWrite() : ERROR : File not open.\n");
        return FILE_FAIL;
    }
    if((openFileTable[handle].mode == READ_ONLY))
    {
        printf("Read Only File. Cannot be written.\n");
        return FILE_FAIL;
    }
    else
    {
        bytesWritten = DfsInodeWriteBytes(openFileTable[handle].inodeHandle,buffer,openFileTable[handle].curPos,len);
        openFileTable[handle].curPos += bytesWritten;  
          
    }
    openFileTable[handle].filesize = DfsInodeFilesize(openFileTable[handle].inodeHandle);
    //printf("filesize = %d\n",openFileTable[handle].filesize);
    PrintFileDescriptor(&openFileTable[handle]);
    return bytesWritten;
}


int FileDelete(char* filename)
{
    
    int i = 0;
    PrintFileDescriptor(&openFileTable[2]);
    for(i = 0; i < MAX_INODES; i++)
    {
        
        if(dstrncmp(openFileTable[i].filename,filename,dstrlen(filename)) == 0)
        {
            dbprintf('r',"FileDeelet() : filename = ");
            dbprintf('r',filename);
            dbprintf('r',"\n");
            dbprintf('r',"The filename pointed by the fd = ");
            dbprintf('r',openFileTable[i].filename);
            dbprintf('r',"\n");
            if(openFileTable[i].inUse == 1)
            {
                printf("The file is in use. Cannot delete.");
                return FILE_FAIL;
            }
            else
            {
                dbprintf('r',"FileDelete() : handle = %d\n",i);
                PrintFileDescriptor(&openFileTable[i]);
                
                DfsInodeDelete(openFileTable[i].inodeHandle);
                 openFileTable[i].curPos = 0;
                 openFileTable[i].mode = 0;
                 openFileTable[i].endOfFileFlag = 0;
                 bzero(openFileTable[i].filename,FILE_MAX_FILENAME_LENGTH);
                break;
            }
            
            
        }
        
        
    }
    if( i== MAX_INODES)
    {
        dbprintf('r',"There is not file by name ");
        dbprintf('r',filename);
        dbprintf('r',"\n");
        return FILE_FAIL;
    }
    return FILE_SUCCESS;
}
int FileRead(int handle, char* buffer, int len)
{
    int bytesRead;
    if(len > 4096)
    {
        printf("Read Error : The max bytes that can be read at once = 4096 bytes.\n");
        return FILE_FAIL;
    }
    dbprintf('r',"FileRead() fundtion\n Handle = %d\n",handle);
    PrintFileDescriptor(&openFileTable[handle]);
    if(openFileTable[handle].inUse == 0)
    {
        printf("FileRead() : ERROR : The file is not open. Cannot Read.\n");
        return FILE_FAIL;
    }

    if(openFileTable[handle].endOfFileFlag == 1)
    {
        printf("FileREad() : The seek is at the end of the file.\n");
        return FILE_FAIL;
    }

    bytesRead = DfsInodeReadBytes(openFileTable[handle].inodeHandle,buffer,openFileTable[handle].curPos,len);
    openFileTable[handle].curPos += bytesRead;
    if(openFileTable[handle].curPos >= openFileTable[handle].filesize)
    {
        dbprintf('r',"EOF reached.\n");
        openFileTable[handle].endOfFileFlag = 1;
    }
    return bytesRead;
}

int FileSeek(int handle, int size, int pos)
{
    if(openFileTable[handle].inUse == 0)
    {
        printf("FileSeek() : The file is not open.\n");
        return FILE_FAIL;
    }
    if(pos == FILE_SEEK_SET)
    {
        openFileTable[handle].curPos = size;
    }
    else if(pos == FILE_SEEK_CUR)
    {
        openFileTable[handle].curPos += size;
    }
    else if(pos == FILE_SEEK_END)
    {
        openFileTable[handle].curPos = DfsInodeFilesize(openFileTable[handle].inodeHandle) + size;
        //printf("cupPos = %d\n",openFileTable[handle].curPos);
    }
    else
    {
        printf("FileSeek() : Error : Unknown seek option.\n");
        return FILE_FAIL;
    }
    openFileTable[handle].endOfFileFlag = -1;

    return openFileTable[handle].curPos;
}

