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
 * Merge wav files, assumes all wave files are in the same rate
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sndfile.h>

static int copy_frames_from_to(int *buffer, size_t buffer_size, SNDFILE *src, SNDFILE *dest);

static int update_output_info(const char *name, SF_INFO *output);

int main(int argc, char **argv) {
	SNDFILE *input, *out = NULL;
	SF_INFO info, outinfo;
  size_t buffer_size = 4096;
  int *buffer = NULL;
  int i = 0;


  if (argc < 4) {
    fprintf(stderr, "usage: %s a1.wav a2.wav ... out.wav\n", argv[0]);
    return 1;
  }

  memset(&outinfo, 0, sizeof(SF_INFO));

  for (i = 1; i < argc-1; ++i ) {
    printf("scan: %s\n", argv[i]);
    if (update_output_info(argv[i], &outinfo)) {
      return 1;
    }
  }

  printf("%s will be %.2f seconds\n", argv[3], ((double)outinfo.frames / (double)outinfo.samplerate));

  out = sf_open(argv[3], SFM_WRITE, &outinfo);
  if (!out) {
    fprintf(stderr, "unable to write file: %s\n", argv[3]);
    return 3;
  }

	buffer = (int*)malloc((buffer_size*2) * sizeof(int)); // 2x size we are not sure how the frame is being read/written in libsndfile...

  memset(&info, 0, sizeof(SF_INFO));
  for (i = 1; i < argc-1; ++i) {
    input = sf_open(argv[i], SFM_READ, &info);
    if (!input) {
      fprintf(stderr, "failed to open input: %s\n", argv[i]);
      return 4;
    }
    copy_frames_from_to(buffer, buffer_size, input, out);
    printf("wrote from src: %s\n", argv[i]);
    sf_close(input);
  }

  free(buffer);
  sf_close(out);

  return 0;
}

// read information about the input file and update the output info
static int update_output_info(const char *name, SF_INFO *output) {
	SNDFILE *input = NULL;
  SF_INFO info;

  memset(&info, 0, sizeof(SF_INFO));

  input = sf_open(name, SFM_READ, &info);
  if (!input) {
    fprintf(stderr, "failed to open input: %s\n", name);
    return 2;
  }

  printf("%s is %.2f seconds\n", name, ((double)info.frames / (double)info.samplerate));
  output->frames     = output->frames + info.frames;
  output->samplerate = info.samplerate;
  output->channels   = info.channels;
  output->format     = info.format;
  output->sections   = output->sections + info.sections;
  output->seekable   = info.seekable;

  sf_close(input);
  return 0;
}

static int copy_frames_from_to(int *buffer, size_t buffer_size, SNDFILE *src, SNDFILE *dest) {
	sf_count_t read_frames;

  do {
    memset(buffer, 0, buffer_size * sizeof(int));
    read_frames = sf_readf_int(src, buffer, buffer_size);
    if (read_frames > 0) {
      sf_writef_int(dest, buffer, read_frames);
    }
  } while (read_frames > 0);

  return 0;
}
