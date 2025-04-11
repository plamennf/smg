#pragma once

#define ANIMATION_FILE_VERSION 1

struct Texture;

struct Animation {
    int num_frames;
    Texture **frames;
    int current_frame_index;

    float inv_sample_rate;
    bool is_looping;
    bool is_completed; // Only valid if is_looping is false.

    float accumulated_dt;
};

bool load_animation(Animation *animation, char *filepath);

void update_animation(Animation *animation, float dt);
Texture *get_current_frame(Animation *animation);
void reset_animation(Animation *animation);
