#define _POSIX_C_SOURCE 200809L
// Kuyil Bridge for FFmpeg Audio Library
#include "ffmpeg_utils.h"
#include "../../src/ast.h"
#include <string.h>
#include <stdlib.h>

// Kuyil interface signature metadata
__attribute__((visibility("default")))
const char* kyl_interface_signature_text = 
    "ffmpeg getLastError() -> string\n"
    "ffmpeg clearError() -> bool\n"
    "audio getDuration(path: string) -> float64\n"
    "audio getSampleRate(path: string) -> int32\n"
    "audio getChannels(path: string) -> int32\n"
    "editor create() -> int32\n"
    "editor destroy(handle: int32) -> bool\n"
    "audio load(path: string) -> int32\n"
    "audio save(handle: int32, path: string) -> bool\n"
    "audio segmentFree(handle: int32) -> bool\n"
    "audio trim(handle: int32, startMs: float64, endMs: float64) -> int32\n"
    "audio fadeIn(handle: int32, durationMs: float64) -> int32\n"
    "audio fadeOut(handle: int32, durationMs: float64) -> int32\n"
    "audio adjustVolume(handle: int32, factor: float64) -> int32\n"
    "audio normalize(handle: int32) -> int32\n"
    "audio concat(handle1: int32, handle2: int32) -> int32\n"
    "audio merge(handle1: int32, handle2: int32) -> int32\n"
    "audio overlay(base: int32, overlay: int32, positionMs: float64) -> int32\n"
    "audio speedChange(handle: int32, factor: float64) -> int32\n"
    "audio reverse(handle: int32) -> int32\n";

// Helper functions
static Value ptr_to_value(void* ptr) {
    Value result;
    memset(&result, 0, sizeof(Value));
    result.type = VALUE_NUMBER;
    result.as.number = (double)(uintptr_t)ptr;
    return result;
}

static void* value_to_ptr(Value* val) {
    if (val->type != VALUE_NUMBER) return NULL;
    return (void*)(uintptr_t)val->as.number;
}

static Value string_to_value(const char* str) {
    Value result;
    memset(&result, 0, sizeof(Value));
    result.type = VALUE_STRING;
    result.as.string = str ? strdup(str) : strdup("");
    return result;
}

static Value bool_to_value(bool b) {
    Value result;
    memset(&result, 0, sizeof(Value));
    result.type = VALUE_BOOL;
    result.as.boolean = b;
    return result;
}

static Value number_to_value(double num) {
    Value result;
    memset(&result, 0, sizeof(Value));
    result.type = VALUE_NUMBER;
    result.as.number = num;
    return result;
}

// Audio Editor Functions
Value audio_editor_create_kyl(int arg_count, Value* args) {
    (void)arg_count; (void)args;
    
    AudioEditor* editor = audio_editor_create();
    if (!editor) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    return ptr_to_value(editor);
}

Value audio_editor_destroy_kyl(int arg_count, Value* args) {
    if (arg_count < 1) return bool_to_value(false);
    
    AudioEditor* editor = (AudioEditor*)value_to_ptr(&args[0]);
    audio_editor_destroy(editor);
    
    return bool_to_value(true);
}

// Audio Loading and Saving
Value audio_load_kyl(int arg_count, Value* args) {
    if (arg_count < 2 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_STRING) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    AudioEditor* editor = (AudioEditor*)value_to_ptr(&args[0]);
    const char* filename = args[1].as.string;
    
    AudioSegment* segment = audio_load(editor, filename);
    if (!segment) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    return ptr_to_value(segment);
}

Value audio_save_kyl(int arg_count, Value* args) {
    if (arg_count < 4 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_NUMBER ||
        args[2].type != VALUE_STRING || args[3].type != VALUE_NUMBER) {
        return bool_to_value(false);
    }
    
    AudioEditor* editor = (AudioEditor*)value_to_ptr(&args[0]);
    AudioSegment* segment = (AudioSegment*)value_to_ptr(&args[1]);
    const char* filename = args[2].as.string;
    AudioFormat format = (AudioFormat)(int)args[3].as.number;
    
    bool result = audio_save(editor, segment, filename, format);
    return bool_to_value(result);
}

Value audio_segment_free_kyl(int arg_count, Value* args) {
    if (arg_count < 1) return bool_to_value(false);
    
    AudioSegment* segment = (AudioSegment*)value_to_ptr(&args[0]);
    audio_segment_free(segment);
    
    return bool_to_value(true);
}

