#ifndef FFMPEG_UTILS_H
#define FFMPEG_UTILS_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// Audio Format Types
typedef enum {
    AUDIO_FORMAT_MP3,
    AUDIO_FORMAT_WAV,
    AUDIO_FORMAT_AAC,
    AUDIO_FORMAT_OGG,
    AUDIO_FORMAT_FLAC,
    AUDIO_FORMAT_M4A
} AudioFormat;

// Audio Segment Structure
typedef struct AudioSegment AudioSegment;

// Audio Editor Context
typedef struct AudioEditor AudioEditor;

// Error Handling
const char* ffmpeg_get_last_error(void);
void ffmpeg_clear_error(void);

// Audio Editor Functions
AudioEditor* audio_editor_create(void);
void audio_editor_destroy(AudioEditor* editor);

// Audio Loading and Saving
AudioSegment* audio_load(AudioEditor* editor, const char* filename);
bool audio_save(AudioEditor* editor, AudioSegment* segment, const char* filename, AudioFormat format);
void audio_segment_free(AudioSegment* segment);

// Audio Information
double audio_get_duration(AudioSegment* segment);
int audio_get_sample_rate(AudioSegment* segment);
int audio_get_channels(AudioSegment* segment);
int audio_get_bitrate(AudioSegment* segment);

// Audio Editing Operations
AudioSegment* audio_trim(AudioEditor* editor, AudioSegment* segment, double start_time, double end_time);
AudioSegment* audio_fade_in(AudioEditor* editor, AudioSegment* segment, double duration);
AudioSegment* audio_fade_out(AudioEditor* editor, AudioSegment* segment, double duration);
AudioSegment* audio_adjust_volume(AudioEditor* editor, AudioSegment* segment, double volume_db);
AudioSegment* audio_normalize(AudioEditor* editor, AudioSegment* segment);

// Audio Combining Operations
AudioSegment* audio_concat(AudioEditor* editor, AudioSegment** segments, int count);
AudioSegment* audio_merge(AudioEditor* editor, AudioSegment** segments, double* volumes, int count);
AudioSegment* audio_overlay(AudioEditor* editor, AudioSegment* base, AudioSegment* overlay, double position);

// Audio Effects
AudioSegment* audio_speed_change(AudioEditor* editor, AudioSegment* segment, double speed_factor);
AudioSegment* audio_pitch_shift(AudioEditor* editor, AudioSegment* segment, double semitones);
AudioSegment* audio_reverse(AudioEditor* editor, AudioSegment* segment);
AudioSegment* audio_apply_echo(AudioEditor* editor, AudioSegment* segment, double delay_ms, double decay);

// Audio Composition (for final audio generation)
typedef struct {
    AudioSegment* segment;
    double start_time;
    double volume;
    bool fade_in;
    bool fade_out;
    double fade_duration;
} AudioTrack;

AudioSegment* audio_compose(AudioEditor* editor, AudioTrack* tracks, int track_count, double total_duration);

// Format Conversion
AudioSegment* audio_convert_format(AudioEditor* editor, AudioSegment* segment, 
                                   int target_sample_rate, int target_channels);

// Audio Splitting
AudioSegment** audio_split(AudioEditor* editor, AudioSegment* segment, double* split_times, 
                           int split_count, int* out_count);

// Silence Detection and Removal
AudioSegment* audio_remove_silence(AudioEditor* editor, AudioSegment* segment, 
                                   double threshold_db, double min_silence_duration);
double* audio_detect_silence(AudioSegment* segment, double threshold_db, 
                             double min_duration, int* out_count);

#endif // FFMPEG_UTILS_H
