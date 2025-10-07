#pragma once
#include <core/def.h>
#include <core/math.h>
#include <core/io.h>

namespace arcaie::audio
{

enum class device_option
{
    LISTENER,
    ROLLOFF,
    REFERENCE_DIST,
    MAX_DIST
};

struct track
{
    /* unstable */ unsigned int P_track_id;
    double sec_len;

    ~track();
};

enum class clip_op
{
    LOCATION,
    GAIN,
    PITCH,

    LOOP,
    PLAY,
    PAUSE,
    STOP
};

enum class clip_status
{
    PLAYING,
    IDLE,
    END,
    PAUSED
};

struct clip
{
    shared<track> relying_track;
    /* unstable */ unsigned int P_clip_id;

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
shared<track> load_track(const path_handle &path);
shared<clip> make_clip(shared<track> track);

// these options should be set between #tk_make_device and #tk_end_make_device.
void tk_set_device_option(device_option opt, double v);
void tk_set_device_option(device_option opt, const vec2 &v);
void tk_set_device_option(device_option opt, const vec3 &v);

} // namespace arcaie::audio