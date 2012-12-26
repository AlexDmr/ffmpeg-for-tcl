#define __STDC_CONSTANT_MACROS
#define inline _inline
#include "stdafx.h"
#include "FFMpegInit.h"

#include <sstream>
//#include <File.h>

#include <Infos_audio.cpp>

#include "FFMpegVideo.h"

#ifdef WIN32
/*  #pragma comment (lib, "avcodec-52.lib" )
  #pragma comment (lib, "avformat-52.lib" )
  #pragma comment (lib, "avutil-50.lib" )
  #pragma comment (lib, "avdevice-52.lib" )
  #pragma comment (lib, "swscale-0.lib" )
*/
  #pragma comment (lib, "avcodec.lib" )
//  #pragma comment (lib, "avcore.lib" )
  #pragma comment (lib, "avdevice.lib" )
  #pragma comment (lib, "avfilter.lib" )
  #pragma comment (lib, "avformat.lib" )
  #pragma comment (lib, "avutil.lib" )
  #pragma comment (lib, "swscale.lib" )
  #pragma comment (lib, "swresample" )

  #pragma comment (lib, "fmodvc.lib" )
  #pragma comment (lib, "tcl85.lib" )
  #pragma comment (lib, "tk85.lib" )
  //#pragma comment (lib, "C:/FMOD/api/lib/fmodvc.lib" )

  //#pragma comment (lib, "Ws2_32.lib") 
#endif

 //______________________________________________________________________________
