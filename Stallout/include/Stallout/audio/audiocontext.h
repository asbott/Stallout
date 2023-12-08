#pragma once //

#include "mz_vector.hpp"
#include "Stallout/timing.h"
#include "Stallout/utils.h"

#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif

// TODO: #performance #audio
// Offload OpenAL calls to a Double_Buffered_Thread


struct ALCdevice;
struct ALCcontext;

NS_BEGIN(stallout);
NS_BEGIN(audio);

enum Audio_Format {
    AUDIO_FORMAT_MONO8,
    AUDIO_FORMAT_MONO16,
    AUDIO_FORMAT_STEREO8,
    AUDIO_FORMAT_STEREO16
};

enum Player_State {
    PLAYER_STATE_STOPPED,
    PLAYER_STATE_PAUSED,
    PLAYER_STATE_PLAYING
};

typedef u32 audio_clip_t;
constexpr audio_clip_t NULL_AUDIO_CLIP = 0;

struct Audio_Driver;

// Not thread safe
struct ST_API Audio_Player {
    float pitch              = 1.0f;
    float gain               = 1.0f;
    float min_gain           = 0.0f;
    float max_gain           = 1.0f;
    float max_distance       = FLT_MAX;
    float rolloff_factor     = 1.0f;
    float cone_outer_gain    = 0.0f;
    float cone_inner_angle   = 360.0f;
    float cone_outer_angle   = 360.0f;
    float reference_distance = 1.0f;
    
    bool looping             = false;
    
    mz::fvec3 position =  {0.0f, 0.0f, 0.0f};
    mz::fvec3 velocity =  {0.0f, 0.0f, 0.0f};
    mz::fvec3 direction = {0.0f, 0.0f, 0.0f};

    u32 _source_id;
    Audio_Driver* _context = NULL;

    // Get timer for current time into playback
    Timer get_timer();
    bool is_playing();
    Player_State get_state();
    audio_clip_t get_current_clip();

    // Changes in config needs to be applied to have effect
    // on the audio
    void apply();

    void set_clip(audio_clip_t clip);
    void play(audio_clip_t clip);
    void play();
    void pause();
    void stop();
};


struct ST_API Audio_Driver {

    Audio_Driver(const char* device_name = NULL);
    ~Audio_Driver();

    audio_clip_t create_clip(void* data, Audio_Format format, u64 frame_count, u32 sample_rate);
    audio_clip_t create_clip(void* data, u32 channels, u32 bits_per_sample, u64 frame_count, u32 sample_rate);
    audio_clip_t create_clip_from_file(const char* filename);
    
    void destroy_clip(audio_clip_t clip);

    Audio_Player* create_player();
    void destroy_player(Audio_Player* player);

    // Play with default player
    void play(audio_clip_t clip, mz::fvec3 position = 0);
    // Pause default player
    void pause();
    // Stop default player
    void stop();

    ALCdevice* _device;
    ALCcontext* _context;
    Audio_Player _default_player;
};

NS_END(audio);
NS_END(stallout);