#include "krad_flac.h"



int krad_flac_encoder_finish(krad_flac_t *flac, unsigned char *encode_buffer) {

	flac->bytes = 0;

	if (encode_buffer != NULL) {
		flac->encode_buffer = encode_buffer;
	} else {
		flac->encode_buffer = NULL;
	}
	
	if (flac->finished == 0) {
		FLAC__stream_encoder_finish(flac->encoder);
		flac->finished = 1;
	}

	
	return flac->bytes;

}


void krad_flac_encoder_destroy(krad_flac_t *flac) {

	krad_flac_encoder_finish(flac, NULL);
	FLAC__stream_encoder_delete(flac->encoder);
	
	free(flac);
}

krad_flac_t *krad_flac_encoder_create(int channels, int sample_rate, int bit_depth) {

	krad_flac_t *flac = calloc(1, sizeof(krad_flac_t));
	
	flac->channels = channels;
	flac->sample_rate = sample_rate;
	flac->bit_depth = bit_depth;
	
	flac->encoder = FLAC__stream_encoder_new();

	FLAC__stream_encoder_set_channels (flac->encoder, flac->channels);
	FLAC__stream_encoder_set_bits_per_sample (flac->encoder, flac->bit_depth);
	FLAC__stream_encoder_set_sample_rate (flac->encoder, flac->sample_rate);
	FLAC__stream_encoder_set_compression_level	(flac->encoder, 7);

	if (0) {
		FLAC__stream_encoder_init_file (flac->encoder, "/home/oneman/kode/testfile6.flac", NULL, NULL);
	} else {

		FLAC__StreamEncoderInitStatus ret = FLAC__stream_encoder_init_stream(flac->encoder, (FLAC__StreamEncoderWriteCallback) krad_flac_encoder_write_callback, (FLAC__StreamEncoderSeekCallback) krad_flac_encoder_seek_callback, (FLAC__StreamEncoderTellCallback) krad_flac_encoder_tell_callback, NULL, flac);

		if (ret != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
			printf("Flac encoder init fail\n");
			exit(1);
		}
	}

	return flac;
	
}

FLAC__StreamEncoderSeekStatus krad_flac_encoder_seek_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 absolute_byte_offset, void *client_data) {

	krad_flac_t *flac = (krad_flac_t *)client_data;
	
	printf("got seek callback %lu\n", absolute_byte_offset);
	
	if (absolute_byte_offset < 666 + 23) {
	
		flac->streaminfo_rewrite = absolute_byte_offset - 666;
		return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
		
	} else {
	
	     return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;
	}

}
 
FLAC__StreamEncoderTellStatus krad_flac_encoder_tell_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 *absolute_byte_offset, void *client_data) {

	*absolute_byte_offset = 666;

	//printf("got tell callback\n");

	return FLAC__STREAM_ENCODER_TELL_STATUS_OK;

}

