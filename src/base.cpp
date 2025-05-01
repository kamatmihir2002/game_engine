#include <cstdlib>
#include <cstdio>
#include <base.h>


char* base::io_open_and_read_file(char* path, size_t* sz, char* mode) {
    char* buf = NULL;
    FILE* f = fopen(path, mode);
    fseek(f, 0, SEEK_END);
    (*sz) = ftell(f);
    buf = (char*)malloc(((*sz) + 1) * sizeof(char));
    fseek(f, 0, SEEK_SET);
    fread(buf, (*sz), sizeof(char), f);
    fclose(f);
    return buf;
}

char* base::io_read_file(void* handle, size_t* sz, char* mode) {
    FILE* f = (FILE*)handle;
    char* buf = NULL;
    
    fseek(f, 0, SEEK_END);
    (*sz) = ftell(f);
    buf = (char*)malloc(((*sz) + 1) * sizeof(char));
    fseek(f, 0, SEEK_SET);
    fread(buf, (*sz), sizeof(char), f);
    fclose(f);
    return buf;

}
