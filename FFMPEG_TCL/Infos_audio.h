#ifndef __ALX_INFO_BUFFER_AUDIO__
#define __ALX_INFO_BUFFER_AUDIO__

//#include <ffmpegvideo.h>
#include <stdint.h>
class Mutex;

const int ALX_INFO_BUFFER_AUDIO_TAILLE_BUFFER = 524288;

struct Info_buffer_audio {
   unsigned int size, deb, duration;
   int64_t pts; int64_t dts;
   char *buffer;
};
 extern "C"
 {
	inline __declspec(dllexport) void Info_buffer_audio_Init (Info_buffer_audio *iba);
	inline __declspec(dllexport) void Info_buffer_audio_Maj(Info_buffer_audio *iba, unsigned int s/*, char*b*/, unsigned int d);
	inline __declspec(dllexport) void Info_buffer_audio_Copy(Info_buffer_audio *iba, const Info_buffer_audio *src);
 }

// For sound callback
const int ALX_INFO_BUFFER_AUDIO_NB_BUFFER = 1024;
struct Info_for_sound_CB
{Mutex *mutex;
 
 Info_buffer_audio Tab_wav[ALX_INFO_BUFFER_AUDIO_NB_BUFFER];
 char internal_buffer[ALX_INFO_BUFFER_AUDIO_TAILLE_BUFFER];
 unsigned int nb_buffers, first, last, nb, size_buffers, last_buffer_index;
 unsigned int audio_sample_rate;
 double       video_sample_rate;
 double       s_SynchronisationThreshold;
 bool is_audio_signed;
 unsigned int size_audio_sample, nb_channels;
 //int64_t /*time_base_audio,*/ time_base_video;
 int64_t video_pts;
 double time_base_audio, time_base_video;

 bool has_skiped, not_enough;
};

 extern "C"
 {
	inline __declspec(dllexport) void Info_for_sound_CB_Init(Info_for_sound_CB *ifscb, Mutex *m);
	inline __declspec(dllexport) Mutex* Info_for_sound_CB_Get_mutex (Info_for_sound_CB *ifscb);
	inline __declspec(dllexport) const unsigned int Info_for_sound_CB_Nb_buffers(Info_for_sound_CB *ifscb);
	inline __declspec(dllexport) const unsigned int Info_for_sound_CB_Nb_pkt    (Info_for_sound_CB *ifscb);
	inline __declspec(dllexport) const unsigned int Info_for_sound_CB_Size_buffers(Info_for_sound_CB *ifscb);
	inline __declspec(dllexport) void Info_for_sound_CB_Put_New(Info_for_sound_CB *ifscb, const void *buf, const int size, const int64_t pts, const int64_t, const int duration);
	inline __declspec(dllexport) void Info_for_sound_CB_Release(Info_for_sound_CB *ifscb);
	inline __declspec(dllexport) void Info_for_sound_CB_Maj_size(Info_for_sound_CB *ifscb, const unsigned int index, const unsigned int d, const unsigned int s);

	inline __declspec(dllexport) void Info_for_sound_CB_Lock  (Info_for_sound_CB *ifscb);
	inline __declspec(dllexport) void Info_for_sound_CB_UnLock(Info_for_sound_CB *ifscb);

	inline __declspec(dllexport) const int Info_for_sound_CB_First_index(Info_for_sound_CB *ifscb);
	inline __declspec(dllexport) const int Info_for_sound_CB_Read(Info_for_sound_CB *ifscb, void *buff, const int dec_buf, const int len);

	inline __declspec(dllexport) const bool Info_for_sound_CB_Synch_audio_to_video(Info_for_sound_CB *ifscb, const int len);

	inline __declspec(dllexport) const double Info_for_sound_get_Synchronisation_threshold(const Info_for_sound_CB *ifscb);
	inline __declspec(dllexport) void Info_for_sound_set_Synchronisation_threshold(Info_for_sound_CB *ifscb, const double v);

	inline __declspec(dllexport) void Info_for_sound_Drain_all(Info_for_sound_CB *ifscb);
 }

#endif
