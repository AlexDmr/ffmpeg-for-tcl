#include "Infos_audio.h"
#include "FFMpegInit.h"
#include <iostream>
#include <slimthread.h>
//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
void Info_buffer_audio_Init (Info_buffer_audio *iba) {
 //iba->buf  = NULL;
 iba->size = 0;
 iba->deb  = 0;
}

//______________________________________________________________________________
void Info_buffer_audio_Maj(Info_buffer_audio *iba, unsigned int s, unsigned int d) {
 iba->size = s;
// iba->buf  = b;
 iba->deb  = d;
}

//______________________________________________________________________________
void Info_buffer_audio_Copy(Info_buffer_audio *iba, const Info_buffer_audio *src) {
 iba->size = src->size;
// iba->buf  = src->buf;
 iba->deb  = src->deb;
}

//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
// For sound callback
void Info_for_sound_CB_Init(Info_for_sound_CB *ifscb, Mutex *m)  {
 ifscb->mutex = m; 
 ifscb->first = ifscb->last = ifscb->nb = ifscb->size_buffers = 0; 
 ifscb->nb_buffers = ALX_INFO_BUFFER_AUDIO_NB_BUFFER;
 for(unsigned int i = 0; i<ifscb->nb_buffers; i++)
  {Info_buffer_audio_Init( &(ifscb->Tab_wav[i]) );}
 ifscb->s_SynchronisationThreshold = 0.15;
 ifscb->last_buffer_index = 0;
}

//______________________________________________________________________________
Mutex* Info_for_sound_CB_Get_mutex (Info_for_sound_CB *ifscb) {return ifscb->mutex;}

//______________________________________________________________________________
const unsigned int Info_for_sound_CB_Nb_buffers(Info_for_sound_CB *ifscb) {return ifscb->nb_buffers;}

//______________________________________________________________________________
const unsigned int Info_for_sound_CB_Nb_pkt    (Info_for_sound_CB *ifscb) {return ifscb->nb;}

//______________________________________________________________________________
const unsigned int Info_for_sound_CB_Size_buffers(Info_for_sound_CB *ifscb) {return ifscb->size_buffers;}

//______________________________________________________________________________
void Info_for_sound_CB_Put_New(Info_for_sound_CB *ifscb, const void *buf_src, const int size, const int64_t pts, const int64_t dts, const int duration)
{if(ifscb->nb >= ALX_INFO_BUFFER_AUDIO_NB_BUFFER) {std::cout << "\n________\nOVERLOAD!\n";}
 ifscb->size_buffers += size;
   ifscb->Tab_wav[ifscb->last].size     = size;
   ifscb->Tab_wav[ifscb->last].pts      = pts;
   ifscb->Tab_wav[ifscb->last].dts      = dts;
   ifscb->Tab_wav[ifscb->last].duration = duration;

   if(ifscb->last_buffer_index + size >= ALX_INFO_BUFFER_AUDIO_TAILLE_BUFFER)
    {ifscb->Tab_wav[ifscb->last].deb    = 0;
     ifscb->Tab_wav[ifscb->last].buffer = ifscb->internal_buffer;
     ifscb->last_buffer_index           = size;
	 //std::cout << "AUDIO CYCLE\n";
    } else {ifscb->Tab_wav[ifscb->last].deb    = 0;
            ifscb->Tab_wav[ifscb->last].buffer = ifscb->internal_buffer + ifscb->last_buffer_index;
			ifscb->last_buffer_index          += size;
           }
   memcpy(ifscb->Tab_wav[ifscb->last].buffer, buf_src, size);
 ifscb->last = (ifscb->last+1)%ifscb->nb_buffers;
 ifscb->nb++;

}

//______________________________________________________________________________
const double Info_for_sound_get_Synchronisation_threshold(const Info_for_sound_CB *ifscb)   {return ifscb->s_SynchronisationThreshold;}
//______________________________________________________________________________
void Info_for_sound_set_Synchronisation_threshold(Info_for_sound_CB *ifscb, double v) {ifscb->s_SynchronisationThreshold = v;}

//______________________________________________________________________________
void Info_for_sound_CB_Release(Info_for_sound_CB *ifscb) {
 //if(ifscb->Tab_wav[ifscb->first].buf) {free(ifscb->Tab_wav[ifscb->first].buf);}
 ifscb->size_buffers -= ifscb->Tab_wav[ifscb->first].size;
 ifscb->Tab_wav[ifscb->first].deb = 0; ifscb->Tab_wav[ifscb->first].size = 0;
 ifscb->first = (ifscb->first+1)%ifscb->nb_buffers;
 ifscb->nb--;
}

//______________________________________________________________________________
void Info_for_sound_Drain_all(Info_for_sound_CB *ifscb)
{Info_for_sound_CB_Lock  (ifscb);
 while(Info_for_sound_CB_Nb_pkt(ifscb))
  {Info_for_sound_CB_Release(ifscb);
  }
 Info_for_sound_CB_UnLock(ifscb);
}

//______________________________________________________________________________
void Info_for_sound_CB_Maj_size(Info_for_sound_CB *ifscb, const unsigned int index, const unsigned int d, const unsigned int s) {
 ifscb->size_buffers -= ifscb->Tab_wav[index].size - s;
 ifscb->Tab_wav[index].deb  = d;
 ifscb->Tab_wav[index].size = s;
}

//______________________________________________________________________________
void Info_for_sound_CB_Lock  (Info_for_sound_CB *ifscb)
{ifscb->mutex->lock();}

