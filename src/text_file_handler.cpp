#include "main.h"

#include "text_file_handler.h"

#include <stdio.h>
#include <stdarg.h>

bool start_file(Text_File_Handler *handler, char *filepath) {
    handler->filepath     = filepath;

    handler->file_data    = read_entire_file(filepath);
    handler->file_data_at = handler->file_data;

    if (!handler->file_data) {
        fprintf(stderr, "Failed to load file '%s'.\n", filepath);
        return false;
    }

    if (handler->do_version_number) {
        char *line = consume_next_line(handler);

        if (!line) {
            fprintf(stderr, "Failed to find a version number at the top of file '%s'!\n", filepath);
            delete [] handler->file_data;
            return false;
        }

        line = trim_spaces(line);
        if (!starts_with(line, "version")) {
            fprintf(stderr, "Error in file '%s': Missing version directive!\n", filepath);
            delete [] handler->file_data;
            return false;
        }
        line += string_length("version");
        line = trim_spaces(line);

        if (line[0] != '=') {
            fprintf(stderr, "Error in file '%s: Missing '=' after 'version'\n'", filepath);
            delete [] handler->file_data;
            return false;
        }
        line++;

        line = trim_spaces(line);

        handler->version = atoi(line);
    }

    return true;
}

void end_file(Text_File_Handler *handler) {
    if (handler->file_data) {
        delete [] handler->file_data;
        handler->file_data = NULL;
    }
}

char *consume_next_line(Text_File_Handler *handler) {
    while (1) {
        char *line = consume_next_line(&handler->file_data_at);
        if (!line) return NULL;

        handler->line_number += 1;

        line = trim_spaces(line);

        if (!line || line[0] == 0) continue;

        if (handler->strip_comments_from_end_of_lines) {
            char *rhs = find_character_from_left(line, handler->comment_character);
            if (rhs) {
                line[rhs - line] = 0;
                if (line[0] == 0) continue;
            }
        } else {
            if (line[0] == handler->comment_character) continue;
        }

        assert(line[0] != 0);
        return line;
    }
}

void report_error(Text_File_Handler *handler, char *fmt, ...) {
    char buf[4096];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    fprintf(stderr, "Error in line %d of file '%s': %s\n", handler->line_number, handler->filepath, buf);
}
