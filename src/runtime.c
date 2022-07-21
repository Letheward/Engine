/* ==== Macros ==== */ 

#define string(s)            (String) {(u8*) s, sizeof(s) - 1}
#define array_string(s)      (String) {(u8*) s, sizeof(s)}
#define data_string(s)       (String) {(u8*) &s, sizeof(s)}
#define length_of(array)     (sizeof(array) / sizeof(array[0]))
#define array(Type, c_array) (Array(Type)) {c_array, length_of(c_array)}

#define Array(Type) Array_ ## Type
#define Define_Array(Type) \
typedef struct {           \
    Type* data;            \
    u64   count;           \
} Array(Type)              \




/* ==== Types ==== */ 

typedef unsigned char           u8;
typedef unsigned short int      u16;
typedef unsigned int            u32;
typedef unsigned long long int  u64;
typedef signed char             s8;
typedef signed short            s16;
typedef signed int              s32;
typedef signed long long int    s64;
typedef float                   f32;
typedef double                  f64;

typedef struct {
    u8* data;
    u64 count;
} String;

typedef struct {
    String base;
    u64    allocated;
} StringBuilder;

Define_Array(String);




/* ==== Constants ==== */

const f32 TAU   = 6.283185307179586476925286766559;
const f64 TAU64 = 6.283185307179586476925286766559;





/* ==== Temp Allocator ==== */

typedef struct {
    u8* data;
    u64 size;
    u64 allocated;
    u64 highest;
} ArenaBuffer;

struct {
    ArenaBuffer   temp_buffer;
    void*         (*alloc)(u64);
    Array(String) command_line_args;
    FILE*         log_file;
} runtime;

void* temp_alloc(u64 count) {

    ArenaBuffer* a = &runtime.temp_buffer;
    
    u64 current = a->allocated;
    u64 wanted  = current + count;
    
    assert(wanted < a->size);
    
    if (wanted > a->highest) a->highest = wanted; // check the highest here, maybe slow?
    a->allocated = wanted;

    return a->data + current;
}

void temp_free(u64 size) {
    runtime.temp_buffer.allocated -= size;
}

void temp_reset() {
    ArenaBuffer* a = &runtime.temp_buffer;
    a->allocated = 0;
    memset(a->data, 0, a->highest); // do we need this?
}

void temp_info() {
    ArenaBuffer* a = &runtime.temp_buffer;
    printf(
        "\nTemp Buffer Info:\n"
        "Data:      %p\n"
        "Size:      %lld\n"
        "Allocated: %lld\n"
        "Highest:   %lld\n\n",
        a->data, a->size, a->allocated, a->highest
    );
}




/* ==== Utils ==== */

// use printf() for quick debugging, use this for actual stuff that needs to log
void logprint(char* s, ...) {
    va_list args;
    va_start(args, s);
    vfprintf(runtime.log_file, s, args);
    va_end(args);
}

void error(char* s, ...) {
    va_list va;
    va_start(va, s);
    fprintf(runtime.log_file, "[Error] ");
    vfprintf(runtime.log_file, s, va);
    va_end(va);
    exit(1); 
}




/* ==== Timer ==== */

typedef struct {
    struct timespec start;
    struct timespec end;
    u8 which;
} PerformanceTimer;

PerformanceTimer timer;

void time_it() {
    if (timer.which == 0) {
        clock_gettime(CLOCK_MONOTONIC, &timer.start);
        timer.which = 1;
    } else {
        clock_gettime(CLOCK_MONOTONIC, &timer.end);
        printf("%fs\n", (timer.end.tv_sec - timer.start.tv_sec) + 1e-9 * (timer.end.tv_nsec - timer.start.tv_nsec));
        timer.which = 0;
    }
}





/* ==== StringBuilder: Basic ==== */

// todo: can only use heap allocator now, how to change allocator?
StringBuilder builder_init() {
    return (StringBuilder) {
        .base.data  = malloc(1024),
        .base.count = 0,
        .allocated  = 1024,
    };
}

void builder_free(StringBuilder* b) {
    free(b->base.data);
}

u64 builder_append(StringBuilder* b, String s) {
    
    u64 wanted = b->base.count + s.count;
    while (wanted > b->allocated) {
        b->base.data = realloc(b->base.data, b->allocated * 2);
        b->allocated *= 2;
    }

    for (u64 i = 0; i < s.count; i++)  b->base.data[b->base.count + i] = s.data[i];
    b->base.count = wanted;

    return s.count;
}





/* ==== String ==== */ 

void print(String s) {
    for (u64 i = 0; i < s.count; i++) {
        putchar(s.data[i]);
    }
}

String temp_print(char* s, ...) {
    
    va_list va, va2;
    va_start(va, s);
    va_copy(va2, va);
    
    u64 count  = vsnprintf(0, 0, s, va);
    String out = {temp_alloc(count + 1), count};
    vsnprintf((char*) out.data, count + 1, s, va2);

    va_end(va);
    va_end(va2);
    
    return out;
}

// naive search for now
String string_find(String a, String b) {
    
    if (!a.count || !b.count || !a.data || !b.data || (b.count > a.count)) return (String) {0};

    for (u64 i = 0; i < a.count - b.count + 1; i++) {
            
        for (u64 j = 0; j < b.count; j++) {
            if (a.data[i + j] != b.data[j]) goto next;
        }
        
        return (String) {a.data + i, a.count - i};
        next: continue;
    }
    
    return (String) {0};
}
