// todo: linux and mac




/* ==== File System ==== */

void print_all_matched_files(char* s) {
    
    #ifdef OS_WINDOWS
    win32_print_all_matched_files(s);
    #endif

}

char** get_all_matched_filename_c_strings(char* s, u64* count_out) {
    
    #ifdef OS_WINDOWS
    return win32_get_all_matched_filename_c_strings(s, count_out);
    #endif

}

void free_filename_c_strings(char** names, u64 count) {
    for (u64 i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);
}





/* ==== Load and Save ==== */

String load_file(char* path) {

    FILE* f = fopen(path, "rb");
    if (!f) error("Cannot load %s\n", path); 

    u8* data;
    u64 count;

    fseek(f, 0, SEEK_END);
    count = ftell(f);
    fseek(f, 0, SEEK_SET);

    data = runtime.alloc(count);
    fread(data, 1, count, f);
    fclose(f);

    return (String) {data, count};
}

char* load_file_as_c_string(char* path) {

    FILE* f = fopen(path, "rb");
    if (!f) error("Cannot open file %s\n", path); 

    fseek(f, 0, SEEK_END);
    u64   length   = ftell(f);
    char* buffer   = runtime.alloc(length + 1);
    buffer[length] = '\0';
    fseek(f, 0, SEEK_SET);

    fread(buffer, 1, length, f);
    fclose(f);

    return buffer;
}

void save_file(String in, char* path) {

    FILE* f = fopen(path, "wb");
    if (!f) error("Cannot open file %s\n", path); 

    fwrite(in.data, sizeof(u8), in.count, f);
    fflush(f);
    fclose(f);
}
