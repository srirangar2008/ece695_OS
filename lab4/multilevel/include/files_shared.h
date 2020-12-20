#ifndef __FILES_SHARED__
#define __FILES_SHARED__

#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3

#define FILE_MAX_FILENAME_LENGTH 72

#define FILE_MAX_READWRITE_BYTES 4096
#define MAX_FILE_DESCRIPTORS 192

#define READ_ONLY 0x01
#define WRITE_ONLY 0x10
#define READ_WRITE 0x11

typedef struct file_descriptor {
  // STUDENT: put file descriptor info here
  char filename[FILE_MAX_FILENAME_LENGTH];
  int inodeHandle;
  int curPos;
  int endOfFileFlag;
  int mode;
  int inUse; //Used to determine if other open already by another process.
  int filesize;

} file_descriptor;

#define R 1
#define W 2
#define RW 3
#define OR 4
#define OW 2
#define OX 1
#define UR 32
#define UW 16
#define UX 8


#define FILE_FAIL -1
#define FILE_EOF -1
#define FILE_SUCCESS 1

#endif
