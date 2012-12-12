#ifndef FFMPEGVIDEO_H
#define FFMPEGVIDEO_H

#define inline _inline
#define __STDC_CONSTANT_MACROS
#include <stdafx.h>
#include <string>
#include <windows.h>
//#include <Image.h>
#include <utilitaires/alx_liste.h>

//DEBUG #include <utils.h>
#include <slimthread.h>
// Video management
typedef enum
{
	ABS,
	FORWARD,
	BACKWARD,
	END
} SeekMode; 




#include <iostream>
using namespace std;

extern "C" {
  #include <libavcodec\avcodec.h>
  #include <libavformat/avformat.h>
  #include "libavformat/avio.h"
  #include <libswscale/swscale.h>
}

#include <Infos_audio.h>


class FFMpegVideo {
private:
	unsigned int video_pts;
	struct SwsContext *img_convert_ctx;
	std::string error_msg;

public :
	FFMpegVideo();
	FFMpegVideo(const char *fname);
	FFMpegVideo(const std::string & fname);
	~FFMpegVideo();

public:
	int startAcquisition();
	void stopAcquisition();

	const char* get_error_message() const {return error_msg.c_str();}

	inline const double get_audio_clock_start() const {return info_for_sound_CB.audio_clock_start;}

	inline const clock_t get_first_time() const {return info_for_sound_CB.first_t;}
	const double get_delta_from_first_time() const;
	inline const clock_t get_time_t0() const {return info_for_sound_CB.t0;}
	void init_time_t0();
	void set_time_t0_now();
	void set_time_t0_from_video();
	const double get_delta_from_t0();

	const char* get_video_codec_name     () const {if (codec) return codec->name     ; else return "";}
	const char* get_video_codec_long_name() const {if (codec) return codec->long_name; else return "";}
	const char* get_audio_codec_name     () const {if (audio_codec) return audio_codec->name     ; else return "";}
	const char* get_audio_codec_long_name() const {if (audio_codec) return audio_codec->long_name; else return "";}

	bool getImage(/*util::ByteImage & img*/void *img);
	bool getImageNr(unsigned long frame, void *img/*util::ByteImage & img*/);
	bool seek(unsigned long frame, bool iframe = false, SeekMode sm = ABS);

	const int   numFrames()      {return nrFrames;  }

	const unsigned int Nb_total_video_frames() const {return nb_total_video_frames;}

	const double getFramerate( ) {return mFramerate;}
	const double setFramerate(const double v) {return mFramerate = v;}

	inline const double    Video_time_base() const {return time_base_video;}
	inline const unsigned int    Video_pts() const {return video_pts;}
	inline void Video_pts(const unsigned int v) {this->video_pts = v;}

	inline const unsigned int Video_pts_for_audio      () {return info_for_sound_CB.video_pts;}
	inline void Video_pts_for_audio(const unsigned int v) {info_for_sound_CB.video_pts = v;}

	inline const int Width () const {return width;}
	inline const int Height() const {return height;}

	inline const bool Debug_mode() const {return debug_mode;}
	inline void Debug_mode(const bool b) {debug_mode = b;}

	inline void IFS_Drain_all() {Info_for_sound_Drain_all(&info_for_sound_CB); init_time_t0();}

	//inline void Synchronize_audio_with_video() {info_for_sound_CB.synchronize_with_video = true;}

	inline long get_num_last_buffer() const {return info_for_sound_CB.num_last_buffer;}

	void Lock();
	void UnLock();

	//const double get_Synchronisation_threshold() const;
	//void set_Synchronisation_threshold(const double v);

	inline const unsigned int Nb_channels() const {return nb_channels;}
	inline Info_for_sound_CB* get_Info_for_sound_CB() {return &info_for_sound_CB;}
	inline const unsigned int Sound_sample_rate() const {return sound_sample_rate;}
	inline const unsigned int Audio_buffer_size() const {return size_audio_buf;}
	inline void Audio_buffer_size(const unsigned int v) {size_audio_buf = v;
														 if(audio_buf == (uint8_t*)NULL) {free(audio_buf);}
														 audio_buf = (uint8_t*)av_malloc(v);
														}

public:
	const bool open(const char *fname);
	const bool open(const std::string & fname);
	void close();
		
public:
	std::string information() const;

private:
	bool getNextFrame();

	private:
		AVFormatContext *formatCtx;
		AVCodecContext *codecCtx, *audio_codecCtx;
		AVCodec *codec;
		int videoStream;
		int audioStream;
		std::string id;
		int nrFrames;
		bool started;
		int curFrame;
		int width, height;
		double mFramerate, time_base_video;
		unsigned int nb_total_video_frames;

		AVFrame *frame;
		bool debug_mode;

	   // Audio
		AVCodec *audio_codec;
		Mutex mutex_audio;
		Info_for_sound_CB info_for_sound_CB;
		Info_buffer_audio info_buffer_audio_tmp;
        alx_liste<AVPacket>          L_audio_pkt;
		
		unsigned int size_audio_buf, sound_sample_rate, nb_channels;
		uint8_t *audio_buf;
		int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size, AVPacket *pkt);
		int get_audio_packet(AVPacket *pkt);
		int put_audio_packet(AVPacket *pkt);
		void Process_audio_packets();
		//PacketQueue audioq;


};

#endif