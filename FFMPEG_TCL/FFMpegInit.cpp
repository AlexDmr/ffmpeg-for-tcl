#include "stdafx.h"
#define inline _inline
#include "FFMpegInit.h"


//__________________

//__________________



extern "C" {
  #include <libavformat\avformat.h>
}

#include <ffmpegvideo.h>

Tcl_Interp *FFMPEG_Tcl_Interp;

#include "SWIG/ffmpeg_dll_wrap.cxx"
// XXX TO BE ADDED AFTER GENERATION : FFMPEG_Tcl_Interp = interp;

const unsigned int NB_MAX_P_FFMpegVideo = 256;
FFMpegVideo *Tab_P_FFMpegVideo[NB_MAX_P_FFMpegVideo];
bool      Tab_used_FFMpegVideo[NB_MAX_P_FFMpegVideo];

Tcl_Obj *Ffmpeg_for_tcl_TCL_obj_array;

//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
int Ffmpeg_for_tcl_SafeInit(Tcl_Interp *interp) {Ffmpeg_for_tcl_TCL_obj_array = NULL; FFMPEG_Tcl_Interp = interp; return SWIG_init(interp);}
int Ffmpeg_for_tcl_Init(Tcl_Interp *interp)     {Ffmpeg_for_tcl_TCL_obj_array = NULL; FFMPEG_Tcl_Interp = interp; return SWIG_init(interp);}

//______________________________________________________________________________
void* FFMPEG_Get_a_buffer(const unsigned int nb_bytes) {return (void*)av_malloc(nb_bytes*sizeof(uint8_t));}
void  FFMPEG_Release_buffer(void *buff) {av_free(buff);}

//______________________________________________________________________________
const bool FFMPEG_get_Debug_mode(const int num_stream)         {
if(FFMPEG_Stream_exists(num_stream))
  {return Tab_P_FFMpegVideo[num_stream]->Debug_mode();}
 return false;
}

//______________________________________________________________________________
void FFMPEG_set_Debug_mode(const int num_stream, const bool b) {
if(FFMPEG_Stream_exists(num_stream))
  {Tab_P_FFMpegVideo[num_stream]->Debug_mode(b);}
}

//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
	void ffmpeg_init() {
		static bool initialized = false;
		if ( ! initialized ) {
			//avcodec_register_all();
			avcodec_register_all();

			av_register_all();
			av_log_set_level(AV_LOG_ERROR);
			initialized = true;
			for(unsigned int i=0; i<NB_MAX_P_FFMpegVideo; i++)
			 {Tab_P_FFMpegVideo   [i] = (FFMpegVideo*)NULL;
			  Tab_used_FFMpegVideo[i] = false;
			 }
		}
	}

//______________________________________________________________________________
const int FFMPEG_get_nb_temp_audio_buffers(const int num_stream) {
	if(FFMPEG_Stream_exists(num_stream)) {
		Info_for_sound_CB *ifscb = Tab_P_FFMpegVideo[num_stream]->get_Info_for_sound_CB();
		return ifscb->last_temp - ifscb->last;
	  }
	return -1;
}

//______________________________________________________________________________
const int FFMPEG_Commit_audio_buffers(const int num_stream) {
	if(FFMPEG_Stream_exists(num_stream)) {
		Info_for_sound_CB_Commit_buffers( Tab_P_FFMpegVideo[num_stream]->get_Info_for_sound_CB() );
		return Tab_P_FFMpegVideo[num_stream]->get_Info_for_sound_CB()->nb;
	  }
	return -1;
}

//______________________________________________________________________________
const char* FFMPEG_get_video_codec_name     (const int num_stream) {
	if(FFMPEG_Stream_exists(num_stream)) {
		return Tab_P_FFMpegVideo[num_stream]->get_video_codec_name();
	  }
	 return "";
}

//______________________________________________________________________________
const char* FFMPEG_get_video_codec_long_name(const int num_stream) {
	if(FFMPEG_Stream_exists(num_stream)) {
		return Tab_P_FFMpegVideo[num_stream]->get_video_codec_long_name();
	  }
	 return "";
}

