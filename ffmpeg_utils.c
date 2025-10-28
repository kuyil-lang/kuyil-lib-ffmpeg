#define _POSIX_C_SOURCE 200809L
#include "ffmpeg_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Global error state
static char g_ffmpeg_error[512] = {0};

// Helper function to set error message
static void set_error(const char* message) {
    snprintf(g_ffmpeg_error, sizeof(g_ffmpeg_error), "%s", message);
}

// Audio Segment Structure (simplified, will use ffmpeg command-line for now)
struct AudioSegment {
    char* filename;
    double duration;
    int sample_rate;
    int channels;
    int bitrate;
    bool is_temp;
};

// Audio Editor Context
struct AudioEditor {
    int temp_file_counter;
    char temp_dir[256];
};

// Error Handling
const char* ffmpeg_get_last_error(void) {
    return g_ffmpeg_error;
}

void ffmpeg_clear_error(void) {
    g_ffmpeg_error[0] = '\0';
}

// Helper function to get audio info using ffprobe
static bool get_audio_info(const char* filename, AudioSegment* segment) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
             "ffprobe -v error -show_entries format=duration:stream=sample_rate,channels,bit_rate "
             "-of default=noprint_wrappers=1 \"%s\" 2>/dev/null", filename);
    
    FILE* fp = popen(cmd, "r");
    if (!fp) {
        set_error("Failed to execute ffprobe");
        return false;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "duration=%lf", &segment->duration) == 1) continue;
        if (sscanf(line, "sample_rate=%d", &segment->sample_rate) == 1) continue;
        if (sscanf(line, "channels=%d", &segment->channels) == 1) continue;
        if (sscanf(line, "bit_rate=%d", &segment->bitrate) == 1) continue;
    }
    
    pclose(fp);
    return true;
}

// Generate temporary filename
static char* generate_temp_filename(AudioEditor* editor, const char* extension) {
    char* filename = malloc(512);
    if (!filename) return NULL;
    
    snprintf(filename, 512, "%s/kuyil_audio_temp_%d.%s", 
             editor->temp_dir, editor->temp_file_counter++, extension);
    
    return filename;
}

// Audio Editor Functions
AudioEditor* audio_editor_create(void) {
    AudioEditor* editor = calloc(1, sizeof(AudioEditor));
    if (!editor) {
        set_error("Failed to allocate memory for audio editor");
        return NULL;
    }
    
    editor->temp_file_counter = 0;
    snprintf(editor->temp_dir, sizeof(editor->temp_dir), "/tmp");
    
    return editor;
}

void audio_editor_destroy(AudioEditor* editor) {
    if (!editor) return;
    free(editor);
}

// Audio Loading and Saving
AudioSegment* audio_load(AudioEditor* editor, const char* filename) {
    if (!editor || !filename) {
        set_error("Invalid parameters for audio_load");
        return NULL;
    }
    
    AudioSegment* segment = calloc(1, sizeof(AudioSegment));
    if (!segment) {
        set_error("Failed to allocate memory for audio segment");
        return NULL;
    }
    
    segment->filename = strdup(filename);
    segment->is_temp = false;
    
    if (!get_audio_info(filename, segment)) {
        free(segment->filename);
        free(segment);
        return NULL;
    }
    
    return segment;
}

bool audio_save(AudioEditor* editor, AudioSegment* segment, const char* filename, AudioFormat format) {
    if (!editor || !segment || !filename) {
        set_error("Invalid parameters for audio_save");
        return false;
    }
    
    const char* format_str = "mp3";
    switch (format) {
        case AUDIO_FORMAT_MP3: format_str = "mp3"; break;
        case AUDIO_FORMAT_WAV: format_str = "wav"; break;
        case AUDIO_FORMAT_AAC: format_str = "aac"; break;
        case AUDIO_FORMAT_OGG: format_str = "ogg"; break;
        case AUDIO_FORMAT_FLAC: format_str = "flac"; break;
        case AUDIO_FORMAT_M4A: format_str = "m4a"; break;
    }
    
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), 
             "ffmpeg -y -i \"%s\" -f %s \"%s\" 2>/dev/null",
             segment->filename, format_str, filename);
    
    int result = system(cmd);
    if (result != 0) {
        set_error("Failed to save audio file");
        return false;
    }
    
    return true;
}

