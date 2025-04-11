#include "main.h"

#include "animation.h"
#include "assets.h"

#include <stdio.h>

bool load_animation(Animation *animation, char *filepath) {
    char *file_data = read_entire_file(filepath);
    if (!file_data) {
        fprintf(stderr, "Failed to read file '%s'.\n", filepath);
        return false;
    }
    defer(delete[] file_data);

    char *file_data_at = file_data;

    // Parse version.
    char *line = consume_next_line(&file_data_at);
    if (!line) {
        fprintf(stderr, "File '%s' is too short to be considered a valid animation file!\n", filepath);
        return false;
    }
    line = trim_spaces(line);
    if (!starts_with(line, "version")) {
        fprintf(stderr, "Error in file '%s': Missing version directive!\n", filepath);
        return false;
    }
    line += string_length("version");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        fprintf(stderr, "Error in file '%s': Missing '=' after 'version'\n", filepath);
        return false;
    }
    line += 1;
    line = trim_spaces(line);
    int version = atoi(line);

    if (version <= 0 || version > ANIMATION_FILE_VERSION) {
        fprintf(stderr, "Error in file '%s': Invalid value for version!\n", filepath);
        return false;
    }

    // Parse sample_rate.
    line = consume_next_line(&file_data_at);
    if (!line) {
        fprintf(stderr, "File '%s' is too short to be considered a valid animation file!\n", filepath);
        return false;
    }
    line = trim_spaces(line);
    if (!starts_with(line, "sample_rate")) {
        fprintf(stderr, "Error in file '%s': Missing sample_rate directive!\n", filepath);
        return false;
    }
    line += string_length("sample_rate");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        fprintf(stderr, "Error in file '%s': Missing '=' after 'sample_rate'\n", filepath);
        return false;
    }
    line += 1;
    line = trim_spaces(line);
    int sample_rate = atoi(line);
    if (sample_rate < 1) {
        printf("Warning in file '%s': sample_rate is less than 1, so setting sample_rate to 1\n", filepath);
        return false;
    }
    float inv_sample_rate = 1.0f / (float)sample_rate;

    // Parse is_looping.
    line = consume_next_line(&file_data_at);
    if (!line) {
        fprintf(stderr, "File '%s' is too short to be considered a valid animation file!\n", filepath);
        return false;
    }
    line = trim_spaces(line);
    if (!starts_with(line, "is_looping")) {
        fprintf(stderr, "Error in file '%s': Missing is_looping directive!\n", filepath);
        return false;
    }
    line += string_length("is_looping");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        fprintf(stderr, "Error in file '%s': Missing '=' after 'is_looping'\n", filepath);
        return false;
    }
    line += 1;
    line = trim_spaces(line);

    bool is_looping = true;
    if (strings_match(line, "true") || strings_match(line, "True") || strings_match(line, "1")) {
        is_looping = true;
    } else if (strings_match(line, "false") || strings_match(line, "False") || strings_match(line, "0")) {
        is_looping = false;
    } else {
        fprintf(stderr, "Error in file '%s': Invalid value '%s' for is_looping!\n", filepath, line);
        fprintf(stderr, "Valid values are:\n");
        fprintf(stderr, "    - true\n");
        fprintf(stderr, "    - false\n");
        fprintf(stderr, "    - 1\n");
        fprintf(stderr, "    - 0\n");
        return false;
    }

    // Parse num_frames.
    line = consume_next_line(&file_data_at);
    if (!line) {
        fprintf(stderr, "File '%s' is too short to be considered a valid animation file!\n", filepath);
        return false;
    }
    line = trim_spaces(line);
    if (!starts_with(line, "num_frames")) {
        fprintf(stderr, "Error in file '%s': Missing num_frames directive!\n", filepath);
        return false;
    }
    line += string_length("num_frames");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        fprintf(stderr, "Error in file '%s': Missing '=' after 'num_frames'\n", filepath);
        return false;
    }
    line += 1;
    line = trim_spaces(line);
    int num_frames = atoi(line);

    if (animation->frames) {
        delete[] animation->frames;
        animation->frames = NULL;
    }

    animation->frames = new Texture*[num_frames];
    for (int i = 0; i < num_frames; i++) {
        line = consume_next_line(&file_data_at);
        if (!line) {
            fprintf(stderr, "File '%s' is too short to be considered a valid animation file: Expected '%d' frames, however less have been provided.\n", filepath, num_frames);
            delete[] animation->frames;
            return false;
        }
        animation->frames[i] = find_or_load_texture(line);
    }

    animation->num_frames          = num_frames;
    animation->current_frame_index = 0;
    animation->inv_sample_rate     = inv_sample_rate;
    animation->is_looping          = is_looping;

    return true;
}

void update_animation(Animation *animation, float dt) {
    if (animation->is_completed) return;
    
    animation->accumulated_dt += dt;
    if (animation->accumulated_dt >= animation->inv_sample_rate) {
        animation->current_frame_index++;
        if (animation->current_frame_index >= animation->num_frames) {
            if (animation->is_looping) {
                animation->current_frame_index = 0;
            } else {
                animation->current_frame_index = animation->num_frames - 1;
                animation->is_completed = true;
            }
        }
        animation->accumulated_dt -= animation->inv_sample_rate;
    }
}

Texture *get_current_frame(Animation *animation) {
    if (!animation) return NULL;
    if (!animation->frames) return NULL;

    assert(animation->current_frame_index >= 0);
    assert(animation->current_frame_index < animation->num_frames);
    return animation->frames[animation->current_frame_index];
}

void reset_animation(Animation *animation) {
    if (!animation) return;

    animation->accumulated_dt = 0.0f;
    animation->is_completed   = false;
    animation->current_frame_index = 0;
}
