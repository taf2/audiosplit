#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sndfile.h>

static int split_by_silence(const char *file);
static sf_count_t locate_silence_offset(SNDFILE *sndfile, SF_INFO *sndfile_info, sf_count_t soff, sf_count_t *offset);

int main(int argc, char **argv) {

	if (argc < 2) {
		fprintf(stderr, "usage: %s input_file\n", argv[0]);
		return 1;
	}
	return split_by_silence(argv[1]);
}



static int split_by_silence(const char *file) {
	SNDFILE *sndfile = NULL;
	SF_INFO sndfile_info;
  sf_count_t offset = 0;
  sf_count_t prev_offset = 0;
  sf_count_t read_frames = 0;
  sf_count_t read_total = 0;

	memset(&sndfile_info, 0, sizeof(SF_INFO));
	sndfile = sf_open(file, SFM_READ, &sndfile_info);

	if (!sndfile) {
		fprintf(stderr, "Failed to open: %s with error: %s\n", file, sf_strerror(NULL));
		return 1;
	}

  //printf("frame rate: %d\n", sndfile_info.samplerate);

  do {
    prev_offset = offset;
    read_frames = locate_silence_offset(sndfile, &sndfile_info, offset, &offset);
    read_total += read_frames;
    //printf("p: %llu, o %llu\n", prev_offset, offset);
    if (prev_offset == offset) {
      printf("silence from: %llu to %llu\n", offset, read_total);
    }
  } while (read_frames > 0);

	printf("\n");

	sf_close(sndfile);

	return 0;
}

static sf_count_t locate_silence_offset(SNDFILE *sndfile, SF_INFO *sndfile_info, sf_count_t soff, sf_count_t *offset) {
	int i, wave_length, avg = 0;
	size_t buffer_size = 1024; // read 32 frames at a time?
	int *buffer = (int*)malloc(buffer_size * sizeof(int));
	sf_count_t read_bytes;

	read_bytes = sf_readf_int(sndfile, buffer, buffer_size);
	for ( i = 0; i < read_bytes; ++i ) {
    wave_length = buffer[i] / sndfile_info->samplerate;
    avg += abs(wave_length);
		//printf("%.2d ", wave_length);
	}
  //printf("r: %llu, avg: %d\n", read_bytes, avg);

  if (read_bytes > 0 && avg > 0) {
    avg /= read_bytes;
    printf("\twave: %d\n", (int)avg);
    fflush(stdout);
  }

  if (avg < 70) {
    //*offset += soff; // start of the sequence is silence
  }
  else {
    *offset += read_bytes; // end of the sequence is possibly silence
  }

	free(buffer);

	return read_bytes;
}