//______________________________________________________________________________
const char* FFMPEG_get_audio_codec_name     (const int num_stream) {
	if(FFMPEG_Stream_exists(num_stream)) {
		return Tab_P_FFMpegVideo[num_stream]->get_audio_codec_name();
	  }
	 return "";
}

//______________________________________________________________________________
const char* FFMPEG_get_audio_codec_long_name(const int num_stream) {
	if(FFMPEG_Stream_exists(num_stream)) {
		return Tab_P_FFMpegVideo[num_stream]->get_audio_codec_long_name();
	  }
	 return "";
}

//______________________________________________________________________________
const bool FFMPEG_Stream_exists(const int id)
{return (id>=0 && id<NB_MAX_P_FFMpegVideo && Tab_used_FFMpegVideo[id] && Tab_P_FFMpegVideo[id]);
}

//______________________________________________________________________________
const int FFMPEG_Get_a_stream_id()
{for(unsigned int i=0; i<NB_MAX_P_FFMpegVideo; i++)
  {if(!Tab_used_FFMpegVideo[i])
    {Tab_used_FFMpegVideo[i] = true;
     return i;
    }
  }
 return -1;
}

//______________________________________________________________________________
void FFMPEG_Release_stream_id(const int id)
{Tab_P_FFMpegVideo   [id] = (FFMpegVideo*)NULL;
 Tab_used_FFMpegVideo[id] = false;
}

//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
const int FFMPEG_Open_video_stream(const char *f_name)
{int stream_id = FFMPEG_Get_a_stream_id();
 if(stream_id != -1)
  {Tab_P_FFMpegVideo[stream_id] = new FFMpegVideo(f_name);
  }
 return stream_id;
}

//______________________________________________________________________________
const int FFMPEG_Close_video_stream(const int num_stream)
{int rep;
 if(num_stream >= 0 && num_stream < NB_MAX_P_FFMpegVideo)
  {rep = 2;
   if(Tab_P_FFMpegVideo[num_stream]) {rep = 1; delete Tab_P_FFMpegVideo[num_stream];}
   FFMPEG_Release_stream_id(num_stream);
  } else {rep = 0;}
 return rep;
}

//______________________________________________________________________________
const char* FFMPEG_get_error_message(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream))
  {if(Tab_P_FFMpegVideo[num_stream]) {return Tab_P_FFMpegVideo[num_stream]->get_error_message();}
  }
 return "";
}

//______________________________________________________________________________
const int FFMPEG_startAcquisition(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream))
  {return Tab_P_FFMpegVideo[num_stream]->startAcquisition();}
 return -1;
}

//______________________________________________________________________________
void FFMPEG_stopAcquisition(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream))
  {Tab_P_FFMpegVideo[num_stream]->stopAcquisition();}
}

//______________________________________________________________________________
const bool FFMPEG_getImage  (const int num_stream, void *img)
{if(FFMPEG_Stream_exists(num_stream))
  {return Tab_P_FFMpegVideo[num_stream]->getImage(img);}
 return false;
}

//______________________________________________________________________________
const bool FFMPEG_getImageNr(const int num_stream, unsigned long frame, void *img)
{if(FFMPEG_Stream_exists(num_stream))
  {return Tab_P_FFMpegVideo[num_stream]->getImageNr(frame, img);}
 return false;
}

//______________________________________________________________________________
//const bool FFMPEG_seek      (const int num_stream, unsigned long frame, bool iframe, SeekMode sm)
const bool FFMPEG_seek      (const int num_stream, unsigned long frame, bool iframe = false, int sm = ABS)
{if(FFMPEG_Stream_exists(num_stream))
  {return Tab_P_FFMpegVideo[num_stream]->seek(frame, iframe, (SeekMode)sm);}
 return false;
}

//______________________________________________________________________________
const unsigned int FFMPEG_get_nb_total_video_frames(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->Nb_total_video_frames();
  }
 return 0;
}

//______________________________________________________________________________
const int FFMPEG_numFrames(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->numFrames();
  }
 return -1;
}

//______________________________________________________________________________
const double FFMPEG_getFramerate(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->getFramerate();
  }
 return 0;
}

//______________________________________________________________________________
const double FFMPEG_setFramerate(const int num_stream, const double v)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->setFramerate(v);
  }
 return 0;
}

