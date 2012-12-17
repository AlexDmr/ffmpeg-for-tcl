#ifndef STREAMS_FFMPEG_INIT_H
#define STREAMS_FFMPEG_INIT_H

#ifndef __BC_COMPILER_PIPO_PROTECTION__
	#define __STDC_CONSTANT_MACROS
	#define inline _inline

//#include <dll.h>

//	#include "C:\FMOD\api\inc\fmod.h"
	//#include <utils.h>

extern "C" {
	#include <tcl.h>
	#include <fmod.h>
	  #include <libavformat\avformat.h>
	  #include <libavcodec\avcodec.h>
	}

	#include <Infos_audio.h>

#endif


	// Initialize the ffmpeg codecs.
    void ffmpeg_init();
    
	const int FFMPEG_FSOUND_GetBufferLengthTotal();

	const int FFMPEG_Open_video_stream (const char *f_name);
	const int FFMPEG_Close_video_stream(const int num_stream);

	const char* FFMPEG_get_error_message(const int num_stream);

	const char* FFMPEG_get_video_codec_name     (const int num_stream);
	const char* FFMPEG_get_video_codec_long_name(const int num_stream);
	const char* FFMPEG_get_audio_codec_name     (const int num_stream);
	const char* FFMPEG_get_audio_codec_long_name(const int num_stream);


	const bool FFMPEG_Stream_exists(const int id);
	const int FFMPEG_startAcquisition(const int num_stream);
	void FFMPEG_stopAcquisition(const int num_stream);

	const bool FFMPEG_getImage  (const int num_stream, void *img);
	const bool FFMPEG_getImageNr(const int num_stream, unsigned long frame, void *img);
	const bool FFMPEG_seek      (const int num_stream, unsigned long frame, bool iframe, int sm);

	const unsigned int FFMPEG_getSoundPosition(FSOUND_STREAM *stream);
	void FFMPEG_getSoundPosition(FSOUND_STREAM *stream, const unsigned int pos);	

	const int FFMPEG_getSoundTime(FSOUND_STREAM *stream);
	void FFMPEG_setSoundTime(FSOUND_STREAM *stream, const int pos);

	const double FFMPEG_get_Video_time_base(const int num_stream);
	const unsigned int FFMPEG_get_Video_pts(const int num_stream);

	const unsigned int FFMPEG_get_Video_pts_for_audio(const int num_stream);
	void FFMPEG_set_Video_pts_for_audio(const int num_stream, const unsigned int v);

	const unsigned int FFMPEG_get_nb_total_video_frames(const int num_stream);
	const int   FFMPEG_numFrames(const int num_stream);
	const double FFMPEG_getFramerate(const int num_stream);
	const double FFMPEG_setFramerate(const int num_stream, const double v);

	const int FFMPEG_Width (const int num_stream);
	const int FFMPEG_Height(const int num_stream);

	const double FFMPEG_get_audio_clock_start(const int num_stream);
	const double FFMPEG_get_first_time(const int num_stream);
	const double FFMPEG_get_delta_from_first_time(const int num_stream);

	const long FFMPEG_get_num_last_buffer(const int num_stream);


	const char* FFMPEG_information(const int num_stream);

	extern "C"
	{
		int /*DLLEXPORT*/ Ffmpeg_for_tcl_SafeInit(Tcl_Interp *interp);
		int /*DLLEXPORT*/ Ffmpeg_for_tcl_Init(Tcl_Interp *interp);
	}

	void* FFMPEG_Get_a_buffer(const unsigned int nb_bytes);
	void  FFMPEG_Release_buffer(void *buff);

	//void  FFMPEG_Synchronize_audio_with_video(const int num_stream);

	const bool FFMPEG_get_Debug_mode(const int num_stream);
	void FFMPEG_set_Debug_mode(const int num_stream, const bool b);

	//const double FFMPEG_get_Synchronisation_threshold(const int num_stream);
	//void FFMPEG_set_Synchronisation_threshold(const int num_stream, const double v);

	Info_for_sound_CB* FFMPEG_Info_for_sound_CB      (const int num_stream);
	const unsigned int FFMPEG_Sound_sample_rate      (const int num_stream);
	const unsigned int FFMPEG_Nb_channels            (const int num_stream);
	const unsigned int FFMPEG_Audio_buffer_size		 (const int num_stream);

	void FFMPEG_Info_for_sound_Drain_all             (const int num_stream);

	void FFMPEG_Lock  (const int num_stream);
	void FFMPEG_UnLock(const int num_stream);

	//signed char F_CALLBACKAPI FFMPEG_FMOD_Stream_Info_audio(FSOUND_STREAM *stream, void *buff, int len, void *userdata);
	//signed char F_CALLBACKAPI FFMPEG_FMOD_Stream_Info_audio(FSOUND_STREAM *stream, void *buff, int len, void *userdata);
	FSOUND_STREAMCALLBACK Get_FFMPEG_FMOD_Stream_Info_audio();

    int FFMPEG_Convert_void_to_binary_tcl_var(void *ptr_void, int length, char *var_name, const bool append);


	// Audio streaming part with FMOD
	void FFMPEG_FSOUND_Init();
    const unsigned int FFMPEG_FSOUND_Mono    ();
    const unsigned int FFMPEG_FSOUND_Stereo  ();
	const unsigned int FFMPEG_FSOUND_16b     ();
    const unsigned int FFMPEG_FSOUND_8b      ();
    const unsigned int FFMPEG_FSOUND_unsigned();
    const unsigned int FFMPEG_FSOUND_signed  ();

	const int FFMPEG_Commit_audio_buffers(const int num_stream);
	
	const int FFMPEG_get_nb_temp_audio_buffers(const int num_stream);

    const int FFMPEG_get_volume_of_canal(const int canal);
         void FFMPEG_set_volume_of_canal(const int canal, const int vol);

	FSOUND_STREAM* FFMPEG_Get_a_new_FSOUND_STREAM( FSOUND_STREAMCALLBACK callback
                                                 , int lenbytes
                                                 , unsigned int mode
                                                 , int samplerate
                                                 , void *userdata );
	const int FFMPEG_FSOUND_STREAM_GetOpenState(FSOUND_STREAM *audio_strm);
    const int FFMPEG_FSOUND_STREAM_Play(Info_for_sound_CB *ifscb, FSOUND_STREAM *audio_strm, int canal);
	const int FFMPEG_FSOUND_STREAM_Stop(Info_for_sound_CB *ifscb, FSOUND_STREAM *audio_strm);
    const bool FFMPEG_close_FSOUND_STREAM(FSOUND_STREAM *strm);

	static int Void2Photo (void *imgVector, 
                       const char *photoName, int width, int height,
                       int numChans);

#endif