void FFMpegVideo::init() {
		formatCtx = 0;
		codecCtx = 0;
		img_convert_ctx = NULL;
		Info_for_sound_CB_Init(&info_for_sound_CB, &mutex_audio);
		debug_mode = false; audio_buf = (uint8_t*)NULL; Audio_buffer_size((AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2);
          sound_sample_rate = 0; nb_channels = 0;
		frame = audio_frame = NULL;
		initial_video_pts = 0xFFFFFFFF;
		this->audio_swr = NULL;
		this->dst_data_for_convert[0] = NULL;
		this->size_buffer_for_convert = 0;
		this->size_temp_audio_convertion_buffer = 65536;
		this->temp_audio_convertion_buffer      = (int*)malloc( this->size_temp_audio_convertion_buffer );

		ffmpeg_init();

		id = "avi";
}

 //______________________________________________________________________________
   FFMpegVideo::FFMpegVideo() : started(false), nrFrames(0), curFrame(0), nb_total_video_frames(0) {
	   init();
	}

//______________________________________________________________________________
	FFMpegVideo::FFMpegVideo(const char *fname) : started(false), nrFrames(0), curFrame(0), nb_total_video_frames(0) {
		init();
		open( fname );	
	}

//______________________________________________________________________________
	FFMpegVideo::FFMpegVideo(const std::string & fname) : started(false), nrFrames(0), curFrame(0), nb_total_video_frames(0) {
		init();
		open( fname.c_str() );
	}

//______________________________________________________________________________
	FFMpegVideo::~FFMpegVideo() {
		close();
	}

//______________________________________________________________________________
//const double FFMpegVideo::get_Synchronisation_threshold() const {return Info_for_sound_get_Synchronisation_threshold(&info_for_sound_CB);}

//______________________________________________________________________________
//void FFMpegVideo::set_Synchronisation_threshold(const double v)
//{Info_for_sound_set_Synchronisation_threshold(&info_for_sound_CB, v);}

//______________________________________________________________________________
const bool FFMpegVideo::open(const char *fname) {
		int code_ret;
		char error_tmp[256];

		close();

        //XXX util::File f(fname);
        //XXX id = f.getName();
		      id = fname;

//		if( (code_ret = av_open_input_file(&formatCtx, fname, NULL, 0, NULL)) != 0 )
	   if( (code_ret = avformat_open_input(&formatCtx, fname, NULL, NULL)) != 0 )
		 {printf("Failed to open input file");
		  error_msg = "Failed to open input ressource (code "; error_msg += _itoa_s(code_ret, error_tmp, 10); error_msg += ").";
	      return false;
		 }
		
		// Retrieve stream information
		if( (code_ret = avformat_find_stream_info(formatCtx, NULL)) < 0 ) {
			av_close_input_file(formatCtx);
			formatCtx = 0;
			
			printf("Failed to find stream information");
			error_msg = "Failed to find stream information (code"; error_msg += code_ret; error_msg += ").";
			return false;
		}

		// Find the first video stream
		// XXX Trouver un moyen d'avoir plusieurs flux en simultané
		// XXX par exemple avec des tableau statiques
		initial_video_pts = 0xFFFFFFFF;
		videoStream = -1; audioStream = -1; sample_fmt = AV_SAMPLE_FMT_NONE;
		for(int i = 0; i < formatCtx->nb_streams; ++i) {
			AVCodecContext *enc = formatCtx->streams[i]->codec;
			if( enc->codec_type == AVMEDIA_TYPE_VIDEO/*CODEC_TYPE_VIDEO*/ && videoStream == -1) {
			  videoStream = i;
			  std::cout << "videoStream : " << videoStream << "\n";
			 }
			if( enc->codec_type == AVMEDIA_TYPE_AUDIO/*CODEC_TYPE_AUDIO*/ && audioStream == -1) {
			  audioStream = i;
			  sample_fmt  = enc->sample_fmt;
			  std::cout << "audioStream : " << audioStream << "\n";
			 }
		}
		
		info_for_sound_CB.has_skiped = info_for_sound_CB.not_enough = false;
		audio_codec = NULL;
		if( audioStream != -1) // There is some SOUND!!!
		 {AVStream *audio_str = formatCtx->streams[audioStream];
		  audio_codecCtx = audio_str->codec;
		  audio_codec = avcodec_find_decoder(audio_codecCtx->codec_id);
          avcodec_open2(audio_codecCtx, audio_codec, NULL);
		  sound_sample_rate = info_for_sound_CB.audio_sample_rate = audio_codecCtx->sample_rate;
		  nb_channels = info_for_sound_CB.nb_channels = audio_codecCtx->channels;
		    
          info_for_sound_CB.is_audio_signed   = true;
          info_for_sound_CB.size_audio_sample = 16;
		  info_for_sound_CB.time_base_audio   = av_q2d( audio_str->time_base );
		  printf("info_for_sound_CB.time_base_audio = %f\n", info_for_sound_CB.time_base_audio);
		  printf("num / denum : %d / %d\n\n", audio_str->time_base.num, audio_str->time_base.den);

		  // Is there a need for audio convertion?
		  if(this->audio_swr) {swr_free( &(this->audio_swr) );}
		  if(sample_fmt != AV_SAMPLE_FMT_S16) {
			  this->audio_swr = swr_alloc();
			  this->audio_swr = swr_alloc_set_opts( this->audio_swr
								, audio_codecCtx->channel_layout, AV_SAMPLE_FMT_S16, sound_sample_rate
                                , audio_codecCtx->channel_layout, sample_fmt, sound_sample_rate
                                , 0, NULL);
			  int ret = swr_init( this->audio_swr );
			  if(ret < 0) printf("\n_-_-_-_-_-_-_-_-> ERROT initialiazing swr_convert -> %d\n\n", ret);
				else printf("\nswr_nit OK -> %d\n\n", ret);
			}
		  //info_for_sound_CB.time_base_audio   = audio_str->time_base.num;
		  //XXXpacket_queue_init(&audioq);str->r_frame_rate.num / str->time_base.num
		 }

		if( videoStream == -1 ) {
			av_close_input_file(formatCtx);
			formatCtx = 0;
		
			printf("Failed to find a video stream");
			error_msg = "Failed to find a video stream";
			return false;
		}

		AVStream *str = formatCtx->streams[ videoStream ];
		info_for_sound_CB.time_base_video = this->time_base_video = av_q2d( str->time_base );
		printf("info_for_sound_CB.time_base_video = %f\n", this->time_base_video);
		printf("num / denum : %d / %d\n", str->time_base.num, str->time_base.den);
		// printf("formatCtx->timestamp = %d\n", formatCtx->timestamp);
		printf("formatCtx->duration = %d\n", formatCtx->duration);
		
		if (str->time_base.den > 1000) {
		  //info_for_sound_CB.time_base_video = str->time_base.den / 1000.0;
          //printf("Changing time_base_video to %f", info_for_sound_CB.time_base_video);
		 } 


		// Get a pointer to the codec context for the video stream
		codecCtx = str->codec;

		// Find the decoder for the video stream __________________________________________________
		codec = avcodec_find_decoder(codecCtx->codec_id);
		if( codec == NULL ) {
			av_close_input_file(formatCtx);
			formatCtx = 0;
		
			printf("Codec not found");
			error_msg = "Codec not found";
			return false;
		}
		
		// Open codec _____________________________________________________________________________
		if( avcodec_open2(codecCtx, codec, NULL) < 0 ) {
			av_close_input_file(formatCtx);
			formatCtx = 0;
			
			printf("Could not open codec");
			error_msg = "Codec found but unable to open it...";
			return false;
		}

		mFramerate = info_for_sound_CB.video_sample_rate = av_q2d( str->r_frame_rate ); //1.0f * av_q2d( str->r_frame_rate ) / av_q2d( str->time_base );
		std::cout << "mFramerate = " << mFramerate << " \n";
		if (str->time_base.num == 0 || str->r_frame_rate.num == 0) {
			 av_close_input_file(formatCtx);
			 formatCtx = 0;
			 printf("str->time_base.num == 0 || str->r_frame_rate.num == 0");
			 error_msg = "str->time_base.num == 0 || str->r_frame_rate.num == 0";
			 return false;
			}
		int64_t frame_size = (int64_t(str->time_base.den) * str->r_frame_rate.den
							 / str->time_base.num) / str->r_frame_rate.num;

		//nrFrames = str->duration / frame_size;

		//nb_total_video_frames  = (int) (formatCtx->duration / 1000;
		//nb_total_video_frames *= info_for_sound_CB.time_base_video * mFramerate;
		nrFrames = nb_total_video_frames = (int) (mFramerate * formatCtx->duration / AV_TIME_BASE);

		printf("Metadata formatCtx :\n");
		AVDictionaryEntry *dict_E = av_dict_get(formatCtx->metadata, "", NULL, AV_DICT_IGNORE_SUFFIX);
		while (dict_E) {
			 printf("\t%s : %s\n", dict_E->key, dict_E->value);
			 dict_E = av_dict_get(formatCtx->metadata, "", dict_E, AV_DICT_IGNORE_SUFFIX);
			}

		AVDictionaryEntry *dict_W = av_dict_get(formatCtx->metadata, "width", NULL, 0);
		if(dict_W) {width = atoi(dict_W->value);
				    printf("width found in formatCtx->metadata, its value is %d\n", width);
				   } else {width  = str->codec->width;
						   printf("width found in str->codec->width, its value is %d\n", width);
						  }
		AVDictionaryEntry *dict_H = av_dict_get(formatCtx->metadata, "height", NULL, 0);
		if(dict_H) {height = atoi(dict_H->value);
				    printf("height found in formatCtx->metadata, its value is %d\n", height);
				   } else {height  = codecCtx->height;
						   printf("height found in codecCtx, its value is %d\n", height);
						  }


		// Get an image context convertissor _____________________________________________________
		img_convert_ctx = sws_getContext(codecCtx->width, codecCtx->height, 
                                         str->codec->pix_fmt, 
                                         codecCtx->width, codecCtx->height, 
										 PIX_FMT_RGB24, SWS_BICUBIC, 
                                         NULL, NULL, NULL);
        if(img_convert_ctx == NULL) {
	      fprintf(stderr, "Cannot initialize the conversion context!\n");
		  error_msg = "Cannot initialize the conversion context!";
		  return false;
		 }
	
		error_msg = "";
		return true;
	}

//______________________________________________________________________________
	void FFMpegVideo::close()	{
		if( frame != 0 ) {
			
			// Free the YUV frame
			av_free(frame);
			frame = 0;
		}

		if( formatCtx != 0 ) {
			// Close the codec
			avcodec_close(codecCtx);
			codecCtx = 0;

			// Close the file
			av_close_input_file(formatCtx);
			formatCtx = 0;
		}
	    if(this->audio_swr) {
			swr_free( &(this->audio_swr) );
	    }
	}

//______________________________________________________________________________
	std::string FFMpegVideo::information() const{
		std::ostringstream ss;
		ss << "FFMpegStream: " << id;
		return ss.str();
	}

//______________________________________________________________________________
	int FFMpegVideo::startAcquisition() {
		if( codecCtx == 0 )
			printf("No video opened!");

		frame       = avcodec_alloc_frame();
		audio_frame = avcodec_alloc_frame();
		if( frame == 0 )
			printf("Failed to allocate frame");

		// Determine required buffer size and allocate buffer
		int numBytes=avpicture_get_size(PIX_FMT_RGB24, codecCtx->width, codecCtx->height);
		started = true;

		this->initial_video_pts = 0xFFFFFFFF;
		getImageNr(0, NULL);

		return numBytes;
	}

//______________________________________________________________________________
	void FFMpegVideo::stopAcquisition() {

		started = false;


		// Free the YUV frame
		if(frame)		av_free(frame);
		if(audio_frame) av_free(audio_frame);
		frame = audio_frame = NULL;
	}

//______________________________________________________________________________
	bool FFMpegVideo::getImage(void *img/*util::ByteImage & img*/) {
        if( ! started )
            printf("Stream was not started!");

		const int width  = codecCtx->width;
		const int height = codecCtx->height;
		//XXX img.create(width, height, 3);

		if(debug_mode) cout << "getNextFrame...";
		if( ! getNextFrame() )
			return false;

		AVPicture frameRGB;
		// Assign appropriate parts of buffer to image planes in frameRGB
		if(debug_mode) cout << "FillConvert...";
		if(img) {
			avpicture_fill(&frameRGB, (uint8_t*)img, PIX_FMT_RGB24, 
						   codecCtx->width, codecCtx->height);
			sws_scale( img_convert_ctx, frame->data, frame->linesize, 0,
					   codecCtx->height, 
					   frameRGB.data, frameRGB.linesize);
			}

		if(debug_mode) cout << "END\n";
		return true;
	}

//______________________________________________________________________________
	bool FFMpegVideo::getImageNr(unsigned long nr, void *img/*util::ByteImage & img*/) {
		if( ! started ) {
			printf("Stream was not started!");
			return false;
		}

		const int width = codecCtx->width;
		const int height = codecCtx->height;

		if( ! seek(nr) )
			return false;

		/*if( ! getNextFrame() )
			return false; // DEJA FAIT DANS seek*/

		AVPicture frameRGB;
		// Assign appropriate parts of buffer to image planes in pFrameRGB
		if(img) {
			avpicture_fill(&frameRGB, (uint8_t*)img, PIX_FMT_RGB24, 
					codecCtx->width, codecCtx->height);
			sws_scale( img_convert_ctx, frame->data, frame->linesize, 0,
					   codecCtx->height, 
					   frameRGB.data, frameRGB.linesize);
			}

		return true;
	}

//______________________________________________________________________________
bool FFMpegVideo::seek(unsigned long nr, bool iframe /*= false*/, SeekMode sm /*= ABS*/) {
		if( (formatCtx == 0) || (codecCtx == 0) ) {
			printf("stream not open");
			return false;
		}

	unsigned long frameNr;
	avcodec_flush_buffers(codecCtx);
	if(audio_codecCtx) avcodec_flush_buffers(audio_codecCtx);

		switch( sm ) {
			case ABS:
				frameNr = nr;
				break;
			case FORWARD:
				frameNr += nr;
				break;
			case BACKWARD:
				frameNr -= nr;
				break;
			case END:
				frameNr = nrFrames - nr;
				break;
		}

        if( frameNr >= nrFrames)
			return false;

		if( true/*! iframe*/ )
			{curFrame = frameNr;}

		AVStream *str = formatCtx->streams[ videoStream ];
		int64_t frame_size = int64_t(str->time_base.den) * str->r_frame_rate.den
							 / str->time_base.num / str->r_frame_rate.num;
		int64_t timestamp = int64_t(str->time_base.den) * curFrame * str->r_frame_rate.den
							 / str->time_base.num / str->r_frame_rate.num + str->start_time;
		//printf("timestamp %d ", timestamp);
		if( timestamp > frame_size/2 )
			timestamp -= frame_size/2;
		else
			timestamp = 0;
		printf("timestamp=%d ; nr=%d ; r_frame_rate.num=%d / %d r_frame_rate.den\n", timestamp, nr, str->r_frame_rate.num, str->r_frame_rate.den);
		

		if( iframe )
		{	//av_seek_frame_generic(
			if( av_seek_frame(formatCtx, videoStream, timestamp/**int64_t(str->time_base.den)/str->time_base.num*/, AVSEEK_FLAG_BACKWARD) < 0 )
				return false;
			//timestamp = nr * str->r_frame_rate.num / str->r_frame_rate.den;
			do {
				if( ! getNextFrame() )
					return false;
				//cout << frameNr << " " << curFrame << endl;
			} while( /*frameNr > curFrame*/ 
					this->video_pts * this->time_base_video < (double)nr * str->r_frame_rate.den / str->r_frame_rate.num);
			curFrame = frameNr;
			printf("video %f/%f timestamp\n", this->time_base_video*this->video_pts, (double)nr * str->r_frame_rate.den / str->r_frame_rate.num);
		}
		else
		{	//avio_seek(AVIOContext *s, int64_t offset, int whence);
			if( av_seek_frame(formatCtx, videoStream, timestamp, 
							  AVSEEK_FLAG_ANY|AVSEEK_FLAG_BACKWARD) < 0 )
				return false;
		}
		
        return true;
	}

//______________________________________________________________________________
	bool FFMpegVideo::getNextFrame() {
		uint64_t dts, pts;

		int frameFinished = 0;
		do {
			AVPacket *pkt = new AVPacket();
			av_init_packet(pkt);
		
			// Find a frame in the current video stream
			if(debug_mode) cout << "Video frame...";
			bool found_video_pkt = false
               , found_audio_pkt = (audioStream != -1);
			do {
				if( av_read_frame(formatCtx, pkt) < 0) return false;
				if(pkt->stream_index == videoStream) {found_video_pkt = true; break;}
				if(pkt->stream_index == audioStream) {found_audio_pkt = true; break;}
			 } while( true );
			
			//if(debug_mode) cout <<  "Found...";

			if(found_video_pkt) {
			  // Decode time of last frame
			  //dts = pkt->dts; 
			  // presentation time of the last frame
			  if(pkt->pts != AV_NOPTS_VALUE) {pts = pkt->dts;} else {pts = pkt->pts;}
			  //printf("considering pts/dts : %d/%d", pts, pkt->dts);
              // DEBUG for best synchro : this->video_pts = info_for_sound_CB.video_pts = pts;
			  this->video_pts = pts;
			  
			  //pts *= av_q2d(codecCtx->time_base);
			  
			  // Decode video frame
			  avcodec_decode_video2(codecCtx, frame, &frameFinished, pkt);//pkt.data, pkt.size);
			  //frameFinished = true; // DEBUG

			  // Free last packet
			  av_free_packet(pkt);
			 } else {
				     put_audio_packet(pkt);
			        }
		} while( frameFinished == 0 );
		
		if(initial_video_pts == 0xFFFFFFFF) {this->initial_video_pts = this->video_pts;}
		this->video_pts -= this->initial_video_pts;
		//printf("video pts : %f\n", pts*time_base_video);
		// Process audio
		Process_audio_packets();
		// Deinterlace if requested
		if( false /* XXX what should be here? */ ) {
		  avpicture_deinterlace( (AVPicture*)frame, (AVPicture*)frame,
								 codecCtx->pix_fmt, codecCtx->width, codecCtx->height );
		}

		AVStream *str = formatCtx->streams[ videoStream ];
		dts += int64_t(str->time_base.den) * str->r_frame_rate.den
				/ str->time_base.num / str->r_frame_rate.num;

	/*unsigned long frameNr;
		frameNr = ((dts - str->start_time)
			* str->r_frame_rate.num / str->r_frame_rate.den
			* str->time_base.num + str->time_base.den/2)
			/ str->time_base.den;
    */
		curFrame++;
		return true;
	}

//______________________________________________________________________________
int FFMpegVideo::audio_decode_frame( AVCodecContext *aCodecCtx
								   //, uint8_t *audio_buf
								   //, int buf_size
								   , int *got_frame
								   , uint8_t **data, int *data_size
								   , AVPacket *pkt) {
 int ret;
 uint8_t *audio_pkt_data = pkt->data;

 // int len1 = avcodec_decode_audio3( aCodecCtx, (int16_t *)audio_buf, &data_size, pkt);
 ret        = avcodec_decode_audio4(aCodecCtx, audio_frame, got_frame, pkt);
 *data_size = av_samples_get_buffer_size( NULL
									, av_get_channel_layout_nb_channels(audio_codecCtx->channel_layout)
									, audio_frame->nb_samples
									, AV_SAMPLE_FMT_S16 // audio_codecCtx->sample_fmt
									, 1);// printf("ret=%d; ", data_size);
 if(this->audio_swr && *got_frame) {
	 if(this->size_buffer_for_convert < *data_size) {
		 if(this->dst_data_for_convert[0]) {av_free(this->dst_data_for_convert[0]); this->dst_data_for_convert[0] = NULL;}
		 ret = av_samples_alloc( this->dst_data_for_convert, NULL
							   , audio_codecCtx->channels, audio_frame->nb_samples*2, AV_SAMPLE_FMT_S16, 0);
		 this->size_buffer_for_convert = *data_size;
		 printf("_____________________\ndata_size=%d; ret=%d; nb_samples=%d;\n", *data_size, ret, audio_frame->nb_samples);
		}
	 ret = swr_convert( this->audio_swr
					  , this->dst_data_for_convert, audio_frame->nb_samples
					  , (const uint8_t**)audio_frame->data, audio_frame->nb_samples
					  ); //printf("swr_convert -> %d\n", ret);
	 *data = this->dst_data_for_convert[0];
 } else {*data = audio_frame->data[0];
		}

 //if(len1 == -1) {if(debug_mode) cout << "\n____Erreur avcodec_decode_audio!\n"; return 0;}
 //if(debug_mode) cout << "\n____audio_decode_frame(" << buf_size << ", " << data_size << ", " << len1 << ")\n";
 return ret;
}

//______________________________________________________________________________
int FFMpegVideo::get_audio_packet(AVPacket **pkt)
{if(L_audio_pkt.Vide()) {return -1;}
 *pkt = L_audio_pkt.Premier_E();
 L_audio_pkt.Retirer_index(0);
 return 1;
}

//______________________________________________________________________________
int FFMpegVideo::put_audio_packet(AVPacket *pkt)
{/*if(av_dup_packet(pkt) < 0) {
    if(debug_mode) cout << "\n___av_dup_packet(pkt) < 0\n";
	return -1;
  } XXX DEBUG becauseav_dup_packet is a hack... what is it used for here? */
 L_audio_pkt.Ajouter_a_la_fin( pkt );
 return 1;
}

//______________________________________________________________________________
void FFMpegVideo::Process_audio_packets()
{//Lock(); 
 //if(debug_mode) printf("Process_audio_packets->Lock---");
 alx_element_liste<AVPacket*> *it     
                            , *it_fin = L_audio_pkt.Fin();
 AVPacket *pkt;
 int got_frame, data_size;
 uint8_t *data;

// if(debug_mode) cout << "\nProcess packets...";
 for(it = L_audio_pkt.Premier();it != it_fin; it = it->svt)
  {pkt = it->E();
   audio_decode_frame(audio_codecCtx/*, audio_buf, size_audio_buf,*/, &got_frame, &data, &data_size, pkt);
		if(!got_frame) continue;
		if(data_size < 0) {if(debug_mode) cout << "Error av_samples_get_buffer_size -> " << data_size << endl; continue;}
   if(debug_mode) cout << "*(" << data_size << ") ";
   Info_for_sound_CB_Put_New( &info_for_sound_CB
							, data, data_size //, audio_frame->data[0], data_size
							, pkt->pts, pkt->dts, pkt->duration);   
  }

// if(debug_mode) cout << "End of processing\n";
 for(it = L_audio_pkt.Premier();it != it_fin; it = it->svt)
  {pkt = it->E();
   //if(pkt->data) av_free_packet(pkt);
	av_free_packet(pkt);
  }

// Release packet list
 L_audio_pkt.Vider();
 //if(debug_mode) printf("---Process_audio_packets->UnLock\n"); 
 //UnLock();
}

//______________________________________________________________________________
void FFMpegVideo::Lock()
{mutex_audio.lock();
}

//______________________________________________________________________________
void FFMpegVideo::UnLock()
{mutex_audio.unlock();
}

//______________________________________________________________________________
const double FFMpegVideo::get_delta_from_first_time() const {
	if(info_for_sound_CB.first_t == 0) return -1;
	return (double)(clock() - info_for_sound_CB.first_t) / (double)CLOCKS_PER_SEC;
}

//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
Mutex::Mutex(void)
{            
    #if defined (WIN32) || defined (_WIN32_WCE)
    hMutex = CreateMutex(
    NULL,           // no security attributes
    FALSE,          // initially not owned
    NULL);          // no name

    if (hMutex == NULL) 
    {
     printf("MUTEX CREATION ERROR !!!");
    }
    #endif

    #if defined (unix) || defined (__APPLE__)
    pthread_mutex_init(&m, NULL);
    #endif
}
        
Mutex::~Mutex(void)
{
    #if defined (WIN32) || defined (_WIN32_WCE)
    CloseHandle(hMutex);
    #endif

    #if defined (unix) || defined (__APPLE__)
    pthread_mutex_destroy(&m);
    #endif
}
        
void Mutex::lock(void)
{
    #if defined (WIN32) || defined (_WIN32_WCE)
    WaitForSingleObject(hMutex, INFINITE);
    #endif
    
    #if defined (unix) || defined (__APPLE__)
    pthread_mutex_lock(&m);
    #endif
}
        
void Mutex::unlock(void)
{
    #if defined (WIN32) || defined (_WIN32_WCE)
    if (! ReleaseMutex(hMutex)) 
    { 
        // Deal with error.
    } 
    #endif
    
    #if defined (unix) || defined (__APPLE__)
    pthread_mutex_unlock(&m);
    #endif
}


signed char F_CALLBACKAPI FFMPEG_FMOD_Stream_Info_audio(FSOUND_STREAM *stream, void *buff, int len, void *userdata)
{//clock_t time0 = clock(); printf("<%d - ", time0);
 Info_for_sound_CB *info_for_sound_CB = (Info_for_sound_CB*)userdata;
 Mutex             *mutex             = Info_for_sound_CB_Get_mutex(info_for_sound_CB);
   mutex->lock();
   info_for_sound_CB->num_last_buffer++;

// Is the audio information up to date with video?  
 int total_size = Info_for_sound_CB_Size_buffers(info_for_sound_CB);
 info_for_sound_CB->not_enough = (total_size < len);
 bool XXX_DEBUG_init = false;
 double t_video = info_for_sound_CB->video_pts * info_for_sound_CB->time_base_video;


 //printf("%x -> %d;", buff, FSOUND_Stream_GetPosition(stream));
 if( info_for_sound_CB->nb == 0 ) 
  {memset(buff, 0, len);
   mutex->unlock();
   //clock_t time1 = clock();
   //printf("%d - dt=%d>\n", time1, time1-time0);
   return TRUE;
  } else {if(info_for_sound_CB->first_t == 0) {
			 //info_for_sound_CB->first_t           = clock(); // Synch with clock
			 info_for_sound_CB->audio_clock_start = (double)info_for_sound_CB->Tab_wav[Info_for_sound_CB_First_index(info_for_sound_CB)].pts * info_for_sound_CB->time_base_audio;
			 XXX_DEBUG_init = true;
			}
		}

 total_size = Info_for_sound_CB_Size_buffers(info_for_sound_CB);
 // is there enough informations?
 int nb_zero, dec_buff, remain, nb_octets_to_skip = 0;
 // If synchronization to achieve, insert some zeros at the start...
	
 nb_zero  = 0;
 dec_buff = 0;
 remain   = len;

 // Insert audio informations
 if(total_size >= remain) {
		 nb_zero  = 0;
		 dec_buff = 0;
		 remain   = remain;
  } else {nb_zero   = remain - total_size;
          cout << "___________ NOT ENOUGH AUDIO INFO, insert " << nb_zero << " zeros ______________\n";
          memset((char*)buff+dec_buff, 0, nb_zero);
		  dec_buff += nb_zero;
		  remain    = total_size;
		  info_for_sound_CB->not_enough = true;
         }

   // Copy informations to the buffer 
   //printf("Copy start time %f at %x - ", (double)info_for_sound_CB->Tab_wav[Info_for_sound_CB_First_index(info_for_sound_CB)].pts * info_for_sound_CB->time_base_audio, buff);
   while(remain > 0)
    {//printf("*");
	 double time_tmp = (double)info_for_sound_CB->Tab_wav[Info_for_sound_CB_First_index(info_for_sound_CB)].pts * info_for_sound_CB->time_base_audio;
	 int nb_copied = Info_for_sound_CB_Read(info_for_sound_CB, buff, dec_buff, remain);
	 if(nb_copied <= 0) {printf("On ne copie rien (%i, requis %i, soit disant dispo %i)!!!\n", nb_copied, len, total_size); break;}
     dec_buff += nb_copied;
	 remain	  -= nb_copied;
    }
  
 mutex->unlock();

 if(XXX_DEBUG_init) {
	 info_for_sound_CB->first_t = clock();
	 //printf("______ Start time is %d ______", info_for_sound_CB->first_t);
	} //else {printf("/*\\");}
 //clock_t time1 = clock();
 //printf("%d - dt=%d>\n", time1, time1-time0);
 return TRUE;
}

/*
signed char F_CALLBACKAPI FFMPEG_FMOD_Stream_Info_audio(FSOUND_STREAM *stream, void *buff, int len, void *userdata)
{Info_for_sound_CB *info_for_sound_CB = (Info_for_sound_CB*)userdata;
 Mutex             *mutex             = Info_for_sound_CB_Get_mutex(info_for_sound_CB);
   mutex->lock();
   info_for_sound_CB->num_last_buffer++;

// Is the audio information up to date with video?  
 int total_size = Info_for_sound_CB_Size_buffers(info_for_sound_CB);
 info_for_sound_CB->not_enough = (total_size < len);
 bool has_synch_audio_video;
 double t_video = info_for_sound_CB->video_pts * info_for_sound_CB->time_base_video;
 if(info_for_sound_CB->synchronize_with_video) {
	 t_video  = (double)(clock() - info_for_sound_CB->t0) / (double)CLOCKS_PER_SEC;
	 info_for_sound_CB->video_pts = t_video / info_for_sound_CB->time_base_video;
	}

 if( !(has_synch_audio_video = !Info_for_sound_CB_Synch_audio_to_video(info_for_sound_CB, len)) ) 
  {memset(buff, 0, len);
   mutex->unlock();
   return FALSE;
 } else {if(info_for_sound_CB->first_t == 0) {info_for_sound_CB->first_t = clock();}
		}

 total_size = Info_for_sound_CB_Size_buffers(info_for_sound_CB);
 // is there enough informations?
 int nb_zero, dec_buff, remain, nb_octets_to_skip = 0;
 // If synchronization to achieve, insert some zeros at the start...
 if(info_for_sound_CB->synchronize_with_video) {
	 double t_audio = (double)info_for_sound_CB->Tab_wav[info_for_sound_CB->first].pts * info_for_sound_CB->time_base_audio,
			t_audio_next = (double)info_for_sound_CB->Tab_wav[(info_for_sound_CB->first + 1)%info_for_sound_CB->nb_buffers].pts * info_for_sound_CB->time_base_audio;
	 
	 nb_zero = (int)((double)info_for_sound_CB->Tab_wav[info_for_sound_CB->first].size*(t_video-t_audio)/(t_audio_next-t_audio));
	 if(nb_zero < 0 || nb_zero > len) {
		 if(nb_zero < 0) {
			 nb_zero = 16*(-nb_zero/16);
			 printf("Insert %d zeros !!!\n", nb_zero);
			 nb_zero = nb_zero<len?nb_zero:len;
			 memset(buff, 0, nb_zero);
			 dec_buff = nb_zero;
			 remain = len - nb_zero;
			} else {std::cout << "Too much zeros to insert, skip the packet\n";
				    while(  info_for_sound_CB->nb > 0
						 && nb_zero > info_for_sound_CB->Tab_wav[info_for_sound_CB->was not start].size) {
						 nb_zero -= info_for_sound_CB->Tab_wav[info_for_sound_CB->first].size;
						 Info_for_sound_CB_Release(info_for_sound_CB);
						 std::cout << ".";
						}
					total_size = Info_for_sound_CB_Size_buffers(info_for_sound_CB);
					nb_zero = 0;
					dec_buff = 0;
					remain   = len;
				   }
		} else { nb_octets_to_skip = 16*(nb_zero/16);
				 printf("Tape ta synchro par coupe :\n\tnb_zero : %d\n\t deb : %d\n\tsize : %d\n", nb_zero, info_for_sound_CB->Tab_wav[info_for_sound_CB->first].deb, info_for_sound_CB->Tab_wav[info_for_sound_CB->first].size);
				 while(  info_for_sound_CB->nb > 0
					  && nb_octets_to_skip > info_for_sound_CB->Tab_wav[info_for_sound_CB->first].size) {
					 nb_octets_to_skip -= info_for_sound_CB->Tab_wav[info_for_sound_CB->first].size;
					 std::cout << "\tSkip a packet... still " << nb_octets_to_skip << " bytes to skip\n";
					 Info_for_sound_CB_Release(info_for_sound_CB);
					}
				 total_size = Info_for_sound_CB_Size_buffers(info_for_sound_CB);
				 nb_zero  = 0;
				 dec_buff = 0;
				 remain   = len;
				 if(info_for_sound_CB->nb) 
					 Info_for_sound_CB_Maj_size( info_for_sound_CB
											   , info_for_sound_CB->first
											   , nb_octets_to_skip
											   , info_for_sound_CB->Tab_wav[info_for_sound_CB->first].size + info_for_sound_CB->Tab_wav[info_for_sound_CB->first].deb - nb_octets_to_skip
											   );
				 printf("\tnb_octets_to_skip : %d\n\tlen : %d\n", nb_octets_to_skip, len);
				}
	 printf("Skip %d bytes, add %d zeros / %d to synchronize audio with video\n\tt_video = %f\n\tt_audio  = %f\n\tt_audio_next = %f\n", nb_octets_to_skip, nb_zero, len, t_video, t_audio, t_audio_next);
	 // XXX mise à jour de remain, dec_buff and co... voir plus bas !
	} else {nb_zero  = 0;
		    dec_buff = 0;
		    remain   = len;
		   }

 // Insert audio informations
 if(total_size >= remain) {
	if (!info_for_sound_CB->synchronize_with_video) {
		 nb_zero  = 0;
		 dec_buff = 0;
		 remain   = remain;
		} else {printf("\tsynchro...\n");}
  } else {nb_zero   = remain - total_size;
          cout << "___________ NOT ENOUGH AUDIO INFO, insert " << nb_zero << " zeros ______________\n";
          memset((char*)buff+dec_buff, 0, nb_zero);
		  dec_buff += nb_zero;
		  remain    = total_size;
		  info_for_sound_CB->not_enough = true;
         }
   // Copy informations to the buffer 
   while(remain > 0)
    {//printf("*");
	 int nb_copied = Info_for_sound_CB_Read(info_for_sound_CB, buff, dec_buff, remain);
	 if(nb_copied <= 0) {printf("On ne copie rien (%i, requis %i, soit disant dispo %i)!!!\n", nb_copied, len, total_size); break;}
	 //printf("%i", nb_copied);
     dec_buff += nb_copied;
	 remain	  -= nb_copied;
    }
  
 //printf("---FFMPEG_FMOD_Stream_Info_audio->UnLock\n");
 if(info_for_sound_CB->synchronize_with_video) {
	 //FSOUND_Stream_SetPosition(stream, FSOUND_Stream_GetPosition(stream));
	 std::cout << "Synchro again?\n\thas_synch_audio_video : " << has_synch_audio_video
		       << "\n\t(nb_zero == len) : " << (nb_zero == len) << "\n";
	}
 info_for_sound_CB->synchronize_with_video = info_for_sound_CB->synchronize_with_video && (!has_synch_audio_video || (nb_zero == len));
 mutex->unlock();
 if(info_for_sound_CB->first_t == 0) {info_for_sound_CB->first_t = clock();}
 return TRUE;
}
*/
FSOUND_STREAMCALLBACK Get_FFMPEG_FMOD_Stream_Info_audio()
{return &FFMPEG_FMOD_Stream_Info_audio;}

