#ifndef IO_H__
#define IO_H__

char* io_open_and_read_file(char* path, size_t* sz, char* mode);

char* io_read_file(void* handle, size_t* sz, char* mode);

#endif