//______________________________________________________________________________
const char* FFMPEG_information(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->information().c_str();
  }
 return (char*)NULL;
}

//______________________________________________________________________________
const int FFMPEG_Width (const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->Width();
  }
 return -1;
}

//______________________________________________________________________________
const int FFMPEG_Height(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->Height();
  }
 return -1;
}

//______________________________________________________________________________
Info_for_sound_CB* FFMPEG_Info_for_sound_CB      (const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->get_Info_for_sound_CB();
  }
 return (Info_for_sound_CB*)NULL;
}

//______________________________________________________________________________
const unsigned int FFMPEG_Sound_sample_rate (const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->Sound_sample_rate();
  }
 return 0;
}

//______________________________________________________________________________
const unsigned int FFMPEG_Nb_channels(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->Nb_channels();
  }
 return 0;
}

//______________________________________________________________________________
const unsigned int FFMPEG_Audio_buffer_size(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->Audio_buffer_size();
  }
 return 0;
}

//______________________________________________________________________________
void FFMPEG_Info_for_sound_Drain_all (const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   Tab_P_FFMpegVideo[num_stream]->IFS_Drain_all();
  }
}


//______________________________________________________________________________
void FFMPEG_Lock  (const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   Tab_P_FFMpegVideo[num_stream]->Lock();
  }
}

//______________________________________________________________________________
void FFMPEG_UnLock(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   Tab_P_FFMpegVideo[num_stream]->UnLock();
  }
}

/*
//______________________________________________________________________________
const double FFMPEG_get_Synchronisation_threshold(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->get_Synchronisation_threshold();
  }
 return -1;
}
//______________________________________________________________________________
void FFMPEG_set_Synchronisation_threshold(const int num_stream, const double v)
{if(FFMPEG_Stream_exists(num_stream)) {
   Tab_P_FFMpegVideo[num_stream]->set_Synchronisation_threshold(v);
  }
}
*/
//______________________________________________________________________________
const double FFMPEG_get_Video_time_base(const int num_stream) {
 if(FFMPEG_Stream_exists(num_stream)) {
	 //printf("FFMPEG_get_Video_time_base -> %f\n", Tab_P_FFMpegVideo[num_stream]->Video_time_base());
	 return Tab_P_FFMpegVideo[num_stream]->Video_time_base();
	}// else {printf("FFMPEG_get_Video_time_base on a non ready video stream...\n");}
 return 0;
}

//______________________________________________________________________________
const unsigned int FFMPEG_get_Video_pts(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->Video_pts();
  }
 return 0;
}

//______________________________________________________________________________
const unsigned int FFMPEG_get_Video_pts_for_audio(const int num_stream)
{if(FFMPEG_Stream_exists(num_stream)) {
   return Tab_P_FFMpegVideo[num_stream]->Video_pts_for_audio();
  }
 return 0;
}

//______________________________________________________________________________
void FFMPEG_set_Video_pts_for_audio(const int num_stream, const unsigned int v)
{if(FFMPEG_Stream_exists(num_stream)) {
   Tab_P_FFMpegVideo[num_stream]->Video_pts_for_audio(v);
  }
}


//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
int FFMPEG_Convert_void_to_binary_tcl_var(void *ptr_void, int length, char *var_name, const bool append)
{ if(Ffmpeg_for_tcl_TCL_obj_array == NULL) {
    Ffmpeg_for_tcl_TCL_obj_array = Tcl_NewByteArrayObj((unsigned char *)ptr_void, length);
   } else {Tcl_SetByteArrayObj(Ffmpeg_for_tcl_TCL_obj_array, (unsigned char *)ptr_void, length);
          }
  //Tcl_IncrRefCount(obj_array);
  int flag;
  if(append) {flag = TCL_APPEND_VALUE;} else {flag = 0;}
  Tcl_SetVar2Ex(FFMPEG_Tcl_Interp, var_name, NULL, Ffmpeg_for_tcl_TCL_obj_array, /*TCL_GLOBAL_ONLY | */flag);
  
  return length;
}


