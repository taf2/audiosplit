#ifndef __SPLITTER_H
#define __SPLITTER_H

typedef struct _silence_detector {
	SNDFILE *sndfile;
	SF_INFO sndfile_info;
  const char *file_name;
  int threshold;
  double max_duration, min_duration;
	size_t buffer_size; // read 32 frames at a time?
	int *buffer;

  char *last_chunk_file_name;
  const char *output_wave;

  int chunk_count;

	SNDFILE *out_sndfile;
} SilenceDetector;

static SilenceDetector* silence_detector_new(const char *file, size_t buffer_size, int threshold, double min_duration, double max_duration, const char *output_wave);
static void silence_detector_free(SilenceDetector *ptr);
static int silence_detector_split_audio(SilenceDetector *ptr);
static sf_count_t silence_detector_locate_silence_offset(SilenceDetector *ptr, sf_count_t soff, sf_count_t *offset, FILE *owave);
static void silence_detector_create_audio_chunk(SilenceDetector *ptr);
static char* silence_detector_create_chunk_file_name(SilenceDetector *ptr);
static double read_sound_file_duration(const char *filepath);

static void silence_detector_verify_last_chunk_duration(SilenceDetector *ptr);

#endif