FLAC__StreamEncoderWriteStatus krad_flac_encoder_write_callback ( 
	const FLAC__StreamEncoder *encoder, const FLAC__byte fbuffer[], 
	unsigned bytes, unsigned samples, unsigned current_frame, void *client_data) {

	krad_flac_t *flac = (krad_flac_t *)client_data;
	
	//printf("Flac write callback with %d bytes, %d samples, %d current frame \n", bytes, samples, current_frame);
	
	//additional header metadata -- this code is above so we dont write streaminfo block twice..
	if ((samples == 0) && (flac->have_min_header == 1) && (flac->total_frames == 0) && (flac->streaminfo_rewrite == 0)) {
		printf("additional!! %d\n", bytes);
		memcpy(flac->header + flac->header_size, fbuffer, bytes);
		flac->header_size += bytes;
	}
	
	// Initial streaminfo block
	if ((samples == 0) && (flac->have_min_header == 0) && (bytes == FLAC_STREAMINFO_BLOCK_SIZE)) {
		// flac METADATA_BLOCK_STREAMINFO
		memcpy(flac->streaminfo_block, fbuffer, FLAC_STREAMINFO_BLOCK_SIZE);
		memcpy(flac->header, FLAC_MARKER, 4);
		memcpy(flac->header + 4, flac->streaminfo_block, FLAC_STREAMINFO_BLOCK_SIZE);

		memcpy(flac->min_header, flac->header, FLAC_MINIMAL_HEADER_SIZE);
		// the following marks the streaminfo block as the final metadata block
		flac->min_header[4] = '\x80';

		flac->header_size = FLAC_MINIMAL_HEADER_SIZE;
		flac->have_min_header = 1;
	}
	
	// needs to be above the below
	if (flac->streaminfo_rewrite == 0) {
		flac->bytes += bytes;
		flac->total_bytes += bytes;
		if (flac->encode_buffer != NULL) {
			memcpy(flac->encode_buffer, fbuffer, bytes);
		}
	}
	
	// rewrites streaminfo block with samples, min/max blocksize and md5
	if (flac->streaminfo_rewrite) {
		printf("yeehaw %d bytes on ye streaminfo rewrite at %d\n", bytes, flac->streaminfo_rewrite);


		if (bytes == 16) {
		
					int b;
			char testbuffer[256];
	
			printf("flac encoder write callback called with 0 samples, this is metadata\n");
			if (bytes < 128) {
				memset(testbuffer, '\0', 256);
				for(b = 0; b < bytes; b++) {
					sprintf(testbuffer + (b * 2), "%02x", fbuffer[b]);
				}
			}
			printf(" --%s-- \n", testbuffer);
		
		}

		memcpy(flac->streaminfo_block + flac->streaminfo_rewrite, fbuffer, bytes);
		memcpy(flac->min_header + 4, flac->streaminfo_block, FLAC_STREAMINFO_BLOCK_SIZE);
		// the following marks the streaminfo block as the final metadata block
		flac->min_header[4] = '\x80';
		memcpy(flac->header + 4, flac->streaminfo_block, FLAC_STREAMINFO_BLOCK_SIZE);

		flac->streaminfo_rewrite = 0;
	}
	
	if (samples > 0) {
		flac->total_frames += samples;
	}
		
	return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
	
}

int krad_flac_encoder_frames_remaining(krad_flac_t *flac) {

	return flac->total_frames_input - flac->total_frames;

}

void krad_flac_encode_info(krad_flac_t *flac) {

	printf("Encoded Frames: %llu\n", flac->total_frames);
	printf("Encoded Bytes: %llu\n", flac->total_bytes);
	printf("Sample Rate: %d\n", flac->sample_rate);
	printf("Bit Depth: %d\n", flac->bit_depth);
	printf("Channels: %d\n", flac->channels);
	printf("\n");
}

void krad_flac_encode_test(krad_flac_t *flac) {

	int test_frame_count;
	char *samples;
	
	test_frame_count = 4096;
	
	samples = calloc(1, flac->channels * test_frame_count * 4);

	FLAC__stream_encoder_process_interleaved(flac->encoder, (const FLAC__int32 *)samples, test_frame_count);

	free(samples);

}

int krad_flac_encoder_read_min_header(krad_flac_t *flac, unsigned char *buffer) {

	memcpy(buffer, flac->min_header, FLAC_MINIMAL_HEADER_SIZE);

	return FLAC_MINIMAL_HEADER_SIZE;

}

int krad_flac_encoder_read_header(krad_flac_t *flac, unsigned char *buffer) {

	memcpy(buffer, flac->header, flac->header_size);

	return flac->header_size;

}


int krad_flac_encode(krad_flac_t *flac, float *audio, int frames, unsigned char *encode_buffer) {

	int ret;
	int num_samples;
	int s;
	int bitshifter;
	double scaled_value;

	if (flac->bit_depth == 24) {
		bitshifter = 8;
	} else {
		bitshifter = 16;
	}

	num_samples = frames * flac->channels;

	for (s = 0; s < num_samples; s++) {
		scaled_value = (audio[s] * (8.0 * 0x10000000));
		flac->int32_samples[s] = (int) (lrint (scaled_value) >> bitshifter);
	}
	
	flac->bytes = 0;
	flac->encode_buffer = encode_buffer;
	
	flac->total_frames_input += frames;
	
	FLAC__stream_encoder_process_interleaved(flac->encoder, (const FLAC__int32 *)flac->int32_samples, frames);
	
	ret = flac->bytes;
	flac->bytes = 0;
	
	return ret;
		
}




/********* decoder ***/


void krad_flac_decoder_int24_to_float_array (const int *in, float *out, int len) {

	while (len)	{	
		len--;
		out[len] = (float) ((in[len] << 8) / (8.0 * 0x10000000));
	};

}

void krad_flac_decoder_int16_to_float_array (const int *in, float *out, int len) {

	while (len)	{	
		len--;
		out[len] = (float) ((in[len] << 16) / (8.0 * 0x10000000));
	};

}

