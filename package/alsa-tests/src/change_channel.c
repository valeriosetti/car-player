/*
 * i2s_test.c — ALSA-based I2S driver test
 *
 * Plays a 1 kHz sine tone at 44100 Hz sample rate.
 * The PCM handle is opened ONCE and kept alive for the whole run.
 * Channel count is changed mid-stream via snd_pcm_hw_params() on the
 * existing handle, without ever closing it.
 *
 * Channel sequence (SEGMENT_SECS seconds each):
 *   2 ch  →  8 ch  →  2 ch
 *
 * Build:
 *   gcc -o i2s_test i2s_test.c -lasound -lm -Wall -O2
 *
 * Usage:
 *   ./i2s_test [device]        (default device: hw:0,0)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <alsa/asoundlib.h>

/* ── tuneable constants ──────────────────────────────────────────────────── */
#define SAMPLE_RATE      44100
#define TONE_HZ          1000
#define SEGMENT_SECS     2            /* seconds per channel-count segment    */
#define PERIOD_FRAMES    512          /* frames per ALSA period               */
#define PERIODS_PER_BUF  4            /* ring-buffer = N × period             */
#define MAX_CHANNELS     8            /* largest channel count we ever use    */

/* Signed 16-bit little-endian — adjust if your codec needs another format   */
#define PCM_FORMAT       SND_PCM_FORMAT_S16_LE
typedef int16_t          sample_t;
#define SAMPLE_MAX       32767

/* ── initial configuration (blank slate) ────────────────────────────────── */
/*
 * initial_hw_params
 *
 * Called exactly once at startup.  Uses snd_pcm_hw_params_any() to start
 * from a full capability mask, then constrains down to our requirements.
 *
 * sw_params are set here and only here: they live above the driver and
 * never need touching again.
 */
