#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mapfile.h"
#include "debug.h"

#ifdef WIN32

#include <io.h>


/**
 * Opens a file and reads gets its size. 
 * @param[in] name name and path of the file to open.
 * @param[out] textlen size in bytes of the file.
 * @param[in] writefile flag indicating if the file shall be open with write aceess. 
 * @returns descriptor of the open file.
 */
static int fileOpen(char *name, Uint *textlen, BOOL writefile)
{
  int fd;
  struct stat buf;

  if((fd = _open(name,(writefile) ? O_RDWR : O_RDONLY)) == -1)
  {
     fprintf(stderr,"fileOpen: Cannot open \"%s\"",name);
     return -1;
  }
  if(fstat(fd,&buf) == -1)
  {
     fprintf(stderr, "file \"%s\": fstat(fd = %d) failed",name,fd);
     return -2;
  }
  *textlen = (Uint) buf.st_size;
  return fd;
}


/** 
 * Maps an open file into RAM memory.
 * @param[in] fd file descriptor of the file to map.
 * @param[in] offset start the map at this offset from the file begining. 
 * @param[in] len length of the section of the file to map.
 * @param[out] hndl win32 file handle.
 * @param[in] writemap flag indicating if it will be possible to write the file though the memory map. 
 * @returns a pointer to the memory map area.
 * @todo change return type to void *.
 */
static void * fileParts(int fd, Uint offset, Uint len, HANDLE *hndl, BOOL writemap) {
  void * ret;

  *hndl = CreateFileMapping (
			     (HANDLE)_get_osfhandle(fd),
			     NULL,
			     (writemap ? PAGE_READWRITE : PAGE_READONLY),
			     0,
			     0,
			     NULL);
    
  if (*hndl == NULL) {
    fprintf(stderr, "fileParts(fd = %d, left = %ld, len = %ld, %s) failed",fd,
            (long) offset, (long) len, writemap ? "writable map" : "readable map");
  }

  ret = (void *)MapViewOfFile (
			       *hndl, 
			       (writemap ? FILE_MAP_WRITE : FILE_MAP_READ),
			       0, 
			       offset, 
			       len);
  if (ret == NULL) {
    fprintf(stderr, "fileParts(fd = %d, left = %ld, len = %ld, %s) failed",fd,
            (long) offset, (long) len, writemap ? "writable map" : "readable map");
  }

  return ret;
}


/** 
 * Opens a file and maps it into memory.
 * @param[in] name name and path of the file to open.
 * @param[out] textlen size in bytes of the file.
 * @param[out] hndl win32 file handle.
 * @param[in] writefile flag indicating if the file shall be open with write aceess. 
 * @param[in] writemap flag indicating if it will be possible to write the file though the memory map. 
 * @returns a pointer to the memory map area.
 */
static void * genfile2String(char *name, Uint *textlen, HANDLE *hndl, BOOL writefile, BOOL writemap) {
  int fd;

  fd = fileOpen(name, textlen, writefile);
  if(fd < 0)
  {
    return NULL;
  }
  return fileParts(fd, 0, *textlen, hndl, writemap);
}


/**
 * @param[in] text pointer to the mapped file.
 * @param[in] hndl win32 file handle.
 */
void freetextspace(Uchar *text, HANDLE hndl) {
  if (!(UnmapViewOfFile(text))) {
    fprintf(stderr, "Unable to unmap file");
  }
  if (!CloseHandle(hndl)) {
    fprintf(stderr, "Unable to close handle");
  }
}


/**
 * @param[in] name name and path of the file to open.
 * @param[out] textlen size in bytes of the file.
 * @param[out] hndl win32 file handle.
 * @returns a pointer to the memory map area.
 */
void * file2String(char *name, Uint *textlen, HANDLE *hndl) {
  return genfile2String(name, textlen, hndl, False, False);
}


#else /* UNIX */

#include <sys/mman.h>


/**
 * Opens a file and reads gets its size. 
 * @param[in] name name and path of the file to open.
 * @param[out] textlen size in bytes of the file.
 * @param[in] writefile flag indicating if the file shall be open with write aceess. 
 * @returns descriptor of the open file.
 */
static int fileOpen(char *name, Uint *textlen, BOOL writefile)
{
  int fd;
  struct stat buf;

  if((fd = open(name,(writefile) ? O_RDWR : O_RDONLY)) == -1)
  {
     fprintf(stderr,"fileOpen: Cannot open \"%s\"",name);
     return -1;
  }
  if(fstat(fd,&buf) == -1)
  {
     fprintf(stderr, "file \"%s\": fstat(fd = %d) failed",name,fd);
     return -2;
  }
  *textlen = (Uint) buf.st_size;
  return fd;
}


/** 
 * Maps an open file into RAM memory.
 * @param[in] fd file descriptor of the file to map.
 * @param[in] offset start the map at this offset from the file begining. 
 * @param[in] len length of the section of the file to map.
 * @param[in] writemap flag indicating if it will be possible to write the file though the memory map. 
 * @returns a pointer to the memory map area.
 */
static void * fileParts(int fd, Uint offset, Uint len, BOOL writemap)
{
  void *addr;

  addr = mmap((void *) 0, (size_t) len, writemap ? (PROT_READ | PROT_WRITE) : PROT_READ, MAP_PRIVATE, fd, (off_t) offset);
  if(addr == (void *) MAP_FAILED)
  {
    fprintf(stderr, "fileParts(fd = %d, left = %ld, len = %ld, %s) failed",fd,
            (long) offset,(long) len,
            writemap ? "writable map" : "readable map");
    return NULL;
  } 
  return addr;
}


/** 
 * Opens a file and maps it into memory.
 * @param[in] name name and path of the file to open.
 * @param[out] textlen size in bytes of the file.
 * @param[in] writefile flag indicating if the file shall be open with write aceess. 
 * @param[in] writemap flag indicating if it will be possible to write the file though the memory map. 
 * @returns a pointer to the memory map area.
 */
static void * genfile2String(char *name, Uint *textlen, BOOL writefile, BOOL writemap) {
  int fd;

  fd = fileOpen(name, textlen, writefile);
  if(fd < 0)
  {
    return NULL;
  }
  return fileParts(fd, 0, *textlen, writemap);
}


/**
 * @param[in] text pointer to the mapped file.
 * @param[in] textlen size in bytes of the mapped file memory area. 
 */
void freetextspace(Uchar *text, Uint textlen) {
  if (munmap((void *) text, (size_t) textlen) == -1) {
    fprintf(stderr, "Unable to unmap file");
  }
}


/**
 * @param[in] name name and path of the file to open.
 * @param[out] textlen size in bytes of the file.
 * @returns a pointer to the memory map area.
 */
void * file2String(char *name, Uint *textlen) {
  return genfile2String(name, textlen, False, False);
}

#endif

