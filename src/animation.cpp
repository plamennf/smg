#include "main.h"

#include "animation.h"
#include "assets.h"
#include "text_file_handler.h"

#include <stdio.h>

bool load_animation(Animation *animation, char *filepath) {
    Text_File_Handler handler;
    if (!start_file(&handler, filepath)) return false;
    defer(end_file(&handler));

    if (handler.version <= 0 || handler.version > ANIMATION_FILE_VERSION) {
        fprintf(stderr, "Invalid value for version!");
        return false;
    }

    // Parse sample_rate.
    char *line = consume_next_line(&handler);
    if (!line) {
        report_error(&handler, "File is too short to be considered a valid animation file!");
        return false;
    }
    line = trim_spaces(line);
    if (!starts_with(line, "sample_rate")) {
        report_error(&handler, "Missing sample_rate directive!");
        return false;
    }
    line += string_length("sample_rate");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        report_error(&handler, "Missing '=' after 'sample_rate'");
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
    line = consume_next_line(&handler);
    if (!line) {
        report_error(&handler, "File is too short to be considered a valid animation file!");
        return false;
    }
    line = trim_spaces(line);
    if (!starts_with(line, "is_looping")) {
        report_error(&handler, "Missing is_looping directive!");
        return false;
    }
    line += string_length("is_looping");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        report_error(&handler, "Missing '=' after 'is_looping'");
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
        report_error(&handler, "Invalid value '%s' for is_looping!\nValid values are:\n    -true\n    -false\n    - 1\n    -0", line);
        return false;
    }

    // Parse num_frames.
    line = consume_next_line(&handler);
    if (!line) {
        report_error(&handler, "File is too short to be considered a valid animation file!");
        return false;
    }
    line = trim_spaces(line);
    if (!starts_with(line, "num_frames")) {
        report_error(&handler, "Missing num_frames directive!");
        return false;
    }
    line += string_length("num_frames");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        report_error(&handler, "Missing '=' after 'num_frames'");
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
        line = consume_next_line(&handler);
        if (!line) {
            report_error(&handler, "File is too short to be considered a valid animation file: Expected '%d' frames, however less have been provided.", num_frames);
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
