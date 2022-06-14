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

#define win32_u64 unsigned long long int
#define win32_bit64(Type, value) ((union {Type a; win32_u64 b;}) {.a = value}).b

#ifndef win32_layer_implementation

void   win32_print_all_matched_files(char* s);
char** win32_get_all_matched_filename_c_strings(char* s, win32_u64* count_out);

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
    {
        handle = FindFirstFile(s, &find_data);

        win32_u64 acc  = 0;
        out[acc] = malloc(strlen(find_data.cFileName) + 1); // ehh....
        strcpy(out[acc], find_data.cFileName); 
        
        while (1) {
            BOOL found = FindNextFile(handle, &find_data);
            if (!found) break;
            acc++;
            out[acc] = malloc(strlen(find_data.cFileName) + 1); // ehh....
            strcpy(out[acc], find_data.cFileName); 
        }
    }

    FindClose(handle);

    return out;
}


#endif

#undef win32_bit64
#undef win32_u64


