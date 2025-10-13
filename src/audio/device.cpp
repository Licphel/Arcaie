#include <core/math.h>
#include <al/al.h>
#include <al/alc.h>
#include <audio/device.h>
#include <core/log.h>
#include <gfx/device.h>

using namespace arc::gfx;

namespace arc::audio
{

static ALCdevice *al_dev;
static ALCcontext *al_ctx;
static std::vector<std::shared_ptr<clip>> clip_r;
static double rolloff = 1.0;
static double max_dist = 1.0;
static double ref_dist = 1.0;

track::~track()
{
    alDeleteBuffers(1, &P_track_id);
}

extern std::shared_ptr<track> P_wav_load(const path_handle &path);

std::shared_ptr<track> track::load(const path_handle &path)
{
    return P_wav_load(path);
}

clip::~clip()
{
    alDeleteSources(1, &P_clip_id);
}

clip_status clip::status()
{
    int sts;
    alGetSourcei(P_clip_id, AL_SOURCE_STATE, &sts);
    if (sts == AL_STOPPED)
        return clip_status::END;
    if (sts == AL_PLAYING)
        return clip_status::PLAYING;
    if (sts == AL_INITIAL)
        return clip_status::IDLE;
    return clip_status::PAUSED;
}

void clip::set(clip_op param, double v)
{
    if (param == clip_op::GAIN)
        alSourcef(P_clip_id, AL_GAIN, v);
    else if (param == clip_op::PITCH)
        alSourcef(P_clip_id, AL_PITCH, v);
}

void clip::set(clip_op param, const vec2 &v)
{
    if (param == clip_op::LOCATION)
        alSource3f(P_clip_id, AL_POSITION, v.x, v.y, 0);
}

void clip::set(clip_op param, const vec3 &v)
{
    if (param == clip_op::LOCATION)
        alSource3f(P_clip_id, AL_POSITION, v.x, v.y, 0);
}

void clip::operate(clip_op param)
{
    if (param == clip_op::PLAY)
        alSourcePlay(P_clip_id);
    else if (param == clip_op::LOOP)
    {
        alSourcei(P_clip_id, AL_LOOPING, AL_TRUE);
        alSourcePlay(P_clip_id);
    }
    else if (param == clip_op::PAUSE)
        alSourcePause(P_clip_id);
    else if (param == clip_op::STOP)
        alSourceStop(P_clip_id);
}

std::shared_ptr<clip> clip::make(std::shared_ptr<track> track)
{
    unsigned int id;
    alGenSources(1, &id);
    alSourcei(id, AL_BUFFER, track->P_track_id);
    alSourcef(id, AL_ROLLOFF_FACTOR, rolloff);
    alSourcef(id, AL_REFERENCE_DISTANCE, ref_dist);
    alSourcef(id, AL_MAX_DISTANCE, max_dist);

    std::shared_ptr<clip> ptr = std::make_shared<clip>();
    ptr->relying_track = track;
    ptr->P_clip_id = id;
    clip_r.push_back(ptr);
    return ptr;
}

void P_process_tracks()
{
    auto it = clip_r.begin();
    while (it != clip_r.end())
    {
        auto &cl = *it;
        if (cl->status() == clip_status::END)
            it = clip_r.erase(it);
        else
            ++it;
    }
}

void tk_make_device()
{
    // init openal
    al_dev = alcOpenDevice(nullptr);
    al_ctx = alcCreateContext(al_dev, nullptr);
    alcMakeContextCurrent(al_ctx);
    tk_hook_event_tick([]() { P_process_tracks(); });
    tk_hook_event_dispose([]() {
        alcDestroyContext(al_ctx);
        alcCloseDevice(al_dev);
    });
}

void tk_end_make_device()
{
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    // nothing
}

void tk_set_device_option(device_option opt, double v)
{
    if (opt == device_option::ROLLOFF)
        rolloff = v;
    else if (opt == device_option::REFERENCE_DIST)
        ref_dist = v;
    else if (opt == device_option::MAX_DIST)
        max_dist = v;
}

void tk_set_device_option(device_option opt, const vec2 &v)
{
    if (opt == device_option::LISTENER)
        alListener3f(AL_POSITION, v.x, v.y, 0);
}

void tk_set_device_option(device_option opt, const vec3 &v)
{
    if (opt == device_option::LISTENER)
        alListener3f(AL_POSITION, v.x, v.y, v.z);
}

} // namespace arc::audio