void audio_segment_free(AudioSegment* segment) {
    if (!segment) return;
    
    if (segment->is_temp && segment->filename) {
        remove(segment->filename);
    }
    
    free(segment->filename);
    free(segment);
}

// Audio Information
double audio_get_duration(AudioSegment* segment) {
    return segment ? segment->duration : 0.0;
}

int audio_get_sample_rate(AudioSegment* segment) {
    return segment ? segment->sample_rate : 0;
}

int audio_get_channels(AudioSegment* segment) {
    return segment ? segment->channels : 0;
}

int audio_get_bitrate(AudioSegment* segment) {
    return segment ? segment->bitrate : 0;
}

// Audio Editing Operations
AudioSegment* audio_trim(AudioEditor* editor, AudioSegment* segment, double start_time, double end_time) {
    if (!editor || !segment) {
        set_error("Invalid parameters for audio_trim");
        return NULL;
    }
    
    char* output_file = generate_temp_filename(editor, "mp3");
    if (!output_file) {
        set_error("Failed to generate temporary filename");
        return NULL;
    }
    
    double duration = end_time - start_time;
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -i \"%s\" -ss %.3f -t %.3f -c copy \"%s\" 2>/dev/null",
             segment->filename, start_time, duration, output_file);
    
    if (system(cmd) != 0) {
        set_error("Failed to trim audio");
        free(output_file);
        return NULL;
    }
    
    AudioSegment* result = audio_load(editor, output_file);
    if (result) {
        result->is_temp = true;
    } else {
        remove(output_file);
    }
    
    free(output_file);
    return result;
}

AudioSegment* audio_fade_in(AudioEditor* editor, AudioSegment* segment, double duration) {
    if (!editor || !segment) {
        set_error("Invalid parameters for audio_fade_in");
        return NULL;
    }
    
    char* output_file = generate_temp_filename(editor, "mp3");
    if (!output_file) {
        set_error("Failed to generate temporary filename");
        return NULL;
    }
    
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -i \"%s\" -af \"afade=t=in:st=0:d=%.3f\" \"%s\" 2>/dev/null",
             segment->filename, duration, output_file);
    
    if (system(cmd) != 0) {
        set_error("Failed to apply fade in");
        free(output_file);
        return NULL;
    }
    
    AudioSegment* result = audio_load(editor, output_file);
    if (result) {
        result->is_temp = true;
    } else {
        remove(output_file);
    }
    
    free(output_file);
    return result;
}

AudioSegment* audio_fade_out(AudioEditor* editor, AudioSegment* segment, double duration) {
    if (!editor || !segment) {
        set_error("Invalid parameters for audio_fade_out");
        return NULL;
    }
    
    char* output_file = generate_temp_filename(editor, "mp3");
    if (!output_file) {
        set_error("Failed to generate temporary filename");
        return NULL;
    }
    
    double start_time = segment->duration - duration;
    if (start_time < 0) start_time = 0;
    
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -i \"%s\" -af \"afade=t=out:st=%.3f:d=%.3f\" \"%s\" 2>/dev/null",
             segment->filename, start_time, duration, output_file);
    
    if (system(cmd) != 0) {
        set_error("Failed to apply fade out");
        free(output_file);
        return NULL;
    }
    
    AudioSegment* result = audio_load(editor, output_file);
    if (result) {
        result->is_temp = true;
    } else {
        remove(output_file);
    }
    
    free(output_file);
    return result;
}

