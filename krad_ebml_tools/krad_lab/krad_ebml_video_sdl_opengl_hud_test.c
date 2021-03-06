#include "krad_sdl_opengl_display.h"
#include "krad_ebml.h"
#include "krad_vpx.h"
#include "krad_dirac.h"
#include "krad_theora.h"
#include "krad_flac.h"
#include "krad_vorbis.h"
#include "krad_opus.h"
#include "krad_gui.h"
#include "krad_audio.h"

#define APPVERSION "Krad EBML VIDEO SDL OPENGL DISPLAY HUD TEST 0.1"

typedef struct krad_ebml_video_player_hud_test_St krad_ebml_video_player_hud_test_t;

struct krad_ebml_video_player_hud_test_St {

	krad_audio_t *audio;
	kradgui_t *kradgui;
	krad_vorbis_t *krad_vorbis;
	
	float *samples[2];
	
	
};


void krad_ebml_video_player_hud_test_audio_callback(int frames, void *userdata) {

	krad_ebml_video_player_hud_test_t *hudtest = (krad_ebml_video_player_hud_test_t *)userdata;

	int len;
	
	len = krad_vorbis_decoder_read_audio(hudtest->krad_vorbis, 0, (char *)hudtest->samples[0], frames * 4);

	if (len) {
		kradaudio_write (hudtest->audio, 0, (char *)hudtest->samples[0], len );
	}

	len = krad_vorbis_decoder_read_audio(hudtest->krad_vorbis, 1, (char *)hudtest->samples[1], frames * 4);

	if (len) {
		kradaudio_write (hudtest->audio, 1, (char *)hudtest->samples[1], len );
	}
	
	//kradgui_add_current_track_time_ms(hudtest->kradgui, (frames / 44.1));
}


