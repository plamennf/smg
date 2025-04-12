#pragma once

char *read_entire_file(char *filepath, s64 *length_pointer = NULL);
bool file_exists(char *filepath);

u64 round_to_next_power_of_2(u64 v);

s64 string_length(char *s);
char *copy_string(char *s);
bool strings_match(char *a, char *b);

char *trim_spaces(char *line);
char *trim_spaces_beginning(char *line);
char *trim_spaces_trailing(char *line);

char *consume_next_line(char **text_ptr);
bool starts_with(char *a, char *b);
void split_line(char *s, char c, Array <char *> &strings);

char *find_character_from_left(char *s, char c);

bool is_end_of_line(char c);
bool is_space(char c);

u64 get_hash(u64 x);
u64 get_hash(char *str);

u64 get_time_nanoseconds();
