/*


Compile this file using compiler command line define, like:
~~~ sh
gcc win32.c -O3 -c -D win32_layer_implementation
~~~

Add `win32.o` or whatever (.obj) to your build script
~~~ sh
gcc test.c win32.o
~~~


*/




/* ==== Header ==== */

#define win32_u8  unsigned char
#define win32_u64 unsigned long long int

#define win32_bit64(Type, value) ((union {Type a; win32_u64 b;}) {.a = value}).b

#ifndef win32_layer_implementation

void   win32_print_all_matched_files(char* s);
char** win32_get_all_matched_filename_c_strings(char* s, win32_u64* count_out);

win32_u8* win32_load_file(char* s, win32_u64* count_out);
win32_u8  win32_save_file(char* s, win32_u8* data, win32_u64 count);

#endif





/* ==== Implementation ==== */

#ifdef win32_layer_implementation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>



void win32_print_find_data(WIN32_FIND_DATA* data) {

    win32_u64 uc = win32_bit64(FILETIME, data->ftCreationTime);
    win32_u64 ua = win32_bit64(FILETIME, data->ftLastAccessTime);
    win32_u64 uw = win32_bit64(FILETIME, data->ftLastWriteTime);
    
    SYSTEMTIME c; 
    SYSTEMTIME a; 
    SYSTEMTIME w; 

    FileTimeToSystemTime(&data->ftCreationTime,   &c);
    FileTimeToSystemTime(&data->ftLastAccessTime, &a);
    FileTimeToSystemTime(&data->ftLastWriteTime,  &w);
    
    // note: this is in UTC 
    printf(
        "Filename:    %s\n"
        "Size (high): %lu\n"
        "Size (low):  %lu\n"
        "Time (creation):       %5hd %2hd %3hd %4hd %3hd %3hd %4hd    (u64) %llu\n"
        "Time (last access):    %5hd %2hd %3hd %4hd %3hd %3hd %4hd    (u64) %llu\n"
        "Time (last modified):  %5hd %2hd %3hd %4hd %3hd %3hd %4hd    (u64) %llu\n\n",
        data->cFileName,
        data->nFileSizeHigh,
        data->nFileSizeLow,
        c.wYear, c.wMonth, c.wDay, c.wHour, c.wMinute, c.wSecond, c.wMilliseconds, uc,
        a.wYear, a.wMonth, a.wDay, a.wHour, a.wMinute, a.wSecond, a.wMilliseconds, ua,
        w.wYear, w.wMonth, w.wDay, w.wHour, w.wMinute, w.wSecond, w.wMilliseconds, uw
    );
}

void win32_print_all_matched_files(char* s) {

    WIN32_FIND_DATA find_data;
    HANDLE handle = FindFirstFile(s, &find_data);
    if (handle == INVALID_HANDLE_VALUE) return;

    win32_print_find_data(&find_data);
    
    while (1) {
        BOOL found = FindNextFile(handle, &find_data);
        if (!found) break;
        win32_print_find_data(&find_data);
    }
    
    FindClose(handle); 
}

char** win32_get_all_matched_filename_c_strings(char* s, win32_u64* count_out) {

    *count_out = 0;

    WIN32_FIND_DATA find_data;
    HANDLE handle = FindFirstFile(s, &find_data);
    if (handle == INVALID_HANDLE_VALUE) return NULL;
    
    win32_u64 count = 1;
    
    while (1) {
        BOOL found = FindNextFile(handle, &find_data);
        if (!found) break;
        count++;
    }

    *count_out = count;

    char** out = malloc(sizeof(char*) * count); // using malloc(), ehh...
    if (!out) goto fail;

    {
        handle = FindFirstFile(s, &find_data);

        win32_u64 acc  = 0;
        out[acc] = malloc(strlen(find_data.cFileName) + 1); // we assume this will succeed for now
        strcpy(out[acc], find_data.cFileName); 
        
        while (1) {
            BOOL found = FindNextFile(handle, &find_data);
            if (!found) break;
            acc++;
            out[acc] = malloc(strlen(find_data.cFileName) + 1); // we assume this will succeed for now
            strcpy(out[acc], find_data.cFileName); 
        }
    }

    FindClose(handle);
    return out;

    fail: 
    FindClose(handle);
    return NULL;
}


win32_u8* win32_load_file(char* s, win32_u64* count_out) {

    *count_out = 0;
    
    HANDLE handle = CreateFile(s, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);    
    if (handle == INVALID_HANDLE_VALUE) return NULL; // microsoft don't close the handle in their sample code
    
    win32_u64 count;
    {
        LARGE_INTEGER size;
        BOOL result = GetFileSizeEx(handle, &size);
        if (!result) goto fail;

        count = (win32_u64) size.QuadPart;
        *count_out = count;
    }
   
    win32_u8* out = malloc(count);
    if (!out) goto fail;
    
    {
        const win32_u64 chunk = 1024 * 1024 * 1024; // 1 GiB
        
        win32_u8* cursor  = out;
        win32_u64 to_read = count;
        
        DWORD read;
        while (to_read > chunk) {
            BOOL result = ReadFile(handle, cursor, chunk, &read, NULL); 
            if (!result || read != chunk) goto fail_but_allocated;
            cursor  += chunk;
            to_read -= chunk;
        }

        BOOL result = ReadFile(handle, cursor, to_read, &read, NULL); 
        if (!result || read != to_read) goto fail_but_allocated;
    }

    CloseHandle(handle);
    return out;

    fail_but_allocated: free(out); // fall-through

    fail: 
    CloseHandle(handle);
    return NULL;
}

// todo: validate that CREATE_ALWAYS does the right thing
win32_u8 win32_save_file(char* s, win32_u8* data, win32_u64 count) {

    HANDLE handle = CreateFile(s, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);    
    if (handle == INVALID_HANDLE_VALUE) return 0; 
    
    {
        const win32_u64 chunk = 1024 * 1024 * 1024; // 1 GiB
        
        win32_u8* cursor   = data;
        win32_u64 to_write = count;

        DWORD written;
        while (to_write > chunk) {
            BOOL result = WriteFile(handle, cursor, chunk, &written, NULL); 
            if (!result || written != chunk) goto fail;
            cursor   += chunk;
            to_write -= chunk;
        }

        BOOL result = WriteFile(handle, cursor, to_write, &written, NULL); 
        if (!result || written != to_write) goto fail;
    }
    
    CloseHandle(handle);
    return 1;
    
    fail: 
    CloseHandle(handle);
    return 0;
}


#endif


#undef win32_u8
#undef win32_u64

#undef win32_bit64