int main (int argc, char *argv[]) {

	struct stat st;
	krad_ebml_video_player_hud_test_t *hudtest;
	krad_codec_type_t audio_codec;
	krad_codec_type_t video_codec;

	krad_sdl_opengl_display_t *krad_opengl_display;
	krad_vpx_decoder_t *krad_vpx_decoder;
	krad_dirac_t *krad_dirac;
	krad_theora_decoder_t *krad_theora_decoder;
	kradebml_t *krad_ebml;
	kradgui_t *kradgui;
	
	krad_audio_t *audio;
	krad_audio_api_t audio_api;
	
	krad_vorbis_t *krad_vorbis;
	krad_flac_t *krad_flac;
	krad_opus_t *krad_opus;
	SDL_Event event;
	
	cairo_surface_t *cst;
	cairo_t *cr;
	
	int hud_width, hud_height;
	int hud_stride;
	int hud_byte_size;
	unsigned char *hud_data;

	unsigned char *buffer;
	
	int bytes;
	float *audio_data;
	int audio_frames;
	
	int i;
	int audio_packets;
	int video_packets;
	int shutdown;
	int packet_time_ms;
	int packet_time_offset;
	int last_packet_time_ms;
	int nosleep;
	struct timespec packet_time;
	struct timespec sleep_time;
	
	int dirac_output_unset;
	int render_hud;
	int c;
	
	float temp_peak;
	
	render_hud = 1;
	dirac_output_unset = true;
		
	if (argc < 2) {
		printf("specify a video file\n");
		exit(1);
	}

	if (stat(argv[1], &st) != 0) {
		printf("cant find file %s\n", argv[1]);
		exit(1);
	}
	
	shutdown = 0;
	
	hud_width = 720;
	hud_height = 340;

	audio_packets = 0;
	video_packets = 0;

	audio_data = calloc(1, 8192 * 4 * 4);
	hudtest = calloc(1, sizeof(krad_ebml_video_player_hud_test_t));
	
	hud_stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, hud_width);
	hud_byte_size = hud_stride * hud_height;
	hud_data = calloc (1, hud_byte_size);
	cst = cairo_image_surface_create_for_data (hud_data, CAIRO_FORMAT_ARGB32, hud_width, hud_height, hud_stride);
	
	
	hudtest->samples[0] = malloc(4 * 8192);
	hudtest->samples[1] = malloc(4 * 8192);
	
	audio_api = JACK;
	audio = kradaudio_create("Krad EBML Video HUD Test Player", KOUTPUT, audio_api);
	
	kradgui = kradgui_create(hud_width, hud_height);

	//kradgui_add_item(kradgui, REEL_TO_REEL);
	//kradgui_add_item(kradgui, PLAYBACK_STATE_STATUS);

	buffer = malloc(1000000);
	krad_dirac = krad_dirac_decoder_create();
	krad_vpx_decoder = krad_vpx_decoder_create();
	krad_ebml = kradebml_create();
	krad_flac = krad_flac_decoder_create();
	kradebml_open_input_file(krad_ebml, argv[1]);
	//kradebml_open_input_stream(krad_ebml, "deimos.kradradio.com", 8080, "/teststream.krad");
	
	kradebml_debug(krad_ebml);

	kradgui_set_total_track_time_ms(kradgui, (krad_ebml->duration / 1e9) * 1000);
	kradgui->render_timecode = 1;
	
	video_codec = krad_ebml_track_codec(krad_ebml, 0);
	audio_codec = krad_ebml_track_codec(krad_ebml, 1);
	
	printf("video codec is %d and audio codec is %d\n", video_codec, audio_codec);
	
	if (audio_codec == KRAD_FLAC) {
		bytes = kradebml_read_audio_header(krad_ebml, 1, buffer);
		printf("got flac header bytes of %d\n", bytes);
		//exit(1);
		krad_flac_decode(krad_flac, buffer, bytes, audio_data);
	}
	
	if (audio_codec == KRAD_VORBIS) {
		printf("got vorbis header bytes of %d %d %d\n", krad_ebml->vorbis_header1_len, krad_ebml->vorbis_header2_len, krad_ebml->vorbis_header3_len);
		krad_vorbis = krad_vorbis_decoder_create(krad_ebml->vorbis_header1, krad_ebml->vorbis_header1_len, krad_ebml->vorbis_header2, krad_ebml->vorbis_header2_len, krad_ebml->vorbis_header3, krad_ebml->vorbis_header3_len);
	}
	
	if (audio_codec == KRAD_OPUS) {
		bytes = kradebml_read_audio_header(krad_ebml, 1, buffer);
		printf("got opus header bytes of %d\n", bytes);
		krad_opus = kradopus_decoder_create(buffer, bytes, 44100.0f);
	}
	
	if (video_codec == KRAD_THEORA) {
		printf("got theora header bytes of %d %d %d\n", krad_ebml->theora_header1_len, krad_ebml->theora_header2_len, krad_ebml->theora_header3_len);
		krad_theora_decoder = krad_theora_decoder_create(krad_ebml->theora_header1, krad_ebml->theora_header1_len, krad_ebml->theora_header2, krad_ebml->theora_header2_len, krad_ebml->theora_header3, krad_ebml->theora_header3_len);
	} else {
		krad_theora_decoder = NULL;
	}

	//krad_opengl_display = krad_sdl_opengl_display_create(APPVERSION, 1920, 1080, krad_ebml->vparams.width, krad_ebml->vparams.height);
	krad_opengl_display = krad_sdl_opengl_display_create(APPVERSION, krad_ebml->vparams.width, krad_ebml->vparams.height, krad_ebml->vparams.width, krad_ebml->vparams.height);
	
	krad_opengl_display->hud_width = hud_width;
	krad_opengl_display->hud_height = hud_height;
	krad_opengl_display->hud_data = hud_data;
	
	hudtest->audio = audio;
	hudtest->krad_vorbis = krad_vorbis;
	hudtest->kradgui = kradgui;
	
	if (audio_codec == KRAD_VORBIS) {
		kradaudio_set_process_callback(audio, krad_ebml_video_player_hud_test_audio_callback, hudtest);
	}	
		
	if (audio_api == JACK) {
		krad_jack_t *jack = (krad_jack_t *)audio->api;
		kradjack_connect_port(jack->jack_client, "Krad EBML Video HUD Test Player:Left", "desktop_recorder:input_1");
		kradjack_connect_port(jack->jack_client, "Krad EBML Video HUD Test Player:Right", "desktop_recorder:input_2");
	}
	
	kradgui_reset_elapsed_time(kradgui);
	
	while (nestegg_read_packet(krad_ebml->ctx, &krad_ebml->pkt) > 0) {
		
		nestegg_packet_track(krad_ebml->pkt, &krad_ebml->pkt_track);
		nestegg_packet_count(krad_ebml->pkt, &krad_ebml->pkt_cnt);
		nestegg_packet_tstamp(krad_ebml->pkt, &krad_ebml->pkt_tstamp);

		packet_time_ms = (krad_ebml->pkt_tstamp / 1e9) * 1000;
		
		if (video_packets == 0) {
		
			nosleep = true;
		
			packet_time_offset = packet_time_ms;
		
			if (packet_time_offset > 0) {
				packet_time_offset -= 1000;
			}

		}

		if (video_packets == 100) {

			nosleep = false;		
		
			if (packet_time_offset > 0) {
				packet_time_offset += 1000;
			}
		}

		
		packet_time_ms -= packet_time_offset;
		
		packet_time.tv_sec = (packet_time_ms - (packet_time_ms % 1000)) / 1000;
		packet_time.tv_nsec = (packet_time_ms % 1000) * 1000000;
		
		if (krad_ebml->pkt_track == krad_ebml->video_track) {


			//printf("\npacket time ms %d :: %ld : %ld\n", packet_time_ms % 1000, packet_time.tv_sec, packet_time.tv_nsec);
			kradgui_set_current_track_time_ms(kradgui, packet_time_ms);
		
			video_packets++;

			for (i = 0; i < krad_ebml->pkt_cnt; ++i) {

				nestegg_packet_data(krad_ebml->pkt, i, &buffer, &krad_ebml->size);

				if (video_codec == KRAD_VP8) {

					krad_vpx_decoder_decode(krad_vpx_decoder, buffer, krad_ebml->size);
				
					if (krad_vpx_decoder->img != NULL) {

						//printf("vpx img: %d %d %d\n", krad_vpx_decoder->img->stride[0],  krad_vpx_decoder->img->stride[1],  krad_vpx_decoder->img->stride[2]); 
						//cr = cairo_create(cst);
						//kradgui->cr = cr;
						//kradgui_render(kradgui);
						//cairo_destroy(cr);

						//krad_sdl_opengl_display_hud_item(krad_opengl_display, hud_data);
						krad_sdl_opengl_display_render(krad_opengl_display, krad_vpx_decoder->img->planes[0], krad_vpx_decoder->img->stride[0], krad_vpx_decoder->img->planes[1], krad_vpx_decoder->img->stride[1], krad_vpx_decoder->img->planes[2], krad_vpx_decoder->img->stride[2]);

						//usleep(13000);
					}
				}
				
				if (video_codec == KRAD_THEORA) {

					krad_theora_decoder_decode(krad_theora_decoder, buffer, krad_ebml->size);
				
					//if (krad_vpx_decoder->img != NULL) {

						//printf("vpx img: %d %d %d\n", krad_vpx_decoder->img->stride[0],  krad_vpx_decoder->img->stride[1],  krad_vpx_decoder->img->stride[2]); 
						//cr = cairo_create(cst);
						//kradgui->cr = cr;
						//kradgui_render(kradgui);
						//cairo_destroy(cr);

						//krad_sdl_opengl_display_hud_item(krad_opengl_display, hud_data);
					krad_sdl_opengl_display_render(krad_opengl_display, krad_theora_decoder->ycbcr[0].data, krad_theora_decoder->ycbcr[0].stride, krad_theora_decoder->ycbcr[1].data, krad_theora_decoder->ycbcr[1].stride, krad_theora_decoder->ycbcr[2].data, krad_theora_decoder->ycbcr[2].stride);

						//usleep(13000);
					//}
				}

							
				if (video_codec == KRAD_DIRAC) {
				
					//printf("dirac packet size %zu\n", krad_ebml->size);		

					krad_dirac_decode(krad_dirac, buffer, krad_ebml->size);

					if ((krad_dirac->format != NULL) && (dirac_output_unset == true)) {
						switch (krad_dirac->format->chroma_format) {
	
							case SCHRO_CHROMA_444:
								krad_sdl_opengl_display_set_input_format(krad_opengl_display, PIX_FMT_YUV444P);
							case SCHRO_CHROMA_422:
								krad_sdl_opengl_display_set_input_format(krad_opengl_display, PIX_FMT_YUV422P);
							case SCHRO_CHROMA_420:
								break;
								// default
								//krad_sdl_opengl_display_set_input_format(krad_sdl_opengl_display, PIX_FMT_YUV420P);

						}
				
						dirac_output_unset = false;				
					}

					if (krad_dirac->frame != NULL) {
						krad_sdl_opengl_display_render(krad_opengl_display, krad_dirac->frame->components[0].data, krad_dirac->frame->components[0].stride, krad_dirac->frame->components[1].data, krad_dirac->frame->components[1].stride, krad_dirac->frame->components[2].data, krad_dirac->frame->components[2].stride);
					}

				}


				for (c = 0; c < 2; c++) {
					temp_peak = read_peak(audio, KOUTPUT, c);
					if (temp_peak >= kradgui->output_peak[c]) {
						kradgui->output_peak[c] = temp_peak;
					} else {
						kradgui->output_peak[c] -= 3.5;
						//kradgui->output_peak[c] = kradgui->output_peak[c] * 0.95;
					}
				}


				if (render_hud == 1) {
				
					krad_opengl_display->hud_data = hud_data;
				
					cr = cairo_create(cst);
					kradgui->cr = cr;
					kradgui->overlay = 1;
					kradgui_render(kradgui);
				
					kradgui_render_meter (kradgui, 102, 122, 83, kradgui->output_peak[0]);
					kradgui_render_meter (kradgui, 248, 122, 83, kradgui->output_peak[1]);
				
					kradgui_render_vtest (kradgui);
				
	//				printf("peak %f %f\n", kradgui->output_peak[0] * 100.0f, kradgui->output_peak[1] * 100.0f);
					cairo_destroy(cr);
				} else {
					krad_opengl_display->hud_data = NULL;
				
				}
				
				if (nosleep == false) {
					//printf("hi\n");
					kradgui_update_elapsed_time(kradgui);
					sleep_time = timespec_diff(kradgui->elapsed_time, packet_time);

//					if (sleep_time.tv_nsec > 16600000 * 1) {
						//sleep_time.tv_nsec -= 16600000 * 1;
						//printf("\nsleep time nsec is %ld\n", sleep_time.tv_nsec);
						nanosleep(&sleep_time, NULL);
	//				}
				}

				krad_sdl_opengl_draw_screen( krad_opengl_display );
//				nanosleep(&sleep_time, NULL);
				
				
			}
		}
		
		
		if (krad_ebml->pkt_track == krad_ebml->audio_track) {
			audio_packets++;

			nestegg_packet_data(krad_ebml->pkt, 0, &buffer, &krad_ebml->size);
			printf("\nAudio packet! %zu bytes\n", krad_ebml->size);

			if (audio_codec == KRAD_VORBIS) {
				krad_vorbis_decoder_decode(krad_vorbis, buffer, krad_ebml->size);
			}
			
			if (audio_codec == KRAD_FLAC) {
				audio_frames = krad_flac_decode(krad_flac, buffer, krad_ebml->size, audio_data);
				kradaudio_write (audio, 0, (char *)audio_data, audio_frames * 4 );
				kradaudio_write (audio, 1, (char *)audio_data, audio_frames * 4 );
			}
			
			if (audio_codec == KRAD_OPUS) {
				kradopus_decoder_set_speed(krad_opus, 100);
				kradopus_write_opus(krad_opus, buffer, krad_ebml->size);
		
				int c;

				bytes = -1;			
				while (bytes != 0) {
					for (c = 0; c < 2; c++) {
				
						bytes = kradopus_read_audio(krad_opus, c + 1, (char *)audio_data, 960 * 4);
						if (bytes) {
							printf("\nAudio data! %zu samplers\n", bytes / 4);
							kradaudio_write (audio, c, (char *)audio_data, bytes );
						}
					}
				}
			}
			
		}
		
		if (krad_ebml->pkt_track == krad_ebml->video_track) {
			printf("Timecode: %f :: Video Packets %d Audio Packets: %d buf frames %zu\r", krad_ebml->pkt_tstamp / 1e9, video_packets, audio_packets, kradaudio_buffered_frames(audio));
			fflush(stdout);
		}
		
		nestegg_free_packet(krad_ebml->pkt);



		while ( SDL_PollEvent( &event ) ){
			switch( event.type ){
				/* Look for a keypress */
				case SDL_KEYDOWN:
				    /* Check the SDLKey values and move change the coords */
				    switch( event.key.keysym.sym ){
				        case SDLK_LEFT:
				            break;
				        case SDLK_RIGHT:
				            break;
				        case SDLK_UP:
				            break;
				        case SDLK_j:
				        	render_hud = 0;
				            break;
				        case SDLK_h:
				        	render_hud = 1;
				            break;
				        case SDLK_q:
				        	shutdown = 1;
				            break;
				        case SDLK_f:
				        	
				            break;
				        default:
				            break;
				    }
				    break;

				case SDL_KEYUP:
				    switch( event.key.keysym.sym ) {
				        case SDLK_LEFT:
				            break;
				        case SDLK_RIGHT:
				            break;
				        case SDLK_UP:
				            break;
				        case SDLK_DOWN:
				            break;
				        default:
				            break;
				    }
				    break;

				case SDL_MOUSEMOTION:
					//printf("mouse!\n");
					kradgui->cursor_x = event.motion.x;
					kradgui->cursor_y = event.motion.y;
					break;	
				
				default:
				    break;
			}
		}

		if (shutdown == 1) {
			break;
		}
	}
	
	printf("\n");
	
	krad_vpx_decoder_destroy(krad_vpx_decoder);
	krad_dirac_decoder_destroy(krad_dirac);
	kradebml_destroy(krad_ebml);
	krad_sdl_opengl_display_destroy(krad_opengl_display);
	kradgui_destroy(kradgui);

	// must be before vorbis
	kradaudio_destroy(audio);

	if (audio_codec == KRAD_VORBIS) {
		krad_vorbis_decoder_destroy(krad_vorbis);
	}
	
	krad_flac_decoder_info(krad_flac);
	krad_flac_decoder_destroy(krad_flac);
	
	//free(buffer);
	cairo_surface_destroy(cst);
	
	free(hudtest->samples[0]);
	free(hudtest->samples[1]);
	
	free(hudtest);
	free(audio_data);
	
	if (krad_theora_decoder != NULL) {
		krad_theora_decoder_destroy(krad_theora_decoder);
	}
	
	return 0;

}
