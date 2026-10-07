// gcc sine_switch.c -o sine_switch -lasound -lm
// Plays a 1 kHz sine on ONE PCM handle: 16-bit, then reconfigures in place to
// 24-bit -- no snd_pcm_close() in between, so the formatter widget can stay
// powered across the format switch (the case speaker-test can't reach).
#include <alsa/asoundlib.h>
#include <math.h>
#include <stdint.h>

#define DEV  "hw:0,0"
#define RATE 44100
#define CH   2
#define FREQ 1000.0
#define DURATION 5.0

static void play(snd_pcm_t *pcm, snd_pcm_format_t fmt, double secs)
{
	int bytes = snd_pcm_format_physical_width(fmt) / 8;	/* 2 or 4 */
	snd_pcm_uframes_t frames = (snd_pcm_uframes_t)(RATE * secs);
	void *buf = malloc(frames * CH * bytes);
	int err;

	/* hw_params() internally does hw_free() first, so this reconfigures
	 * the already-open handle in place (SETUP -> OPEN -> PREPARED). */
	if ((err = snd_pcm_set_params(pcm, fmt, SND_PCM_ACCESS_RW_INTERLEAVED,
				      CH, RATE, 1, 200000)) < 0) {
		fprintf(stderr, "set_params: %s\n", snd_strerror(err));
		exit(1);
	}

	for (snd_pcm_uframes_t i = 0; i < frames; i++)
		for (int c = 0; c < CH; c++) {
			double v = 0.3 * sin(2.0 * M_PI * FREQ * i / RATE);
			if (bytes == 2)
				((int16_t *)buf)[i * CH + c] = v * 32767;
			else	/* S24_LE: 24-bit sample in a 32-bit word */
				((int32_t *)buf)[i * CH + c] = v * 8388607;
		}

	snd_pcm_sframes_t n = snd_pcm_writei(pcm, buf, frames);
	if (n < 0)
		n = snd_pcm_recover(pcm, n, 0);
	if (n < 0) {
		fprintf(stderr, "writei: %s\n", snd_strerror(n));
		exit(1);
	}
	snd_pcm_drain(pcm);	/* finish playback, return to SETUP */
	free(buf);
}

int main(void)
{
	snd_pcm_t *pcm;
	int err;

	if ((err = snd_pcm_open(&pcm, DEV, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "open: %s\n", snd_strerror(err));
		return 1;
	}

	printf("width: 16-bit - channels: %d - rate: %d - duration:%f \n",
		CH, RATE, DURATION);
	play(pcm, SND_PCM_FORMAT_S16_LE, DURATION);
	printf("width: 24-bit - channels: %d - rate: %d - duration:%f \n",
		CH, RATE, DURATION);
	play(pcm, SND_PCM_FORMAT_S24_LE, DURATION);
	printf("width: 16-bit - channels: %d - rate: %d - duration:%f \n",
		CH, RATE, DURATION);
	play(pcm, SND_PCM_FORMAT_S16_LE, DURATION);
	printf("width: 24-bit - channels: %d - rate: %d - duration:%f \n",
		CH, RATE, DURATION);
	play(pcm, SND_PCM_FORMAT_S24_LE, DURATION);

	snd_pcm_close(pcm);
	return 0;
}