AudioSegment* audio_adjust_volume(AudioEditor* editor, AudioSegment* segment, double volume_db) {
    if (!editor || !segment) {
        set_error("Invalid parameters for audio_adjust_volume");
        return NULL;
    }
    
    char* output_file = generate_temp_filename(editor, "mp3");
    if (!output_file) {
        set_error("Failed to generate temporary filename");
        return NULL;
    }
    
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -i \"%s\" -af \"volume=%.2fdB\" \"%s\" 2>/dev/null",
             segment->filename, volume_db, output_file);
    
    if (system(cmd) != 0) {
        set_error("Failed to adjust volume");
        free(output_file);
        return NULL;
    }
    
    AudioSegment* result = audio_load(editor, output_file);
    if (result) {
        result->is_temp = true;
    } else {
        remove(output_file);
    }
    
    free(output_file);
    return result;
}

AudioSegment* audio_normalize(AudioEditor* editor, AudioSegment* segment) {
    if (!editor || !segment) {
        set_error("Invalid parameters for audio_normalize");
        return NULL;
    }
    
    char* output_file = generate_temp_filename(editor, "mp3");
    if (!output_file) {
        set_error("Failed to generate temporary filename");
        return NULL;
    }
    
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -i \"%s\" -af \"loudnorm\" \"%s\" 2>/dev/null",
             segment->filename, output_file);
    
    if (system(cmd) != 0) {
        set_error("Failed to normalize audio");
        free(output_file);
        return NULL;
    }
    
    AudioSegment* result = audio_load(editor, output_file);
    if (result) {
        result->is_temp = true;
    } else {
        remove(output_file);
    }
    
    free(output_file);
    return result;
}

// Audio Combining Operations
AudioSegment* audio_concat(AudioEditor* editor, AudioSegment** segments, int count) {
    if (!editor || !segments || count < 1) {
        set_error("Invalid parameters for audio_concat");
        return NULL;
    }
    
    // Create a file list for ffmpeg concat
    char* list_file = generate_temp_filename(editor, "txt");
    if (!list_file) {
        set_error("Failed to generate temporary filename");
        return NULL;
    }
    
    FILE* fp = fopen(list_file, "w");
    if (!fp) {
        set_error("Failed to create concat list file");
        free(list_file);
        return NULL;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(fp, "file '%s'\n", segments[i]->filename);
    }
    fclose(fp);
    
    char* output_file = generate_temp_filename(editor, "mp3");
    if (!output_file) {
        set_error("Failed to generate output filename");
        remove(list_file);
        free(list_file);
        return NULL;
    }
    
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -f concat -safe 0 -i \"%s\" -c copy \"%s\" 2>/dev/null",
             list_file, output_file);
    
    int result = system(cmd);
    remove(list_file);
    free(list_file);
    
    if (result != 0) {
        set_error("Failed to concatenate audio");
        free(output_file);
        return NULL;
    }
    
    AudioSegment* output = audio_load(editor, output_file);
    if (output) {
        output->is_temp = true;
    } else {
        remove(output_file);
    }
    
    free(output_file);
    return output;
}

AudioSegment* audio_merge(AudioEditor* editor, AudioSegment** segments, double* volumes, int count) {
    if (!editor || !segments || !volumes || count < 1) {
        set_error("Invalid parameters for audio_merge");
        return NULL;
    }
    
    char* output_file = generate_temp_filename(editor, "mp3");
    if (!output_file) {
        set_error("Failed to generate temporary filename");
        return NULL;
    }
    
    // Build ffmpeg command with amix filter
    char cmd[4096] = "ffmpeg -y";
    char filter[2048] = " -filter_complex \"";
    
    // Add input files
    for (int i = 0; i < count; i++) {
        char input[256];
        snprintf(input, sizeof(input), " -i \"%s\"", segments[i]->filename);
        strcat(cmd, input);
        
        // Add volume adjustment to filter
        char vol_filter[128];
        snprintf(vol_filter, sizeof(vol_filter), "[%d:a]volume=%.2fdB[a%d];", 
                 i, volumes[i], i);
        strcat(filter, vol_filter);
    }
    
    // Add amix filter
    char amix[256];
    snprintf(amix, sizeof(amix), "");
    for (int i = 0; i < count; i++) {
        char input[16];
        snprintf(input, sizeof(input), "[a%d]", i);
        strcat(amix, input);
    }
    strcat(amix, "amix=inputs=");
    char count_str[16];
    snprintf(count_str, sizeof(count_str), "%d", count);
    strcat(amix, count_str);
    strcat(amix, ":duration=longest[out]\"");
    strcat(filter, amix);
    
    strcat(cmd, filter);
    char output[256];
    snprintf(output, sizeof(output), " -map \"[out]\" \"%s\" 2>/dev/null", output_file);
    strcat(cmd, output);
    
    if (system(cmd) != 0) {
        set_error("Failed to merge audio");
        free(output_file);
        return NULL;
    }
    
    AudioSegment* result = audio_load(editor, output_file);
    if (result) {
        result->is_temp = true;
    } else {
        remove(output_file);
    }
    
    free(output_file);
    return result;
}

