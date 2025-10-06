#pragma once
#include <core/def.h>
#include <core/math.h>
#include <core/hio.h>

namespace arcaie::audi
{

enum device_option
{
    ARC_AUDIO_LISTENER,
    ARC_AUDIO_ROLLOFF,
    ARC_AUDIO_REFERENCE_DIST,
    ARC_AUDIO_MAX_DIST
};

struct track
{
    /* unstable */ unsigned int __track_id;
    double sec_len;

    ~track();
};

enum clip_op
{
    ARC_CLIP_LOCATION,
    ARC_CLIP_GAIN,
    ARC_CLIP_PITCH,

    ARC_CLIP_LOOP,
    ARC_CLIP_PLAY,
    ARC_CLIP_PAUSE,
    ARC_CLIP_STOP
};

enum clip_status
{
    ARC_CLIP_PLAYING,
    ARC_CLIP_IDLE,
    ARC_CLIP_END,
    ARC_CLIP_PAUSED
};

struct clip
{
    shared<track> relying_track;
    /* unstable */ unsigned int __clip_id;

    ~clip();
    clip_status status();
    void set(clip_op param, double v);
    void set(clip_op param, const vec2 &v);
    void set(clip_op param, const vec3 &v);
    // play, loop, pause or stop the clip.
    // note: once the clip is stopped, it cannot be played again.
    void operate(clip_op param);
};

void tk_make_device();
void tk_end_make_device();
shared<track> load_track(const hio_path &path);
shared<clip> make_clip(shared<track> track);

// these options should be set between #tk_make_device and #tk_end_make_device.
void tk_set_device_option(device_option opt, double v);
void tk_set_device_option(device_option opt, const vec2 &v);
void tk_set_device_option(device_option opt, const vec3 &v);

} // namespace arcaie::audi