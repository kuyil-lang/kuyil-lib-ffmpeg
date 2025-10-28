# Kuyil FFmpeg Audio Library

A comprehensive audio editing library for the Kuyil programming language, providing easy-to-use functions for audio manipulation using FFmpeg.

## Features

### Basic Operations
- ✅ **Load and Save Audio** - Support for MP3, WAV, AAC, OGG, FLAC, M4A formats
- ✅ **Get Audio Info** - Duration, sample rate, channels, bitrate
- ✅ **Trim Audio** - Cut audio segments by time
- ✅ **Fade In/Out** - Smooth volume transitions
- ✅ **Volume Adjustment** - Change volume in dB
- ✅ **Normalize** - Automatic loudness normalization

### Combining Audio
- ✅ **Concatenate** - Join multiple audio files sequentially
- ✅ **Merge** - Mix multiple audio tracks with individual volume levels
- ✅ **Overlay** - Layer audio on top of base track at specific position

### Effects
- ✅ **Speed Change** - Speed up or slow down audio
- ✅ **Reverse** - Play audio backwards
- ⏳ **Pitch Shift** - Change pitch (planned)
- ⏳ **Echo** - Add echo effect (planned)

### Advanced
- ⏳ **Format Conversion** - Convert between sample rates and channels
- ⏳ **Split Audio** - Split at specific timestamps
- ⏳ **Silence Detection** - Find and remove silent parts

## Requirements

- FFmpeg (command-line tool)
- FFprobe (usually included with FFmpeg)

### Installation

```bash
# Ubuntu/Debian
sudo apt-get install ffmpeg

# Or use the Makefile
make deps
```

## Building

```bash
# From kuyil root directory
make -f Makefile.libs all

# Or build just ffmpeg library
cd additional_libs/ffmpeg
make
```

## Usage Example

```kuyil
// Create audio editor
let editor = audio_editor_create_kyl()

// Load audio file
let audio = audio_load_kyl(editor, "input.mp3")

// Get information
let duration = audio_get_duration_kyl(audio)
print("Duration: " + to_string(duration) + " seconds")

// Trim audio (from 5s to 10s)
let trimmed = audio_trim_kyl(editor, audio, 5.0, 10.0)

// Add fade in (2 seconds)
let faded = audio_fade_in_kyl(editor, trimmed, 2.0)

// Adjust volume (+3dB louder)
let louder = audio_adjust_volume_kyl(editor, faded, 3.0)

// Save result
audio_save_kyl(editor, louder, "output.mp3", 0)  // 0 = MP3 format

// Cleanup
audio_segment_free_kyl(louder)
audio_segment_free_kyl(faded)
audio_segment_free_kyl(trimmed)
audio_segment_free_kyl(audio)
audio_editor_destroy_kyl(editor)
```

## Merging Multiple Audio Tracks

```kuyil
let editor = audio_editor_create_kyl()

// Load multiple audio files
let track1 = audio_load_kyl(editor, "music.mp3")
let track2 = audio_load_kyl(editor, "voice.mp3")
let track3 = audio_load_kyl(editor, "effects.mp3")

// Define volume levels (in dB)
let volumes = [0.0, -3.0, -6.0]  // music at 0dB, voice -3dB, effects -6dB
let tracks = [track1, track2, track3]

// Merge all tracks
let final_audio = audio_merge_kyl(editor, tracks, volumes)

// Save the composition
audio_save_kyl(editor, final_audio, "final_mix.mp3", 0)

// Cleanup
audio_segment_free_kyl(final_audio)
audio_segment_free_kyl(track1)
audio_segment_free_kyl(track2)
audio_segment_free_kyl(track3)
audio_editor_destroy_kyl(editor)
```

## API Reference

### Audio Editor

- `audio_editor_create_kyl()` - Create a new audio editor
- `audio_editor_destroy_kyl(editor)` - Destroy audio editor

### Loading/Saving

- `audio_load_kyl(editor, filename)` - Load audio file
- `audio_save_kyl(editor, segment, filename, format)` - Save audio file
- `audio_segment_free_kyl(segment)` - Free audio segment

### Information

- `audio_get_duration_kyl(segment)` - Get duration in seconds
- `audio_get_sample_rate_kyl(segment)` - Get sample rate
- `audio_get_channels_kyl(segment)` - Get number of channels

### Editing

- `audio_trim_kyl(editor, segment, start_time, end_time)` - Trim audio
- `audio_fade_in_kyl(editor, segment, duration)` - Add fade in
- `audio_fade_out_kyl(editor, segment, duration)` - Add fade out
- `audio_adjust_volume_kyl(editor, segment, volume_db)` - Adjust volume
- `audio_normalize_kyl(editor, segment)` - Normalize loudness

### Combining

- `audio_concat_kyl(editor, segments_array)` - Concatenate segments
- `audio_merge_kyl(editor, segments_array, volumes_array)` - Merge with volumes
- `audio_overlay_kyl(editor, base, overlay, position)` - Overlay audio

### Effects

- `audio_speed_change_kyl(editor, segment, speed_factor)` - Change speed (0.5 = half speed, 2.0 = double speed)
- `audio_reverse_kyl(editor, segment)` - Reverse audio

### Audio Formats

| Code | Format |
|------|--------|
| 0    | MP3    |
| 1    | WAV    |
| 2    | AAC    |
| 3    | OGG    |
| 4    | FLAC   |
| 5    | M4A    |

## Implementation Notes

- This library uses FFmpeg command-line tools for audio processing
- Temporary files are created in `/tmp` during operations
- The library automatically manages temporary files
- All audio segments should be freed with `audio_segment_free_kyl()` when done
- Volume levels are in decibels (dB): 0 = original, positive = louder, negative = quieter

## Error Handling

```kuyil
let error = ffmpeg_get_last_error_kyl()
if error != "" {
    print("Error: " + error)
}
ffmpeg_clear_error_kyl()
```

## License

MIT License - Part of the Kuyil Programming Language
