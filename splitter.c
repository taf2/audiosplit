/*
 * Copyright (C) 2011 Todd A Fisher <todd.fisher@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Given an input wav file locate quiet portions of audio stream and clip them, 
 * outputing a new stream of audio chunks that include only the noisy parts of the audio file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sndfile.h>
#include "splitter.h"

#define usage() fprintf(stderr, "usage: splitter -i file.wav -t 30 -d 0.5 -w out.asciiwave\n");

static SilenceDetector* silence_detector_parse_cli(int argc, char **argv) {
  int ch;
  double duration = 1; // seconds
  int threshold = 30;
  const char *output_wave = NULL;
  const char *input_wav_file = NULL;

  opterr = 0;

  while ((ch = getopt(argc, argv, "i:t:d:w:")) != -1) {
    printf("ch: %d\n", ch);
    switch (ch) {
    case 'i':
      printf("using input wav file\n");
      input_wav_file = optarg;
      break;
    case 't':
      printf("using threshold\n");
      threshold = atoi(optarg);
      break;
    case 'd':
      printf("using duration\n");
      duration = atof(optarg);
      break;
    case 'w':
      printf("output wave\n");
      output_wave = optarg;
      break;
    case '?':
    default:
      usage();
      exit(0);
    }
  }
  printf("ch: %d\n", ch);
  argc -= optind;
  argv += optind;

  if (argc != 0) {
    usage();
    exit(1);
  }

  printf("max silence duration: %.2f\nsilence threshold: %d\noutput wave: %s\n", duration, threshold, output_wave);

  return silence_detector_new(input_wav_file, threshold, duration, output_wave);
}

int main(int argc, char **argv) {

  SilenceDetector *detector = silence_detector_parse_cli(argc, argv);

  silence_detector_split_audio(detector);

  silence_detector_free(detector);

  return 0;
}

static SilenceDetector* silence_detector_new(const char *file, int threshold, double duration, const char *output_wave) {
  SilenceDetector *ptr = (SilenceDetector*)malloc(sizeof(SilenceDetector));
  memset(ptr, 0, sizeof(SilenceDetector));

	ptr->sndfile = sf_open(file, SFM_READ, &(ptr->sndfile_info));

	if (!ptr->sndfile) {
		fprintf(stderr, "Failed to open: %s with error: %s\n", file, sf_strerror(NULL));
    free(ptr);
		return NULL;
	}

  ptr->file_name = file;
  ptr->threshold = threshold;
  ptr->duration = duration;
  ptr->output_wave = output_wave;

  /*
  printf("file: %s\n\tframes:\t\t%llu\n\tsamplerate:\t%d\n\tchannels:\t%d\n\tformat:\t\t%d\n\tsections:\t%d\n\tseekable:\t%d\n", file, ptr->sndfile_info.frames,
                                                                                                         ptr->sndfile_info.samplerate,
                                                                                                         ptr->sndfile_info.channels,
                                                                                                         ptr->sndfile_info.format,
                                                                                                         ptr->sndfile_info.sections,
                                                                                                         ptr->sndfile_info.seekable);
  printf("duration: %.2f seconds\n", (double)ptr->sndfile_info.frames / (double)ptr->sndfile_info.samplerate);
  // itunes for example will round seconds up... should we do this too?
  printf("duration: %.2f seconds\n", ceil((double)ptr->sndfile_info.frames / (double)ptr->sndfile_info.samplerate));
  */
	ptr->buffer_size = 512; // read 16 frames at a time?
	ptr->buffer = (int*)malloc((ptr->buffer_size*2) * sizeof(int)); // 2x size we are not sure how the frame is being read/written in libsndfile...

  return ptr;
}

static void silence_detector_free(SilenceDetector *ptr) {
  if (ptr) {
    sf_close(ptr->sndfile);
    free(ptr->buffer);

    if (ptr->out_sndfile) {
      sf_close(ptr->out_sndfile);
    }

    if (ptr->last_chunk_file_name) {
      free(ptr->last_chunk_file_name);
    }

    free(ptr);
  }
}

