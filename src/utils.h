#pragma once

char *read_entire_file(char *filepath, s64 *length_pointer = NULL);
bool file_exists(char *filepath);

u64 round_to_next_power_of_2(u64 v);

s64 string_length(char *s);
char *copy_string(char *s);
bool strings_match(char *a, char *b);

bool is_end_of_line(char c);
bool is_space(char c);

u64 get_hash(u64 x);
u64 get_hash(char *str);

u64 get_time_nanoseconds();
