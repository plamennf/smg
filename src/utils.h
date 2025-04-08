#pragma once

char *read_entire_file(char *filepath, s64 *length_pointer = NULL);
bool file_exists(char *filepath);

u64 round_to_next_power_of_2(u64 v);

bool is_end_of_line(char c);
bool is_space(char c);

u64 get_hash(u64 x);
u64 get_hash(char *str);