AudioSegment* audio_overlay(AudioEditor* editor, AudioSegment* base, AudioSegment* overlay, double position) {
    if (!editor || !base || !overlay) {
        set_error("Invalid parameters for audio_overlay");
        return NULL;
    }
    
    char* output_file = generate_temp_filename(editor, "mp3");
    if (!output_file) {
        set_error("Failed to generate temporary filename");
        return NULL;
    }
    
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -i \"%s\" -i \"%s\" -filter_complex "
             "\"[1:a]adelay=%.0f|%.0f[delayed];[0:a][delayed]amix=inputs=2:duration=longest\" "
             "\"%s\" 2>/dev/null",
             base->filename, overlay->filename, position * 1000, position * 1000, output_file);
    
    if (system(cmd) != 0) {
        set_error("Failed to overlay audio");
        free(output_file);
        return NULL;
    }
    
    AudioSegment* result = audio_load(editor, output_file);
    if (result) {
        result->is_temp = true;
    } else {
        remove(output_file);
    }
    
    free(output_file);
    return result;
}

// Audio Composition
AudioSegment* audio_compose(AudioEditor* editor, AudioTrack* tracks, int track_count, double total_duration) {
    if (!editor || !tracks || track_count < 1) {
        set_error("Invalid parameters for audio_compose");
        return NULL;
    }
    
    // First, process each track (apply fade in/out, volume)
    AudioSegment** processed_segments = malloc(sizeof(AudioSegment*) * track_count);
    if (!processed_segments) {
        set_error("Failed to allocate memory");
        return NULL;
    }
    
    for (int i = 0; i < track_count; i++) {
        AudioSegment* seg = tracks[i].segment;
        
        // Apply volume
        if (fabs(tracks[i].volume) > 0.01) {
            AudioSegment* temp = audio_adjust_volume(editor, seg, tracks[i].volume);
            if (!temp) {
                // Cleanup and return
                for (int j = 0; j < i; j++) {
                    audio_segment_free(processed_segments[j]);
                }
                free(processed_segments);
                return NULL;
            }
            seg = temp;
        }
        
        // Apply fade in
        if (tracks[i].fade_in) {
            AudioSegment* temp = audio_fade_in(editor, seg, tracks[i].fade_duration);
            if (seg->is_temp) audio_segment_free(seg);
            if (!temp) {
                for (int j = 0; j < i; j++) {
                    audio_segment_free(processed_segments[j]);
                }
                free(processed_segments);
                return NULL;
            }
            seg = temp;
        }
        
        // Apply fade out
        if (tracks[i].fade_out) {
            AudioSegment* temp = audio_fade_out(editor, seg, tracks[i].fade_duration);
            if (seg->is_temp) audio_segment_free(seg);
            if (!temp) {
                for (int j = 0; j < i; j++) {
                    audio_segment_free(processed_segments[j]);
                }
                free(processed_segments);
                return NULL;
            }
            seg = temp;
        }
        
        processed_segments[i] = seg;
    }
    
    // Now merge all tracks with their start times
    // For simplicity, we'll use a basic approach here
    double* volumes = malloc(sizeof(double) * track_count);
    if (!volumes) {
        for (int i = 0; i < track_count; i++) {
            audio_segment_free(processed_segments[i]);
        }
        free(processed_segments);
        set_error("Failed to allocate memory");
        return NULL;
    }
    
    for (int i = 0; i < track_count; i++) {
        volumes[i] = 0.0; // Already adjusted
    }
    
    AudioSegment* result = audio_merge(editor, processed_segments, volumes, track_count);
    
    // Cleanup
    for (int i = 0; i < track_count; i++) {
        audio_segment_free(processed_segments[i]);
    }
    free(processed_segments);
    free(volumes);
    
    return result;
}