//______________________________________________________________________________
void Info_for_sound_CB_UnLock(Info_for_sound_CB *ifscb)
{ifscb->mutex->unlock();}

//______________________________________________________________________________
const int Info_for_sound_CB_First_index(Info_for_sound_CB *ifscb)
{return ifscb->first;
}

//______________________________________________________________________________
const int Info_for_sound_CB_Read(Info_for_sound_CB *ifscb, void *buff, const int dec_buf, const int len)
{if(ifscb->nb == 0) {return -1;}
 int len_wav     = ifscb->Tab_wav[ifscb->first].size;
 int nb_to_copy  = len_wav<len?len_wav:len;
 void *buf_src   = ifscb->Tab_wav[ifscb->first].buffer;
 int deb_buf_src = ifscb->Tab_wav[ifscb->first].deb;

 //printf("\n  pkt : %i\n  len : %i\n  dec_buff : %i\n  deb_buf_src : %i\n  memcpy(%p, %p, %i)\n"
 //	   , len_wav, len, dec_buf, deb_buf_src, (char*)buff+dec_buf, (char*)(buf_src) + deb_buf_src, nb_to_copy);
 memcpy((char*)buff+dec_buf, (char*)(buf_src) + deb_buf_src, nb_to_copy);
 
 if(nb_to_copy == len_wav)
  {//printf("  Info_for_sound_CB_Release\n"); 
   Info_for_sound_CB_Release(ifscb);
  } else {//printf("  Info_for_sound_CB_Maj_size (%i, %i)", ifscb->Tab_wav[ifscb->first].deb, ifscb->Tab_wav[ifscb->first].size);
          Info_for_sound_CB_Maj_size( ifscb
                                    , ifscb->first
                                    , ifscb->Tab_wav[ifscb->first].deb  + nb_to_copy
                                    , ifscb->Tab_wav[ifscb->first].size - nb_to_copy );
		 //XXX ALREADY DONE IN Info_for_sound_CB_Maj_size XXX ifscb->size_buffers -= nb_to_copy;
		  //printf(" => (%i, %i)\n", ifscb->Tab_wav[ifscb->first].deb, ifscb->Tab_wav[ifscb->first].size);
         }
 return nb_to_copy;
}

//______________________________________________________________________________
// Difference between audio time and video time should never be more than a video frame time.
// Return true if sound must be delayed, false elsewhere.
const bool Info_for_sound_CB_Synch_audio_to_video(Info_for_sound_CB *ifscb, const int len)
{if(ifscb->nb == 0) {
	//std::cout << "NO AUDIO PACKET...\n"; 
	return true;
   }
double pts; 
if(ifscb->video_pts != AV_NOPTS_VALUE) {
   pts = ifscb->video_pts;
  } else {pts = 0;
         }
 //pts /= ifscb->time_base_video / ifscb->video_sample_rate;// av_q2d( (AVRational)ifscb->time_base_video );
 pts *= ifscb->time_base_video;
 double t_video      = pts //- ifscb->s_SynchronisationThreshold//((double)ifscb->video_pts / (double)ifscb->time_base_video / ifscb->video_sample_rate) - ifscb->s_SynchronisationThreshold
	  , t_video_next = pts + ifscb->s_SynchronisationThreshold; //((double)ifscb->video_pts / (double)ifscb->time_base_video / ifscb->video_sample_rate) + ifscb->s_SynchronisationThreshold;
 bool go_on;

 do {
   double /*dt            = ifscb->Tab_wav[ifscb->first].duration    * (double)ifscb->time_base_audio
        ,*/t_start_audio = (double)ifscb->Tab_wav[ifscb->first].pts * (double)ifscb->time_base_audio
		, t_next_audio  = (double)ifscb->Tab_wav[(ifscb->first + 1) % ifscb->nb_buffers].pts * (double)ifscb->time_base_audio
		;
   if ( 0/*!ifscb->synchronize_with_video
	  &&( t_start_audio > t_video_next )*/
	  )
    {
	 std::cout << "Delay audio frame cause A:" << t_start_audio << " > NextV:" << t_video_next << "\n";
     return true;
    }
   if( /*(t_start_audio+dt)*//*t_next_audio*///(t_next_audio + ifscb->s_SynchronisationThreshold) < t_video 
	      ( ifscb->synchronize_with_video && t_next_audio < t_video)
	  // || (!ifscb->synchronize_with_video && (t_next_audio+ifscb->s_SynchronisationThreshold) < t_video)
	 ) {
	 //std::cout << "Audio is late / video " << ifscb->not_enough << " , " << ((Info_for_sound_CB_Size_buffers(ifscb)-ifscb->Tab_wav[ifscb->first].size) >= len ) << "\n";
	 if( ((Info_for_sound_CB_Size_buffers(ifscb)-ifscb->Tab_wav[ifscb->first].size) >= len ) 
	   &&(!ifscb->not_enough) )
	  {ifscb->has_skiped = true;
	   go_on = true;
      // Skip audio frame
	   std::cout << "Skip audio frame cause V(" << t_video << "); A(" << t_start_audio << "); "" A.next:" << /*t_start_audio+dt*/t_next_audio << " < V:" << t_video << "\n";
	   Info_for_sound_CB_Release(ifscb);
	  } else {go_on = false;
	          if((Info_for_sound_CB_Size_buffers(ifscb)-ifscb->Tab_wav[ifscb->first].size) >= len )
			   {ifscb->not_enough = false;
			    std::cout << "Skip skipped...\n";
			   }
	         }
    } else {go_on = false;}
  } while( ifscb->nb && go_on );
 return (ifscb->nb == 0);
}
