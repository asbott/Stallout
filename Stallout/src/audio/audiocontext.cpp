#include "pch.h"

#include "Stallout/audio/audiocontext.h"

#include "Stallout/logger.h"
#include "Stallout/utils.h"

#include "Stallout/memory.h"

#include <AL/al.h>
#include <AL/alc.h>



bool check_al_errors(const char* filename, u32 line)
{
    (void)filename;(void)line;
    ALenum error = alGetError();
    if(error != AL_NO_ERROR)
    {
        #define fmt "OpenAL ERROR ({}:{})\n{}"
        switch(error)
        {
        case AL_INVALID_NAME:
            log_error(fmt, filename, line, "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function");
            break;
        case AL_INVALID_ENUM:
            log_error(fmt, filename, line, "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function");
            break;
        case AL_INVALID_VALUE:
            log_error(fmt, filename, line, "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function");  
            break;
        case AL_INVALID_OPERATION:
            log_error(fmt, filename, line, "AL_INVALID_OPERATION: the requested operation is not valid");  
            break;
        case AL_OUT_OF_MEMORY:
            log_error(fmt, filename, line, "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory");  
            break;
        default:
            log_error(fmt, filename, line, error);  
        }
        return false;
    }
    return true;
}

#ifdef _ST_CONFIG_DEBUG
// Overload for non-void functions
template<typename Func, typename Ret = std::invoke_result_t<Func>>
std::enable_if_t<!std::is_void_v<Ret>, Ret> al_call_impl(Func&& func, const char* filename, int line) {
    Ret ret = func();
    ST_ASSERT(check_al_errors(filename, line), "OpenAL Error ^^");
    return ret;
}

// Overload for void functions
template<typename Func, typename Ret = std::invoke_result_t<Func>>
std::enable_if_t<std::is_void_v<Ret>, void> al_call_impl(Func&& func, const char* filename, int line) {
    func();
    ST_ASSERT(check_al_errors(filename, line), "OpenAL Error ^^");
}

// The macro itself
#define AL_CALL(call) al_call_impl([&]() { return call; }, __FILE__, __LINE__)

#else
#define AL_CALL(call) call
#endif



NS_BEGIN(stallout);
NS_BEGIN(audio);

using namespace utils;

Timer Audio_Player::get_timer() {
    ST_ASSERT(AL_CALL(alIsSource(_source_id)), "Invalid source");
    ALfloat offset;
    AL_CALL(alGetSourcef(_source_id, AL_SEC_OFFSET, &offset));

    Timer timer;
    timer.set_time(offset);

    if (this->get_state() != PLAYER_STATE_PLAYING) {
        timer.pause();
    }

    return timer;
}
bool Audio_Player::is_playing() {
    return this->get_state() == PLAYER_STATE_PLAYING;

}
Player_State Audio_Player::get_state() {
    ST_ASSERT(AL_CALL(alIsSource(_source_id)), "Invalid source");
    ALint source_state;
    AL_CALL(alGetSourcei(_source_id, AL_SOURCE_STATE, &source_state));

    switch (source_state) {
        case AL_INITIAL:
        case AL_STOPPED:
            return PLAYER_STATE_STOPPED;
        case AL_PAUSED:
            return PLAYER_STATE_PAUSED;
        case AL_PLAYING:
            return PLAYER_STATE_PLAYING;
        default:
            INTENTIONAL_CRASH("Invalid player state");
            return PLAYER_STATE_PLAYING;
    }
}

audio_clip_t Audio_Player::get_current_clip() {
    audio_clip_t current_clip = 0;
    AL_CALL(alGetSourcei(_source_id, AL_BUFFER, (ALint*)&current_clip));

    return current_clip;
}