//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
void FFMPEG_FSOUND_Init()
{FSOUND_Init(44100, 32, FSOUND_INIT_USEDEFAULTMIDISYNTH | FSOUND_INIT_DONTLATENCYADJUST);
}

//______________________________________________________________________________
const int FFMPEG_FSOUND_GetBufferLengthTotal() {
	return FSOUND_DSP_GetBufferLengthTotal();
}

//______________________________________________________________________________
const unsigned int FFMPEG_FSOUND_Mono    () {return FSOUND_MONO;}
const unsigned int FFMPEG_FSOUND_Stereo  () {return FSOUND_STEREO;}
const unsigned int FFMPEG_FSOUND_16b     () {return FSOUND_16BITS;}
const unsigned int FFMPEG_FSOUND_8b      () {return FSOUND_8BITS;}
const unsigned int FFMPEG_FSOUND_unsigned() {return FSOUND_UNSIGNED;}
const unsigned int FFMPEG_FSOUND_signed  () {return FSOUND_SIGNED;}

//______________________________________________________________________________
const int FFMPEG_get_volume_of_canal(const int canal)           {return FSOUND_GetVolume(canal);}
void FFMPEG_set_volume_of_canal(const int canal, const int vol) {FSOUND_SetVolume(canal, vol);}

//______________________________________________________________________________
FSOUND_STREAM* FFMPEG_Get_a_new_FSOUND_STREAM( FSOUND_STREAMCALLBACK callback
                               , int lenbytes
                               , unsigned int mode
                               , int samplerate
                               , void *userdata )
{FSOUND_STREAM *audio_strm = FSOUND_Stream_Create( callback
												 , lenbytes, FSOUND_STREAMABLE | FSOUND_NONBLOCKING | mode
												 , samplerate
												 , userdata );

/* if( FSOUND_Stream_GetOpenState(audio_strm) == 0)
   {FSOUND_Stream_Play(canal, audio_strm);
   }*/

 return audio_strm;
}

//______________________________________________________________________________
//______________________________________________________________________________
const int FFMPEG_FSOUND_STREAM_Play(Info_for_sound_CB *ifscb, FSOUND_STREAM *audio_strm, int canal)
{	//ifscb->first_t = clock();
	return FSOUND_Stream_Play(canal, audio_strm);
}

//______________________________________________________________________________
//______________________________________________________________________________
const int FFMPEG_FSOUND_STREAM_Stop(Info_for_sound_CB *ifscb, FSOUND_STREAM *audio_strm)
{return FSOUND_Stream_Stop(audio_strm);
}

//______________________________________________________________________________
const int FFMPEG_FSOUND_STREAM_GetOpenState(FSOUND_STREAM *audio_strm)
{return FSOUND_Stream_GetOpenState(audio_strm);
}
//______________________________________________________________________________
/*void  FFMPEG_Synchronize_audio_with_video(const int num_stream) {
	if(FFMPEG_Stream_exists(num_stream)) {
		 Tab_P_FFMpegVideo[num_stream]->Synchronize_audio_with_video();
		}
}*/


//______________________________________________________________________________
const double FFMPEG_get_audio_clock_start(const int num_stream) {
	if(FFMPEG_Stream_exists(num_stream)) {
		 return Tab_P_FFMpegVideo[num_stream]->get_audio_clock_start();
		} else return 0;
}


//______________________________________________________________________________
const double FFMPEG_get_first_time(const int num_stream) {
	if(FFMPEG_Stream_exists(num_stream)) {
		 return (double)Tab_P_FFMpegVideo[num_stream]->get_first_time();
		} else return 0;
}

//______________________________________________________________________________
const bool FFMPEG_close_FSOUND_STREAM(FSOUND_STREAM *strm)
{FSOUND_Stream_Stop(strm);
{printf("FFMPEG_close_FSOUND_STREAM : waiting for fmod stream to stop\n");}
 printf("Stream closed!\n");
 return FSOUND_Stream_Close(strm);
}

//______________________________________________________________________________
const unsigned int FFMPEG_getSoundPosition(FSOUND_STREAM *stream) {
	return FSOUND_Stream_GetPosition(stream);
}

//______________________________________________________________________________
void FFMPEG_getSoundPosition(FSOUND_STREAM *stream, const unsigned int pos) {
	FSOUND_Stream_SetPosition(stream, pos);
}

