#include "main.h"

#ifdef PLATFORM_WINDOWS
#ifdef APIENTRY
#undef APIENTRY
#endif
#ifdef WINGDIAPI
#undef WINGDIAPI
#endif
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <stdio.h>

static FILE *log_file;

static bool temporary_storage_initted;
static Memory_Arena temporary_storage_arena;

int string_length(char *s) {
    if (!s) return 0;

    int length = 0;
    while (*s++) {
        length++;
    }
    return length;
}

char *copy_string(char *s) {
    if (!s) return 0;

    int len = string_length(s);
    char *result = new char[len + 1];
    memcpy(result, s, len + 1);
    return result;
}

bool strings_match(char *a, char *b) {
    if (a == b) return true;
    if (!a || !b) return false;

    while (*a && *b) {
        if (*a != *b) {
            return false;
        }

        a++;
        b++;
    }

    return *a == 0 && *b == 0;
}

bool is_end_of_line(char c) {
    bool result = ((c == '\n') ||
                   (c == '\v'));
    return result;
}

bool is_space(char c) {
    bool result = (is_end_of_line(c) ||
                   (c == '\v') ||
                   (c == '\t') ||
                   (c == ' '));
    return result;
}

char *find_character_from_right(char *s, char c) {
    if (!s) return NULL;

    char *end = s + string_length(s) - 1;
    while (end != s) {
        if (*end == c) return end;
        end--;
    }

    return NULL;
}

char *find_character_from_left(char *s, char c) {
    if (!s) return NULL;

    while (*s) {
        if (*s == c) return s;
        s++;
    }

    return NULL;
}

void init_logging(char *log_filepath) {
    log_file = fopen(log_filepath, "wb");
}

void shutdown_logging() {
    if (log_file) {
        fclose(log_file);
    }
}

void logprintf(char *fmt, ...) {
    char buf[4096];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    fprintf(stdout, "%s", buf);
    fflush(stdout);
    fprintf(log_file, "%s", buf);
    fflush(log_file);
}

// From https://www.bytesbeneath.com/p/the-arena-custom-memory-allocators


void ma_init(Memory_Arena *arena, s64 size) {
#ifdef PLATFORM_WINDOWS
    arena->data     = (u8 *)VirtualAlloc(0, size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
#endif
    arena->size     = size;
    arena->occupied = 0;
}

void *ma_alloc(Memory_Arena *arena, s64 size, bool zero_memory, s64 alignment) {
    size = (size + alignment - 1) & ~(alignment - 1);

    if (arena->occupied + size > arena->size) {
        Assert(!"Memory arena is full!");
        return 0;
    }
    
    void *result     = (void *)(arena->data + arena->occupied);
    arena->occupied += size;

    if (zero_memory) {
        memset(result, 0, size);
    }
    
    return result;
}

void ma_reset(Memory_Arena *arena) {
    arena->occupied = 0;
}

void init_temporary_storage(s64 size) {
    Assert(!temporary_storage_initted);

    ma_init(&temporary_storage_arena, size);
    
    temporary_storage_initted  = true;
}

void reset_temporary_storage() {
    ma_reset(&temporary_storage_arena);
}

bool is_temporary_storage_initialized() {
    return temporary_storage_initted;
}

s64 get_temporary_storage_mark() {
    return temporary_storage_arena.occupied;
}

void set_temporary_storage_mark(s64 mark) {
    Assert(mark >= 0);
    Assert(mark < temporary_storage_arena.size);
    temporary_storage_arena.occupied = mark;
}

void *talloc(s64 size, s64 alignment) {
    return ma_alloc(&temporary_storage_arena, size, alignment);
}

void clamp(int *v, int a, int b) {
    if (*v < a) *v = a;
    else if (*v > b) *v = b;
}

void clamp(float *v, float a, float b) {
    if (*v < a) *v = a;
    else if (*v > b) *v = b;    
}

u64 round_to_next_power_of_2(u64 v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

bool file_exists(char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) return false;
    fclose(file);
    return true;
}
