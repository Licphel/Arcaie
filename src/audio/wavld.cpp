#include <al/al.h>
#include <audio/device.h>
#include <core/io.h>
#include <core/log.h>

namespace arc::audio
{

std::shared_ptr<track> P_wav_load(const path_handle &path)
{
    auto file = io_read_bytes(path);

    size_t index = 0;

    if (file.size() < 12)
        print_throw(ARC_FATAL, "too small file: {}", path.abs_path);

    if (file[index++] != 'R' || file[index++] != 'I' || file[index++] != 'F' || file[index++] != 'F')
        print_throw(ARC_FATAL, "not a wave file: {}", path.abs_path);

    index += 4;

    if (file[index++] != 'W' || file[index++] != 'A' || file[index++] != 'V' || file[index++] != 'E')
        print_throw(ARC_FATAL, "not a wave file: {}", path.abs_path);

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
            print_throw(ARC_FATAL, "invalid chunk size: {}", path.abs_path);

        if (identifier == "fmt ")
        {
            if (chunk_size != 16)
                print_throw(ARC_FATAL, "unknown format: {}", path.abs_path);

            int16_t audio_format = *reinterpret_cast<const int16_t *>(&file[index]);
            index += 2;
            if (audio_format != 1)
                print_throw(ARC_FATAL, "unknown format: {}", path.abs_path);

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
                    print_throw(ARC_FATAL, "can't play mono " + std::to_string(bps) + " sound.");
            }
            else if (n_ch == 2)
            {
                if (bps == 8)
                    format = AL_FORMAT_STEREO8;
                else if (bps == 16)
                    format = AL_FORMAT_STEREO16;
                else
                    print_throw(ARC_FATAL, "can't play stereo " + std::to_string(bps) + " sound.");
            }
            else
                print_throw(ARC_FATAL, "can't play audio with " + std::to_string(n_ch) + " channels");
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

    std::shared_ptr<track> ptr = std::make_shared<track>();
    ptr->P_track_id = buffer;
    ptr->sec_len = (double)byte_size / (samp_rate * bps / 8.0) / n_ch;

    return ptr;
}

} // namespace arc::audio