static int initial_hw_params(snd_pcm_t *pcm, unsigned int channels)
{
    int err;
    snd_pcm_hw_params_t *hw;
    snd_pcm_sw_params_t *sw;
    unsigned int rate         = SAMPLE_RATE;
    snd_pcm_uframes_t period  = PERIOD_FRAMES;
    snd_pcm_uframes_t buffer  = PERIOD_FRAMES * PERIODS_PER_BUF;

    snd_pcm_hw_params_alloca(&hw);
    if ((err = snd_pcm_hw_params_any(pcm, hw)) < 0) goto err;
    if ((err = snd_pcm_hw_params_set_access(pcm, hw,
                 SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) goto err;
    if ((err = snd_pcm_hw_params_set_format(pcm, hw, PCM_FORMAT)) < 0)
        goto err;
    if ((err = snd_pcm_hw_params_set_channels(pcm, hw, channels)) < 0)
        goto err;
    if ((err = snd_pcm_hw_params_set_rate_near(pcm, hw, &rate, 0)) < 0)
        goto err;
    if (rate != SAMPLE_RATE)
        fprintf(stderr, "Warning: driver settled on %u Hz (requested %u)\n",
                rate, SAMPLE_RATE);
    if ((err = snd_pcm_hw_params_set_period_size_near(pcm, hw,
                                                       &period, 0)) < 0)
        goto err;
    if ((err = snd_pcm_hw_params_set_buffer_size_near(pcm, hw,
                                                       &buffer)) < 0)
        goto err;
    if ((err = snd_pcm_hw_params(pcm, hw)) < 0) goto err;

    snd_pcm_hw_params_get_period_size(hw, &period, 0);
    snd_pcm_hw_params_get_buffer_size(hw, &buffer);
    printf("[ALSA] initial    rate=%u Hz  channels=%u  "
           "period=%lu frames  buffer=%lu frames\n",
           rate, channels, period, buffer);

    /* sw_params: set once, never touched again — these are ALSA middleware,
     * not driver state, so re-applying them on channel switches is pointless */
    snd_pcm_sw_params_alloca(&sw);
    snd_pcm_sw_params_current(pcm, sw);
    snd_pcm_sw_params_set_start_threshold(pcm, sw, period);
    snd_pcm_sw_params_set_avail_min(pcm, sw, period);
    if ((err = snd_pcm_sw_params(pcm, sw)) < 0) {
        fprintf(stderr, "snd_pcm_sw_params: %s\n", snd_strerror(err));
        return err;
    }

    return 0;

err:
    fprintf(stderr, "initial_hw_params (ch=%u): %s\n",
            channels, snd_strerror(err));
    return err;
}

/* ── mid-stream reconfiguration (refinement path) ───────────────────────── */
/*
 * refine_hw_params
 *
 * Called when switching channel count on a live (but dropped) handle.
 * Seeds the constraint set from the current hardware state (_current, not
 * _any), then changes only channel count — everything else stays pinned.
 * This hits the driver's refinement path rather than a full reset, which
 * is the whole point of this function existing separately from initial_hw_params.
 *
 * No sw_params here: they don't go to the driver and haven't changed.
 */
static int refine_hw_params(snd_pcm_t *pcm, unsigned int new_channels)
{
    int err;
    snd_pcm_hw_params_t *hw;

    snd_pcm_hw_params_alloca(&hw);
    if ((err = snd_pcm_hw_params_current(pcm, hw)) < 0) {
        fprintf(stderr, "snd_pcm_hw_params_current: %s\n", snd_strerror(err));
        return err;
    }
    if ((err = snd_pcm_hw_params_set_channels(pcm, hw, new_channels)) < 0) {
        fprintf(stderr, "snd_pcm_hw_params_set_channels(%u): %s\n",
                new_channels, snd_strerror(err));
        return err;
    }
    if ((err = snd_pcm_hw_params(pcm, hw)) < 0) {
        fprintf(stderr, "snd_pcm_hw_params (ch=%u): %s\n",
                new_channels, snd_strerror(err));
        return err;
    }

    printf("[ALSA] refined    channels=%u\n", new_channels);
    return 0;
}

/* ── sine generation ─────────────────────────────────────────────────────── */
/*
 * fill_period
 *
 * Fills 'frames' frames of interleaved sine into buf.
 * The sine is written identically to all 'channels' channels so any
 * channel can be probed on a logic analyser.
 * phase_accum is updated in place; it stays in [0, 2π) to avoid drift.
 */
static void fill_period(sample_t *buf,
                        snd_pcm_uframes_t frames,
                        unsigned int channels,
                        double *phase_accum)
{
    const double step = 2.0 * M_PI * TONE_HZ / SAMPLE_RATE;

    for (snd_pcm_uframes_t f = 0; f < frames; f++) {
        sample_t s = (sample_t)(SAMPLE_MAX * sin(*phase_accum));
        for (unsigned int c = 0; c < channels; c++)
            buf[f * channels + c] = s;
        *phase_accum += step;
        if (*phase_accum >= 2.0 * M_PI)
            *phase_accum -= 2.0 * M_PI;
    }
}

/* ── playback segment ────────────────────────────────────────────────────── */
/*
 * play_segment
 *
 * Writes 'seconds' of audio to the already-prepared PCM handle.
 * phase_accum is shared with the caller so the waveform is phase-continuous
 * across segment boundaries — useful for spotting glitches on a scope.
 *
 * Timing note: there is no explicit sleep or wait call here, but the loop
 * still takes exactly 'seconds' of real time to complete.  snd_pcm_writei()
 * on a blocking fd will block whenever the ring buffer is full — i.e. when
 * the kernel has accepted (PERIODS_PER_BUF × PERIOD_FRAMES) frames but the
 * hardware DMA hasn't consumed enough of them yet to free a slot.  The loop
 * therefore runs at hardware speed: it can never get more than one buffer-
 * size ahead of the DAC clock.  No explicit wait is needed.
 */
static int play_segment(snd_pcm_t *pcm,
                        unsigned int channels,
                        int seconds,
                        double *phase_accum)
{
    const long total_frames = (long)SAMPLE_RATE * seconds;
    /* Allocate for the worst-case channel count so we never realloc */
    sample_t *buf = malloc(PERIOD_FRAMES * MAX_CHANNELS * sizeof(sample_t));
    if (!buf) { perror("malloc"); return -ENOMEM; }

    long frames_done = 0;
    int  err = 0;

    while (frames_done < total_frames) {
        snd_pcm_uframes_t to_write = PERIOD_FRAMES;
        long remaining = total_frames - frames_done;
        if ((long)to_write > remaining)
            to_write = (snd_pcm_uframes_t)remaining;

        fill_period(buf, to_write, channels, phase_accum);

        snd_pcm_sframes_t written = snd_pcm_writei(pcm, buf, to_write);
        if (written < 0) {
            fprintf(stderr, "snd_pcm_writei: %s — attempting recovery\n",
                    snd_strerror((int)written));
            written = snd_pcm_recover(pcm, (int)written, 0);
            if (written < 0) {
                fprintf(stderr, "snd_pcm_recover: %s\n",
                        snd_strerror((int)written));
                err = (int)written;
                break;
            }
            continue;   /* retry the same block */
        }
        frames_done += written;
    }

    free(buf);
    return err;
}

/* ── reconfigure without closing ─────────────────────────────────────────── */
/*
 * switch_channels
 *
 * Stops the running stream, applies a new hw_params (new channel count),
 * and brings the handle back to PREPARED — all without closing the PCM.
 *
 * snd_pcm_drop()    — stops immediately, discards the ring-buffer contents.
 *                     Use this rather than snd_pcm_drain() which would block
 *                     and potentially stall the I2S clock during the switch.
 * refine_hw_params() — seeds constraints from current hw state (_current,
 *                     not _any) and changes only the channel count, hitting
 *                     the driver's refinement path rather than a full reset.
 * snd_pcm_prepare()  — moves the handle from SETUP → PREPARED so the next
 *                     snd_pcm_writei() can restart playback cleanly.
 */
static int switch_channels(snd_pcm_t *pcm, unsigned int new_channels)
{
    int err;

    printf("[switch] stopping stream ... ");
    fflush(stdout);

    if ((err = snd_pcm_drop(pcm)) < 0) {
        fprintf(stderr, "snd_pcm_drop: %s\n", snd_strerror(err));
        return err;
    }

    printf("refining hw_params (channels=%u) ... ", new_channels);
    fflush(stdout);

    /* Use the refinement path: seeds from current state, changes only
     * channel count — exercises a different driver path than a full reset. */
    if ((err = refine_hw_params(pcm, new_channels)) < 0)
        return err;

    /* refine_hw_params -> snd_pcm_hw_params() leaves handle in SETUP state;
     * snd_pcm_prepare() advances it to PREPARED.                           */
    if ((err = snd_pcm_prepare(pcm)) < 0) {
        fprintf(stderr, "snd_pcm_prepare: %s\n", snd_strerror(err));
        return err;
    }

    printf("ready.\n");
    return 0;
}

/* ── main ────────────────────────────────────────────────────────────────── */

typedef struct {
    unsigned int channels;
    int          seconds;
} Segment;

int main(int argc, char *argv[])
{
    const char *device = (argc > 1) ? argv[1] : "hw:0,0";

    const Segment schedule[] = {
        { 2, SEGMENT_SECS },
        { 8, SEGMENT_SECS },
        { 2, SEGMENT_SECS },
    };
    const int num_segments = (int)(sizeof(schedule) / sizeof(schedule[0]));

    printf("I2S test — 1 kHz sine @ %u Hz  (PCM stays open throughout)\n",
           SAMPLE_RATE);
    printf("Device  : %s\n", device);
    printf("Sequence: ");
    for (int i = 0; i < num_segments; i++)
        printf("%uch×%ds%s", schedule[i].channels, schedule[i].seconds,
               i < num_segments - 1 ? " → " : "\n\n");

    /* ── open the PCM handle exactly once ───────────────────────────────── */
    snd_pcm_t *pcm = NULL;
    int err;

    if ((err = snd_pcm_open(&pcm, device,
                            SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "snd_pcm_open(%s): %s\n", device, snd_strerror(err));
        return EXIT_FAILURE;
    }

    /* Configure for the first segment — full blank-slate init */
    if ((err = initial_hw_params(pcm, schedule[0].channels)) < 0)
        goto fail;

    if ((err = snd_pcm_prepare(pcm)) < 0) {
        fprintf(stderr, "snd_pcm_prepare: %s\n", snd_strerror(err));
        goto fail;
    }

    double phase = 0.0;   /* shared phase — keeps waveform continuous */

    for (int seg = 0; seg < num_segments; seg++) {
        unsigned int ch = schedule[seg].channels;
        int secs        = schedule[seg].seconds;

        /* Switch channels on all segments after the first */
        if (seg > 0) {
            printf("\n[Segment %d/%d] switching to %u channels ...\n",
                   seg + 1, num_segments, ch);
            if ((err = switch_channels(pcm, ch)) < 0)
                goto fail;
        } else {
            printf("[Segment %d/%d] starting with %u channels\n",
                   seg + 1, num_segments, ch);
        }

        printf("[Segment %d/%d] playing %ds at %u channels\n",
               seg + 1, num_segments, secs, ch);

        if ((err = play_segment(pcm, ch, secs, &phase)) < 0)
            goto fail;
    }

    /* Drain remaining buffered samples before closing */
    snd_pcm_drain(pcm);
    snd_pcm_close(pcm);

    printf("\nDone.\n");
    return EXIT_SUCCESS;

fail:
    snd_pcm_close(pcm);
    return EXIT_FAILURE;
}
