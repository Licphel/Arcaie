#include <audio/device.h>
#include <al/alc.h>
#include <al/al.h>
#include <gfx/device.h>
#include <vector>
#include <core/log.h>
#include <memory>

using namespace arcaie::gfx;

namespace arcaie::audio
{

track::~track()
{
    alDeleteBuffers(1, &P_track_id);
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

static ALCdevice *al_dev;
static ALCcontext *al_ctx;
static std::vector<shared<clip>> clip_r;
static double rolloff = 1.0;
static double max_dist = 1.0;
static double ref_dist = 1.0;

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

shared<track> load_track(const path_handle &path)
{
    auto file = io_read_bytes(path);

    size_t index = 0;

    if (file.size() < 12)
        arcthrow(ARC_FATAL, "too small file: {}", path.absolute);

    if (file[index++] != 'R' || file[index++] != 'I' || file[index++] != 'F' || file[index++] != 'F')
        arcthrow(ARC_FATAL, "not a wave file: {}", path.absolute);

    index += 4;

    if (file[index++] != 'W' || file[index++] != 'A' || file[index++] != 'V' || file[index++] != 'E')
        arcthrow(ARC_FATAL, "not a wave file: {}", path.absolute);

    int samp_rate = 0;
    int16_t bps = 0;
    int16_t n_ch = 0;
    int byte_size = 0;
    ALenum format = 0;

    ALuint buffer;
    alGenBuffers(1, &buffer);

    while (index + 8 <= file.size())
    {
        std::string identifier(4, '\0');
        identifier[0] = file[index++];
        identifier[1] = file[index++];
        identifier[2] = file[index++];
        identifier[3] = file[index++];

        uint32_t chunk_size = *reinterpret_cast<const uint32_t *>(&file[index]);
        index += 4;

        if (index + chunk_size > file.size())
            arcthrow(ARC_FATAL, "invalid chunk size: {}", path.absolute);

        if (identifier == "fmt ")
        {
            if (chunk_size != 16)
                arcthrow(ARC_FATAL, "unknown format: {}", path.absolute);

            int16_t audio_format = *reinterpret_cast<const int16_t *>(&file[index]);
            index += 2;
            if (audio_format != 1)
                arcthrow(ARC_FATAL, "unknown format: {}", path.absolute);

            n_ch = *reinterpret_cast<const int16_t *>(&file[index]);
            index += 2;
            samp_rate = *reinterpret_cast<const int32_t *>(&file[index]);
            index += 4;
            index += 4;
            index += 2;
            bps = *reinterpret_cast<const int16_t *>(&file[index]);
            index += 2;

            if (n_ch == 1)
            {
                if (bps == 8)
                    format = AL_FORMAT_MONO8;
                else if (bps == 16)
                    format = AL_FORMAT_MONO16;
                else
                    arcthrow(ARC_FATAL, "can't play mono " + std::to_string(bps) + " sound.");
            }
            else if (n_ch == 2)
            {
                if (bps == 8)
                    format = AL_FORMAT_STEREO8;
                else if (bps == 16)
                    format = AL_FORMAT_STEREO16;
                else
                    arcthrow(ARC_FATAL, "can't play stereo " + std::to_string(bps) + " sound.");
            }
            else
                arcthrow(ARC_FATAL, "can't play audio with " + std::to_string(n_ch) + " channels");
        }
        else if (identifier == "data")
        {
            byte_size = chunk_size;
            const byte *data = &file[index];
            alBufferData(buffer, format, data, chunk_size, samp_rate);
            index += chunk_size;
        }
        else if (identifier == "JUNK" || identifier == "iXML")
            index += chunk_size;
        else
            index += chunk_size;
    }

    auto ptr = std::make_shared<track>();
    ptr->P_track_id = buffer;
    ptr->sec_len = (double)byte_size / (samp_rate * bps / 8.0) / n_ch;

    return ptr;
}

shared<clip> make_clip(shared<track> track)
{
    unsigned int id;
    alGenSources(1, &id);
    alSourcei(id, AL_BUFFER, track->P_track_id);
    alSourcef(id, AL_ROLLOFF_FACTOR, rolloff);
    alSourcef(id, AL_REFERENCE_DISTANCE, ref_dist);
    alSourcef(id, AL_MAX_DISTANCE, max_dist);

    auto ptr = std::make_shared<clip>();
    ptr->relying_track = track;
    ptr->P_clip_id = id;
    clip_r.push_back(ptr);
    return ptr;
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

} // namespace arcaie::audio