// Audio Information
Value audio_get_duration_kyl(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VALUE_NUMBER) {
        return number_to_value(0.0);
    }
    
    AudioSegment* segment = (AudioSegment*)value_to_ptr(&args[0]);
    double duration = audio_get_duration(segment);
    
    return number_to_value(duration);
}

Value audio_get_sample_rate_kyl(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VALUE_NUMBER) {
        return number_to_value(0);
    }
    
    AudioSegment* segment = (AudioSegment*)value_to_ptr(&args[0]);
    int sample_rate = audio_get_sample_rate(segment);
    
    return number_to_value(sample_rate);
}

Value audio_get_channels_kyl(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VALUE_NUMBER) {
        return number_to_value(0);
    }
    
    AudioSegment* segment = (AudioSegment*)value_to_ptr(&args[0]);
    int channels = audio_get_channels(segment);
    
    return number_to_value(channels);
}

// Audio Editing Operations
Value audio_trim_kyl(int arg_count, Value* args) {
    if (arg_count < 4 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_NUMBER ||
        args[2].type != VALUE_NUMBER || args[3].type != VALUE_NUMBER) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    AudioEditor* editor = (AudioEditor*)value_to_ptr(&args[0]);
    AudioSegment* segment = (AudioSegment*)value_to_ptr(&args[1]);
    double start_time = args[2].as.number;
    double end_time = args[3].as.number;
    
    AudioSegment* result = audio_trim(editor, segment, start_time, end_time);
    if (!result) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    return ptr_to_value(result);
}

Value audio_fade_in_kyl(int arg_count, Value* args) {
    if (arg_count < 3 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_NUMBER ||
        args[2].type != VALUE_NUMBER) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    AudioEditor* editor = (AudioEditor*)value_to_ptr(&args[0]);
    AudioSegment* segment = (AudioSegment*)value_to_ptr(&args[1]);
    double duration = args[2].as.number;
    
    AudioSegment* result = audio_fade_in(editor, segment, duration);
    if (!result) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    return ptr_to_value(result);
}

Value audio_fade_out_kyl(int arg_count, Value* args) {
    if (arg_count < 3 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_NUMBER ||
        args[2].type != VALUE_NUMBER) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    AudioEditor* editor = (AudioEditor*)value_to_ptr(&args[0]);
    AudioSegment* segment = (AudioSegment*)value_to_ptr(&args[1]);
    double duration = args[2].as.number;
    
    AudioSegment* result = audio_fade_out(editor, segment, duration);
    if (!result) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    return ptr_to_value(result);
}

Value audio_adjust_volume_kyl(int arg_count, Value* args) {
    if (arg_count < 3 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_NUMBER ||
        args[2].type != VALUE_NUMBER) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    AudioEditor* editor = (AudioEditor*)value_to_ptr(&args[0]);
    AudioSegment* segment = (AudioSegment*)value_to_ptr(&args[1]);
    double volume_db = args[2].as.number;
    
    AudioSegment* result = audio_adjust_volume(editor, segment, volume_db);
    if (!result) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    return ptr_to_value(result);
}

Value audio_normalize_kyl(int arg_count, Value* args) {
    if (arg_count < 2 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_NUMBER) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    AudioEditor* editor = (AudioEditor*)value_to_ptr(&args[0]);
    AudioSegment* segment = (AudioSegment*)value_to_ptr(&args[1]);
    
    AudioSegment* result = audio_normalize(editor, segment);
    if (!result) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    return ptr_to_value(result);
}

// Audio Combining Operations
Value audio_concat_kyl(int arg_count, Value* args) {
    if (arg_count < 2 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_ARRAY) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    AudioEditor* editor = (AudioEditor*)value_to_ptr(&args[0]);
    int count = args[1].as.array.count;
    
    AudioSegment** segments = malloc(sizeof(AudioSegment*) * count);
    if (!segments) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    for (int i = 0; i < count; i++) {
        segments[i] = (AudioSegment*)value_to_ptr(&args[1].as.array.values[i]);
    }
    
    AudioSegment* result = audio_concat(editor, segments, count);
    free(segments);
    
    if (!result) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    return ptr_to_value(result);
}

Value audio_merge_kyl(int arg_count, Value* args) {
    if (arg_count < 3 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_ARRAY ||
        args[2].type != VALUE_ARRAY) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    AudioEditor* editor = (AudioEditor*)value_to_ptr(&args[0]);
    int count = args[1].as.array.count;
    
    if (args[2].as.array.count != count) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    AudioSegment** segments = malloc(sizeof(AudioSegment*) * count);
    double* volumes = malloc(sizeof(double) * count);
    
    if (!segments || !volumes) {
        free(segments);
        free(volumes);
        Value error = {VALUE_NIL};
        return error;
    }
    
    for (int i = 0; i < count; i++) {
        segments[i] = (AudioSegment*)value_to_ptr(&args[1].as.array.values[i]);
        volumes[i] = args[2].as.array.values[i].as.number;
    }
    
    AudioSegment* result = audio_merge(editor, segments, volumes, count);
    free(segments);
    free(volumes);
    
    if (!result) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    return ptr_to_value(result);
}