//______________________________________________________________________________
const int FFMPEG_getSoundTime(FSOUND_STREAM *stream) {
	return FSOUND_Stream_GetTime(stream);
}

//______________________________________________________________________________
void FFMPEG_setSoundTime(FSOUND_STREAM *stream, const int pos) {
	FSOUND_Stream_SetTime(stream, pos);
}

//______________________________________________________________________________
const long FFMPEG_get_num_last_buffer(const int num_stream) {
	if(FFMPEG_Stream_exists(num_stream)) {
		 return (double)Tab_P_FFMpegVideo[num_stream]->get_num_last_buffer();
		} else return 0;
}

//______________________________________________________________________________
const double FFMPEG_get_delta_from_first_time(const int num_stream) {
	if(FFMPEG_Stream_exists(num_stream)) {
		 return (double)Tab_P_FFMpegVideo[num_stream]->get_delta_from_first_time();
		} else return 0;
}

//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
#include <tk.h>
static int Void2Photo (void *imgVector, 
                       const char *photoName, int width, int height,
                       int numChans)
{	Tcl_Interp *interp = FFMPEG_Tcl_Interp;
    int y, usedChans;
    Tk_PhotoHandle photo;
    Tk_PhotoImageBlock photoCont;
    unsigned char *imgVec = (unsigned char *) imgVector;
    unsigned char *srcPtr, *dstPtr;
    unsigned char *srcStop;
    unsigned char *pixBuf;

    if (!(photo = Tk_FindPhoto (interp, (char *)photoName))) {
        Tcl_AppendResult (interp, "cannot find photo image: ",
                          photoName, (char *)NULL);
        return TCL_ERROR;
    }

    #if (TK_MAJOR_VERSION > 8) || \
       ((TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION >= 5))
        Tk_PhotoSetSize (interp, photo, width, height);
    #else
        Tk_PhotoSetSize (photo, width, height);
    #endif

    usedChans = (numChans <= 3? 3: 4);

    if (!(pixBuf = (unsigned char *)malloc (width * height * usedChans * sizeof (unsigned char)))) {
        Tcl_AppendResult (interp, "cannot alloc scanline bufer for photo: ",
                          photoName, (char *)NULL);
        return TCL_ERROR;
    }

    photoCont.pixelSize = usedChans;
    photoCont.pitch = width * usedChans;
    photoCont.width = width;
    photoCont.height = height;
    photoCont.offset[0] = 0;
    photoCont.offset[1] = 1;
    photoCont.offset[2] = 2;
    photoCont.offset[3] = (usedChans == 3? 0: 3);

    photoCont.pixelPtr = pixBuf;
    srcPtr = imgVec;
    dstPtr = pixBuf;

    if (numChans == 3 || numChans == 4) {
        memcpy (dstPtr, srcPtr, width*height*numChans);
    } else {
        for (y = 0; y < height; y++) {
            srcStop = srcPtr + numChans * width;
            if (numChans == 1) {
                while (srcPtr < srcStop) {
                    *(dstPtr++) = *srcPtr;
                    *(dstPtr++) = *srcPtr;
                    *(dstPtr++) = *(srcPtr++);
                }
            } else if (numChans == 2) {
                while (srcPtr < srcStop) {
                    *(dstPtr++) = *srcPtr;
                    *(dstPtr++) = *srcPtr;
                    *(dstPtr++) = *(srcPtr++);
                    *(dstPtr++) = *(srcPtr++);
                }
            }
        }
    }
    #if (TK_MAJOR_VERSION > 8) || \
        ((TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION >= 5))
        Tk_PhotoPutBlock (interp, photo, &photoCont, 0, 0,
                          width, height, TK_PHOTO_COMPOSITE_OVERLAY);
    #elif ((TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION > 3))
        Tk_PhotoPutBlock (photo, &photoCont, 0, 0,
                          width, height, TK_PHOTO_COMPOSITE_OVERLAY);
    #else
        Tk_PhotoPutBlock (photo, &photoCont, 0, 0,
                          width, height);
    #endif

    free (pixBuf);
    return TCL_OK;
}
