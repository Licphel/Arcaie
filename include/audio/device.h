#pragma once
#include <core/def.h>
#include <memory>
#include <core/math.h>
#include <core/io.h>

namespace arc::audio
{

struct track
{
    /* unstable */ unsigned int P_track_id;
    double sec_len;

    ~track();

    static std::shared_ptr<track> load(const path_handle &path);
};

enum class device_option
{
    LISTENER,
    ROLLOFF,
    REFERENCE_DIST,
    MAX_DIST
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
    std::shared_ptr<track> relying_track;
    /* unstable */ unsigned int P_clip_id;

    ~clip();
    clip_status status();
    void set(clip_op param, double v);
    void set(clip_op param, const vec2 &v);
    void set(clip_op param, const vec3 &v);
    // play, loop, pause or stop the clip.
    // note: once the clip is stopped, it cannot be played again.
    void operate(clip_op param);

    static std::shared_ptr<clip> make(std::shared_ptr<track> track);
};

void tk_make_device();
void tk_end_make_device();

// these options should be set between #tk_make_device and #tk_end_make_device.
void tk_set_device_option(device_option opt, double v);
void tk_set_device_option(device_option opt, const vec2 &v);
void tk_set_device_option(device_option opt, const vec3 &v);

} // namespace arc::audio