Value audio_overlay_kyl(int arg_count, Value* args) {
    if (arg_count < 4 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_NUMBER ||
        args[2].type != VALUE_NUMBER || args[3].type != VALUE_NUMBER) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    AudioEditor* editor = (AudioEditor*)value_to_ptr(&args[0]);
    AudioSegment* base = (AudioSegment*)value_to_ptr(&args[1]);
    AudioSegment* overlay = (AudioSegment*)value_to_ptr(&args[2]);
    double position = args[3].as.number;
    
    AudioSegment* result = audio_overlay(editor, base, overlay, position);
    if (!result) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    return ptr_to_value(result);
}

// Audio Effects
Value audio_speed_change_kyl(int arg_count, Value* args) {
    if (arg_count < 3 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_NUMBER ||
        args[2].type != VALUE_NUMBER) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    AudioEditor* editor = (AudioEditor*)value_to_ptr(&args[0]);
    AudioSegment* segment = (AudioSegment*)value_to_ptr(&args[1]);
    double speed_factor = args[2].as.number;
    
    AudioSegment* result = audio_speed_change(editor, segment, speed_factor);
    if (!result) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    return ptr_to_value(result);
}

Value audio_reverse_kyl(int arg_count, Value* args) {
    if (arg_count < 2 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_NUMBER) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    AudioEditor* editor = (AudioEditor*)value_to_ptr(&args[0]);
    AudioSegment* segment = (AudioSegment*)value_to_ptr(&args[1]);
    
    AudioSegment* result = audio_reverse(editor, segment);
    if (!result) {
        Value error = {VALUE_NIL};
        return error;
    }
    
    return ptr_to_value(result);
}

// Error Handling
Value ffmpeg_get_last_error_kyl(int arg_count, Value* args) {
    (void)arg_count; (void)args;
    
    const char* error = ffmpeg_get_last_error();
    return string_to_value(error);
}

Value ffmpeg_clear_error_kyl(int arg_count, Value* args) {
    (void)arg_count; (void)args;
    
    ffmpeg_clear_error();
    return bool_to_value(true);
}

// LowerCamel aliases for cleaner interface
Value getLastError(int arg_count, Value* args) { return ffmpeg_get_last_error_kyl(arg_count, args); }
Value clearError(int arg_count, Value* args) { return ffmpeg_clear_error_kyl(arg_count, args); }
Value getDuration(int arg_count, Value* args) { return audio_get_duration_kyl(arg_count, args); }
Value getSampleRate(int arg_count, Value* args) { return audio_get_sample_rate_kyl(arg_count, args); }
Value getChannels(int arg_count, Value* args) { return audio_get_channels_kyl(arg_count, args); }
Value create(int arg_count, Value* args) { return audio_editor_create_kyl(arg_count, args); }
Value destroy(int arg_count, Value* args) { return audio_editor_destroy_kyl(arg_count, args); }
Value load(int arg_count, Value* args) { return audio_load_kyl(arg_count, args); }
Value save(int arg_count, Value* args) { return audio_save_kyl(arg_count, args); }
Value segmentFree(int arg_count, Value* args) { return audio_segment_free_kyl(arg_count, args); }
Value trim(int arg_count, Value* args) { return audio_trim_kyl(arg_count, args); }
Value fadeIn(int arg_count, Value* args) { return audio_fade_in_kyl(arg_count, args); }
Value fadeOut(int arg_count, Value* args) { return audio_fade_out_kyl(arg_count, args); }
Value adjustVolume(int arg_count, Value* args) { return audio_adjust_volume_kyl(arg_count, args); }
Value normalize(int arg_count, Value* args) { return audio_normalize_kyl(arg_count, args); }
Value concat(int arg_count, Value* args) { return audio_concat_kyl(arg_count, args); }
Value merge(int arg_count, Value* args) { return audio_merge_kyl(arg_count, args); }
Value overlay(int arg_count, Value* args) { return audio_overlay_kyl(arg_count, args); }
Value speedChange(int arg_count, Value* args) { return audio_speed_change_kyl(arg_count, args); }
Value reverse(int arg_count, Value* args) { return audio_reverse_kyl(arg_count, args); }