void krad_flac_decoder_info(krad_flac_t *flac) {
	printf("frames decoded: %llu\n", flac->total_frames_input);
	printf("total frames: %llu\n", flac->total_frames);
	printf("Sample Rate: %d\n", flac->sample_rate);
	printf("Bit Depth: %d\n", flac->bit_depth);
	printf("Channels: %d\n", flac->channels);
	printf("\n");
}

void krad_flac_decoder_callback_error (const FLAC__StreamDecoder *flacdecoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {

	//krad_flac_t *flac = (krad_flac_t *)client_data;
	
	printf ("Flac decoder fail: %s\n", FLAC__StreamDecoderErrorStatusString[status]);

}

void krad_flac_decoder_metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data) {

	krad_flac_t *flac = (krad_flac_t *)client_data;

	flac->channels = metadata->data.stream_info.channels;
	flac->sample_rate = metadata->data.stream_info.sample_rate;
	flac->bit_depth = metadata->data.stream_info.bits_per_sample;
	
	if (metadata->data.stream_info.total_samples != 0) {
		flac->total_frames = metadata->data.stream_info.total_samples;
		printf("has total frames of %llu\n", flac->total_frames);
	}

	krad_flac_decoder_info(flac);
}

FLAC__StreamDecoderWriteStatus krad_flac_decoder_callback_write (const FLAC__StreamDecoder *flacdecoder,
																 const FLAC__Frame *frame, 
																 const FLAC__int32 * const fbuffer[],
																 void *client_data) {

	krad_flac_t *flac = (krad_flac_t *)client_data;

	//printf("flac decoder callback write with %d frames\n", frame->header.blocksize);
	int c;	
	
	flac->frames = frame->header.blocksize;
	flac->total_frames_input += flac->frames;
	
	// for each channnel

	for (c = 0; c < flac->channels; c++) {
		if (flac->bit_depth == 16) { // if 16bit
			krad_flac_decoder_int16_to_float_array ((const int *)fbuffer[c], flac->output_buffer[c], frame->header.blocksize);
		} else {
			krad_flac_decoder_int24_to_float_array ((const int *)fbuffer[c], flac->output_buffer[c], frame->header.blocksize);
		}
	}
	
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;

}


FLAC__StreamDecoderReadStatus krad_flac_decoder_callback_read (const FLAC__StreamDecoder *flacdecoder, FLAC__byte fbuffer[],
															   size_t *bytes, void *client_data) {
	
	krad_flac_t *flac = (krad_flac_t *)client_data;

	//printf("flac decoder callback read with %zu bytes\n", *bytes);

	if (!(*bytes <= (flac->decode_buffer_len - flac->decode_buffer_pos))) {
		*bytes = flac->decode_buffer_len - flac->decode_buffer_pos;
	}

	memcpy(fbuffer, flac->decode_buffer + flac->decode_buffer_pos, *bytes);
	flac->decode_buffer_pos += *bytes;

	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;

}

int krad_flac_decode(krad_flac_t *flac, unsigned char *encoded_buffer, int len, float **audio) {


	flac->decode_buffer = encoded_buffer;
	flac->decode_buffer_len = len;
	flac->decode_buffer_pos = 0;
	
	flac->output_buffer = audio;

	FLAC__bool ret = FLAC__stream_decoder_process_single ( flac->decoder );

	if (ret == false) {
		printf("flac decoder fail\n");
	}

	if (flac->decode_buffer_len != flac->decode_buffer_pos) {
		printf("Flac mhrmm\n");
//		memove(flac->decode_buffer, flac->decode_buffer + flac->decode_buffer_pos, flac->decode_buffer_len - flac->decode_buffer_pos);
	} else {
//		flac->decode_buffer_pos = 0;
	}

	return flac->frames;

}

krad_flac_t *krad_flac_decoder_create() {

	krad_flac_t *flac = calloc(1, sizeof(krad_flac_t));

	flac->decoder = FLAC__stream_decoder_new();

	FLAC__StreamDecoderInitStatus ret = FLAC__stream_decoder_init_stream (flac->decoder,
		krad_flac_decoder_callback_read, NULL, NULL, NULL, NULL, krad_flac_decoder_callback_write, krad_flac_decoder_metadata_callback, krad_flac_decoder_callback_error, flac );

	if (ret != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		printf("Flac decoder fail\n");
	}

	return flac;

}


void krad_flac_decoder_destroy(krad_flac_t *flac) {

	FLAC__stream_decoder_finish ( flac->decoder );
	FLAC__stream_decoder_delete ( flac->decoder );

	free(flac);

}