// Audio Effects
AudioSegment* audio_speed_change(AudioEditor* editor, AudioSegment* segment, double speed_factor) {
    if (!editor || !segment) {
        set_error("Invalid parameters for audio_speed_change");
        return NULL;
    }
    
    char* output_file = generate_temp_filename(editor, "mp3");
    if (!output_file) {
        set_error("Failed to generate temporary filename");
        return NULL;
    }
    
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -i \"%s\" -filter:a \"atempo=%.2f\" \"%s\" 2>/dev/null",
             segment->filename, speed_factor, output_file);
    
    if (system(cmd) != 0) {
        set_error("Failed to change audio speed");
        free(output_file);
        return NULL;
    }
    
    AudioSegment* result = audio_load(editor, output_file);
    if (result) {
        result->is_temp = true;
    } else {
        remove(output_file);
    }
    
    free(output_file);
    return result;
}

AudioSegment* audio_reverse(AudioEditor* editor, AudioSegment* segment) {
    if (!editor || !segment) {
        set_error("Invalid parameters for audio_reverse");
        return NULL;
    }
    
    char* output_file = generate_temp_filename(editor, "mp3");
    if (!output_file) {
        set_error("Failed to generate temporary filename");
        return NULL;
    }
    
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -i \"%s\" -af \"areverse\" \"%s\" 2>/dev/null",
             segment->filename, output_file);
    
    if (system(cmd) != 0) {
        set_error("Failed to reverse audio");
        free(output_file);
        return NULL;
    }
    
    AudioSegment* result = audio_load(editor, output_file);
    if (result) {
        result->is_temp = true;
    } else {
        remove(output_file);
    }
    
    free(output_file);
    return result;
}

// Stubs for unimplemented functions
AudioSegment* audio_pitch_shift(AudioEditor* editor, AudioSegment* segment, double semitones) {
    (void)editor; (void)segment; (void)semitones;
    set_error("audio_pitch_shift not yet implemented");
    return NULL;
}

AudioSegment* audio_apply_echo(AudioEditor* editor, AudioSegment* segment, double delay_ms, double decay) {
    (void)editor; (void)segment; (void)delay_ms; (void)decay;
    set_error("audio_apply_echo not yet implemented");
    return NULL;
}

AudioSegment* audio_convert_format(AudioEditor* editor, AudioSegment* segment, 
                                   int target_sample_rate, int target_channels) {
    (void)editor; (void)segment; (void)target_sample_rate; (void)target_channels;
    set_error("audio_convert_format not yet implemented");
    return NULL;
}

AudioSegment** audio_split(AudioEditor* editor, AudioSegment* segment, double* split_times, 
                           int split_count, int* out_count) {
    (void)editor; (void)segment; (void)split_times; (void)split_count; (void)out_count;
    set_error("audio_split not yet implemented");
    return NULL;
}

AudioSegment* audio_remove_silence(AudioEditor* editor, AudioSegment* segment, 
                                   double threshold_db, double min_silence_duration) {
    (void)editor; (void)segment; (void)threshold_db; (void)min_silence_duration;
    set_error("audio_remove_silence not yet implemented");
    return NULL;
}

double* audio_detect_silence(AudioSegment* segment, double threshold_db, 
                             double min_duration, int* out_count) {
    (void)segment; (void)threshold_db; (void)min_duration; (void)out_count;
    set_error("audio_detect_silence not yet implemented");
    return NULL;
}