void Audio_Player::apply() {
    AL_CALL(alSourcei(_source_id, AL_LOOPING, looping));
    AL_CALL(alSourcef(_source_id, AL_PITCH, pitch));
    AL_CALL(alSourcef(_source_id, AL_GAIN, gain));
    AL_CALL(alSourcef(_source_id, AL_MIN_GAIN, min_gain));
    AL_CALL(alSourcef(_source_id, AL_MAX_GAIN, max_gain));
    AL_CALL(alSourcef(_source_id, AL_MAX_DISTANCE, max_distance));
    AL_CALL(alSourcef(_source_id, AL_ROLLOFF_FACTOR, rolloff_factor));
    AL_CALL(alSourcef(_source_id, AL_CONE_OUTER_GAIN, cone_outer_gain));
    AL_CALL(alSourcef(_source_id, AL_CONE_INNER_ANGLE, cone_inner_angle));
    AL_CALL(alSourcef(_source_id, AL_CONE_OUTER_ANGLE, cone_outer_angle));
    AL_CALL(alSourcef(_source_id, AL_REFERENCE_DISTANCE, reference_distance));
    AL_CALL(alSource3f(_source_id, AL_POSITION, position.x, position.y, position.z));
    AL_CALL(alSource3f(_source_id, AL_VELOCITY, velocity.x, velocity.y, velocity.z));
    AL_CALL(alSource3f(_source_id, AL_DIRECTION, direction.x, direction.y, direction.z));
}
void Audio_Player::set_clip(audio_clip_t clip) {
    AL_CALL(alSourcei(_source_id, AL_BUFFER, clip));
}
void Audio_Player::play(audio_clip_t clip) {
    ST_ASSERT(_context, "Invalid audio player; it needs to be created through a context");
    AL_CALL(alcMakeContextCurrent(_context->_context));
    ST_ASSERT(alIsSource(_source_id), "Invalid OpenAL source ID");
    ST_ASSERT(alIsBuffer(clip), "Invalid OpenAL buffer ID");

    this->stop();

    this->apply();
    this->set_clip(clip);

    // Play the source
    AL_CALL(alSourcePlay(_source_id));
}
void Audio_Player::play() {
    ST_ASSERT(_context, "Invalid audio player; it needs to be created through a context");
    ST_ASSERT(AL_CALL(alIsSource(_source_id)), "Invalid source");
    
    if (this->get_current_clip() == NULL_AUDIO_CLIP) {
        log_warn("Called play() on a player with no set audio clip");
        return;
    }

    AL_CALL(alSourcePlay(_source_id));
}
void Audio_Player::pause() {
    ST_ASSERT(_context, "Invalid audio player; it needs to be created through a context");
    AL_CALL(alcMakeContextCurrent(_context->_context));
    ST_ASSERT(alIsSource(_source_id), "Invalid OpenAL source ID");
    AL_CALL(alSourcePause(_source_id));
}
void Audio_Player::stop() {
    ST_ASSERT(_context, "Invalid audio player; it needs to be created through a context");
    AL_CALL(alcMakeContextCurrent(_context->_context));
    ST_ASSERT(alIsSource(_source_id), "Invalid OpenAL source ID");
    AL_CALL(alSourceStop(_source_id));
}



Audio_Driver::Audio_Driver(const char* device_name) {
    _device = alcOpenDevice(device_name);

    ST_ASSERT(_device, "Failed opening AL device");

    _context = alcCreateContext(_device, NULL);
    AL_CALL(alcMakeContextCurrent(_context));

    log_info("Initialized audio context with device {}", alcGetString(NULL, ALC_DEFAULT_ALL_DEVICES_SPECIFIER));

    AL_CALL(alGenSources(1, &_default_player._source_id));  
    _default_player._context = this;
}

Audio_Driver::~Audio_Driver() {
    AL_CALL(alcMakeContextCurrent(_context));
    AL_CALL(alcDestroyContext(_context));
    AL_CALL(alcCloseDevice(_device));

    log_info("Audio context successfully shut down");
}

