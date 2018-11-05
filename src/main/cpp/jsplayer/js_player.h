#ifndef JS_PLAYER_H
#define JS_PLAYER_H

#include "decoder/js_media_decoder.h"
#include "audio/js_audio_player.h"
#include "render/js_egl_renderer.h"
#include "util/js_queue.h"
#include "js_constant.h"

//time_base AV_TIME_BASE_Q

#define DEFAULT_PREPARE_TIME_OUT_MICROSECONDS               10000000//10s
#define DEFAULT_READ_PKT_TIME_OUT_MICROSECONDS              10000000//10s
#define NO_TIME_OUT_MICROSECONDS                            0

#define DEFAULT_MIN_CACHED_DURATION_LIVE                    0
#define DEFAULT_MAX_CACHED_DURATION_LIVE                    500000//500ms
#define DEFAULT_MIN_DECODED_DURATION_LIVE                   0
#define DEFAULT_MAX_DECODED_DURATION_LIVE                   500000//500ms

#define DEFAULT_MIN_CACHED_DURATION_RECORD                  1000000*10//10s
#define DEFAULT_MAX_CACHED_DURATION_RECORD                  1000000*60*3/2//1.5min
#define DEFAULT_MIN_DECODED_DURATION_RECORD                 0
#define DEFAULT_MAX_DECODED_DURATION_RECORD                 1000000//1s


class JSPlayer;

typedef void(JSPlayer::*CACHE_PACKET_FUNC)(AVPacket *avpkt);

class JSPlayer {

/***
 * ********************************
 * funcs
 * ********************************
 */
public:

    JSPlayer(jobject java_js_player);

    ~JSPlayer();

    /*basic controller*/

    void prepare();

    void play(bool is_to_resume);

    void pause();

    void resume();

    void stop();

    void reset();

    void destroy();

    /*prepare*/

    JS_RET find_stream_info();

    JS_RET prepare_audio();

    JS_RET prepare_video();

    void init_options();

    /*play*/

    void start_read_frame();

    void start_decode_audio();

    void start_decode_video();

    /*reset*/

    void stop_read_frame();

    void stop_play(bool is_to_pause);

    void stop_prepare();

    void free_res();

    /*others*/

    void set_cur_timing();

    void set_is_intercept_audio(bool is_intercept_audio);

    void on_surface_hold_state_changed();


    CACHE_PACKET_FUNC m_cache_audio_packet;
    CACHE_PACKET_FUNC m_cache_video_packet;

    /*save video packet way*/

    void cache_record_video_packet(AVPacket *src);

    void cache_live_video_packet_hold_surface(AVPacket *src);

    void cache_live_video_packet_not_hold_surface(AVPacket *src);

    /*save audio packet way*/

    void cache_record_audio_packet(AVPacket *src);

    void cache_live_audio_packet(AVPacket *src);

/***
 * ********************************
 * fields
 * ********************************
 */

    /*basic*/

    const char *m_url = NULL;
    bool m_is_live = false;
    volatile int m_cur_play_status = 0;
    AVDictionary *m_options = NULL;

    jobject m_java_js_player = NULL;
    JSAudioPlayer *m_audio_player = NULL;
    JSEglRenderer *m_egl_renderer = NULL;
    JSMediaDecoder *m_js_media_decoder = NULL;
    JSEventHandler *m_js_event_handler = NULL;


    pthread_t m_prepare_tid,
            m_read_packet_tid,
            m_decode_video_tid,
            m_decode_audio_tid,
            m_play_audio_tid;


    AVFormatContext *m_format_ctx = NULL;
    int m_video_stream_index = -1;
    int m_audio_stream_index = -1;
    AVStream *m_audio_stream = NULL;
    AVStream *m_video_stream = NULL;
    bool m_has_audio_stream = false;
    bool m_has_video_stream = false;
    JSQueue<AVPacket *> *m_audio_cached_que = NULL, *m_video_cached_que = NULL;
    JSQueue<AVFrame *> *m_audio_decoded_que = NULL, *m_video_decoded_que = NULL;

    int64_t m_timing = 0;
    int m_io_time_out = 0;
    volatile bool m_is_intercept_audio = false;

    volatile bool m_is_parse_data_from_video_packet = false;

    int64_t m_min_cached_duration = -1;
    //live buffer drop frame threshold.
    int64_t m_max_cached_duration = -1;

    int64_t m_min_decoded_duration = -1;
    int64_t m_max_decoded_duration = -1;


    volatile bool m_is_playing = false;
    volatile bool m_is_reading_frame = false;
    volatile bool m_is_audio_data_consuming = false;


    /*func pointers*/

    void (*m_native_intercepted_pcm_data_callback)(jlong native_js_player,
                                                   short *pcm_data,
                                                   int sample_num,
                                                   int channel_num);

    void (*m_native_parse_data_from_video_packet_callback)(uint8_t *data,
                                                           int size);

    /*sync audio and video control field*/

};

/***
 * ********************************
 * non-member func
 * ********************************
 */

/* threads */

static void *prepare_thread(void *data);

static void *play_audio_thread(void *data);

static void *read_frame_thread(void *data);

static void *decode_video_thread(void *data);

static void *decode_audio_thread(void *data);


/*callback*/

static int io_interrupt_cb(void *data);

static void opensles_buffer_queue_cb(SLAndroidSimpleBufferQueueItf caller,
                                     void *data);

static void egl_buffer_queue_cb(void *data);


/*consume audio data way*/

static void consume_audio_data_by_interceptor(JSPlayer *player);

static void consume_audio_data_by_audiotrack(JSPlayer *player);

static void consume_audio_data_by_opensles(JSPlayer *player);


/*others*/

static inline bool is_live(AVFormatContext *s);


#endif //JS_PLAYER_H