#include "main.h"

#include <stdio.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#ifdef APIENTRY
#undef APIENTRY
#endif
#include <Windows.h>
#endif

char *read_entire_file(char *filepath, s64 *length_pointer) {
    char *result = NULL;
    if (length_pointer) *length_pointer = 0;

    FILE *file = fopen(filepath, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        auto length = ftell(file);
        fseek(file, 0, SEEK_SET);

        result = new char[length + 1];
        auto num_read = fread(result, 1, length, file);
        fclose(file);

        result[num_read] = 0;
        if (length_pointer) *length_pointer = num_read;
    }
    return result;
}

bool file_exists(char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) return false;
    fclose(file);
    return true;
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

s64 string_length(char *s) {
    if (!s) return 0;

    s64 len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

char *copy_string(char *s) {
    if (!s) return NULL;

    s64 len = string_length(s);
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

char *trim_spaces(char *s) {
    s = trim_spaces_beginning(s);
    s = trim_spaces_trailing(s);
    return s;
}

char *trim_spaces_beginning(char *s) {
    if (!s) return NULL;

    while (is_space(*s)) {
        s++;
    }

    return s;
}

char *trim_spaces_trailing(char *s) {
    if (!s) return NULL;
    
    char *end = s + string_length(s) - 1;
    while (end > s && is_space(*end)) end--;

    end[1] = 0;

    return s;
}

char *consume_next_line(char **text_ptr) {
    char *t = *text_ptr;
    if (!*t) return NULL;

    char *s = t;

    while (*t && (*t != '\n') && (*t != '\r')) t++;

    char *end = t;
    if (*t) {
        end++;

        if (*t == '\r') {
            if (*end == '\n') ++end;
        }

        *t = '\0';
    }
    
    *text_ptr = end;
    
    return s;
}

bool starts_with(char *a, char *b) {
    if (a == b) return true;
    if (!a || !b) return false;

    s64 a_len = string_length(a);
    s64 b_len = string_length(b);
    if (a_len < b_len) return false;

    for (s64 i = 0; i < b_len; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }

    return true;
}

void split_line(char *s, char c, Array <char *> &strings) {
    if (!s) return;

    s = trim_spaces(s);

    char buf[4096];
    int buf_len = 0;
    while (*s && !is_end_of_line(*s)) {
        if (*s == c) {
            buf[buf_len] = 0;
            strings.add(copy_string(buf));
            buf_len = 0;
            s++;

            s = trim_spaces(s);
        }

        buf[buf_len++] = *s;
        s++;
    }

    if (buf_len > 0) {
        buf[buf_len] = 0;
        strings.add(copy_string(buf));
    }
}

char *find_character_from_left(char *s, char c) {
    if (!s) return NULL;

    while (1) {
        if (!*s) return NULL;
        if (*s == c) return s;
        s++;
    }

    return NULL;
}

bool is_end_of_line(char c) {
    bool result = ((c == '\n') ||
                   (c == '\r'));
    return result;
}

bool is_space(char c) {
    bool result = (is_end_of_line(c) ||
                   (c == '\v') ||
                   (c == '\t') ||
                   (c == ' '));
    return result;
}

u64 get_hash(u64 x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

u64 get_hash(char *str) {
    u64 hash = 5381;
    for (char *at = str; *at; at++) {
        hash = ((hash << 5) + hash) + *at;
    }
    return hash;
}

#ifdef _WIN32

static LARGE_INTEGER global_perf_freq;
static u64 nanoseconds_per_tick;

u64 get_time_nanoseconds() {
    if (!global_perf_freq.QuadPart) {
        QueryPerformanceFrequency(&global_perf_freq);
        nanoseconds_per_tick = 1000000000 / global_perf_freq.QuadPart;
    }

    LARGE_INTEGER perf_counter;
    QueryPerformanceCounter(&perf_counter);

    return perf_counter.QuadPart * nanoseconds_per_tick;
}

#endif