audio_clip_t Audio_Driver::create_clip(void* data, Audio_Format format, u64 frame_count, u32 sample_rate) {
    u32 channels = 0, bits_per_sample = 0;
    switch (format) {
        case AUDIO_FORMAT_MONO8:    channels = 1; bits_per_sample = 8; break;
        case AUDIO_FORMAT_MONO16:   channels = 1; bits_per_sample = 16; break;
        case AUDIO_FORMAT_STEREO8:  channels = 2; bits_per_sample = 8; break;
        case AUDIO_FORMAT_STEREO16: channels = 2; bits_per_sample = 16; break;
        default: INTENTIONAL_CRASH("Unhandled enum"); break;
    }
    return this->create_clip(data, channels, bits_per_sample, frame_count, sample_rate);
}

audio_clip_t Audio_Driver::create_clip(void* data, u32 channels, u32 bits_per_sample, u64 frame_count, u32 sample_rate) {
    AL_CALL(alcMakeContextCurrent(_context));

    ALuint buffer;
    AL_CALL(alGenBuffers(1, &buffer));

    ALenum format = 0;
    if(channels == 1 && bits_per_sample == 8)
        format = AL_FORMAT_MONO8;
    else if(channels == 1 && bits_per_sample == 16)
        format = AL_FORMAT_MONO16;
    else if(channels == 2 && bits_per_sample == 8)
        format = AL_FORMAT_STEREO8;
    else if(channels == 2 && bits_per_sample == 16)
        format = AL_FORMAT_STEREO16;
    else
    {
        INTENTIONAL_CRASH("Invalid bits per sample ({})", bits_per_sample);
    }

    AL_CALL(alBufferData(buffer, format, data, (ALsizei)(frame_count * channels * (bits_per_sample / 8)), sample_rate));
    
    return (audio_clip_t)buffer;
}
audio_clip_t Audio_Driver::create_clip_from_file(const char* filename) {
    u32 channels, sample_rate, bits_per_sample;
    u64 frame_count;
    void* sound_data = utils::load_audio_from_file(filename, &channels, &sample_rate, &bits_per_sample, &frame_count);
    if (!sound_data) return NULL_AUDIO_CLIP;
    return this->create_clip(sound_data, channels, bits_per_sample, frame_count, sample_rate);
}

void Audio_Driver::destroy_clip(audio_clip_t clip) {
    AL_CALL(alcMakeContextCurrent(_context));
    ST_ASSERT(AL_CALL(alIsBuffer(clip)), "Invalid audio clip handle");
    alDeleteBuffers(1, &clip);
    auto err = alGetError();
    (void)err;
    ST_ASSERT(err != AL_INVALID_OPERATION, "Tried destroying a clip that's in use; make sure it's stopped before destroying.");
    ST_ASSERT(err == AL_NO_ERROR, "OpenAL Error");
}

Audio_Player* Audio_Driver::create_player() {
    // TODO #performance? 
    // Maybe allocate contigiously but also probably doesn't really
    // matter because there's no reason to process several players
    // so won't really avoid cache misses and the memory limit is
    // quite annoying and can't use pointers if it's just an Array

    Audio_Player* player = ST_NEW(Audio_Player);

    AL_CALL(alcMakeContextCurrent(_context));
    AL_CALL(alGenSources(1, &player->_source_id));
    player->_context = this;

    return player;
}
void Audio_Driver::destroy_player(Audio_Player* player) {
    AL_CALL(alcMakeContextCurrent(_context));
    ST_ASSERT(AL_CALL(alIsSource(player->_source_id)), "Invalid player");
    alDeleteSources(1, &player->_source_id);
}

void Audio_Driver::play(audio_clip_t clip, mz::fvec3 position) {
    _default_player.position = position;
    _default_player.play(clip);
}

void Audio_Driver::pause() {
    _default_player.pause();
}

void Audio_Driver::stop() {
    _default_player.stop();
}

NS_END(audio);
NS_END(stallout);