static int silence_detector_split_audio(SilenceDetector *ptr) {
  sf_count_t offset = 0;
  sf_count_t prev_offset = 0;
  sf_count_t read_frames = 0;
  sf_count_t read_total = 0;
  FILE *owave = NULL;

  if (ptr->output_wave) {
    owave = fopen(ptr->output_wave, "wb");
    if (!owave) {
      fprintf(stderr, "Error opening output wave file: %s\n", ptr->output_wave);
      exit(4);
    }
  }

  silence_detector_create_audio_chunk(ptr);

  do {
    prev_offset = offset;
    read_frames = silence_detector_locate_silence_offset(ptr, offset, &offset, owave);
    read_total += read_frames;
    //printf("p: %llu, o %llu\n", prev_offset, offset);
    if (prev_offset == offset) {
      //printf("silence from: %llu to %llu\n", offset, read_total);
    }
  } while (read_frames > 0);

	//printf("\n");
  if (owave) {
    fclose(owave);
  }

  return 0;
}

static void silence_detector_create_audio_chunk(SilenceDetector *ptr) {
  char *out_chunk_file_name = silence_detector_create_chunk_file_name(ptr);
	ptr->out_sndfile = sf_open(out_chunk_file_name, SFM_WRITE, &(ptr->sndfile_info));
  if (ptr->last_chunk_file_name) {
    free(ptr->last_chunk_file_name);
  }
  ptr->last_chunk_file_name = out_chunk_file_name;

  if (!ptr->out_sndfile) {
    fprintf(stderr, "Failed to create audio chunk!\n");
    exit(3);
  }
}

static char* silence_detector_create_chunk_file_name(SilenceDetector *ptr) {
  size_t file_name_length = (strlen(ptr->file_name)+1 + strlen("chunk-") + 1 + ptr->chunk_count + 1);
  char *out_chunk_file_name = (char*)malloc(file_name_length* sizeof(char));
  memset(out_chunk_file_name, 0, file_name_length);
  snprintf(out_chunk_file_name, file_name_length, "%s.chunk%d", ptr->file_name, ptr->chunk_count++);
  printf("create new chunk: %s\n", out_chunk_file_name);
  return out_chunk_file_name;
}

static double read_sound_file_duration(const char *filepath) {
	SNDFILE *sndfile;
	SF_INFO sndfile_info;
  double duration;

	sndfile = sf_open(filepath, SFM_READ, &sndfile_info);
  if (sndfile) {
    //duration_in_seconds = (int)ceill( sndfile_info.frames / sndfile_info.samplerate );
    duration = (double)sndfile_info.frames / (double)sndfile_info.samplerate;
    sf_close(sndfile);
    return duration;
  }

  return 0;
}

static sf_count_t silence_detector_locate_silence_offset(SilenceDetector *ptr, sf_count_t soff, sf_count_t *offset, FILE *owave) {
	int i, j, wave_length, avg = 0;
  double chunk_duration;
	sf_count_t read_frames;

  memset(ptr->buffer, 0, ptr->buffer_size * sizeof(int));
	read_frames = sf_readf_int(ptr->sndfile, ptr->buffer, ptr->buffer_size);

	for ( i = 0; i < read_frames; ++i ) {
    wave_length = ptr->buffer[i] / (ptr->sndfile_info.samplerate);
    avg += abs(wave_length);
    if (owave) {
      // write out the wave amps
      for (j = 0; j < wave_length; ++j) {
        fwrite("+", sizeof(char), 1, owave);
      }
      fwrite("\n", sizeof(char), 1, owave);
    }
		//printf("%.2d ", wave_length);
	}
  //printf("r: %llu, avg: %d\n", read_frames, avg);

  if (read_frames > 0 && avg > 0) {
    avg /= read_frames;
    //printf("\t%d\n", (int)avg);
    //fflush(stdout);
  }

  if (avg > ptr->threshold) {
    //printf("\tnoisy here: %llu until %llu\n", *offset, ((*offset) + read_frames));
    *offset += read_frames; // end of the sequence is possibly silence
    // open the sound file if necessary
    if (!ptr->out_sndfile) {
      silence_detector_create_audio_chunk(ptr);
    }
    // write these bytes to our file 
    sf_writef_int(ptr->out_sndfile, ptr->buffer, read_frames);
  }
  else {
    //printf("starting silence at %llu\n", *offset);
    if (ptr->out_sndfile) {
      sf_close(ptr->out_sndfile);
      ptr->out_sndfile = NULL;
      // TODO: get the duration of the chunk and compare to the filter duration
      chunk_duration = read_sound_file_duration(ptr->last_chunk_file_name);
      if (chunk_duration < ptr->duration) {
        printf("chunk %s:%.2f is smaller than duration: %.2f\n", ptr->last_chunk_file_name, chunk_duration, ptr->duration);
        unlink(ptr->last_chunk_file_name);
      }
    }
  }

	return read_frames;
}
