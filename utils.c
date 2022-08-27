#include "utils.h"
#include "math.h"
// #include <stdio.h>

#ifndef SAMPLE_T
#define SAMPLE_T float
#endif

// pow(10, dbGain / 40);
SAMPLE_T utils_db_to_gain(SAMPLE_T db) {
    SAMPLE_T gain = pow(10.0, db / 10.0);
    // printf("utils_db_to_gain(%f) == %f\n", db, gain);
    return gain;
}

SAMPLE_T utils_gain_to_db(SAMPLE_T gain) {
    SAMPLE_T db = 10 * log10(gain);
    // printf("utils_gain_to_db(%f) == %f\n", gain, db);
    return db;
}

/**
 * @brief convert midi-style pitch to frequency
 * MIDI pitch 69 = 440Hz.
 * semitones = pitch - 69
 * return 440 * pow(2, semitones / 12)
 */
SAMPLE_T utils_pitch_to_hz(SAMPLE_T midi_pitch) {
    SAMPLE_T hz = 440 * pow(2, (midi_pitch - 69.0) / 12.0);
    // printf("utils_pitch_to_hz(%f) == %f\n", midi_pitch, hz);
    return hz;
}

SAMPLE_T utils_hz_to_pitch(SAMPLE_T hz) {
    SAMPLE_T midi_pitch = 69.0 + 12 * log2(hz / 440.0);
    // printf("utils_hz_to_pitch(%f) == %f\n", hz, midi_pitch);
    return midi_pitch;
}

/**
 * @brief Return -1 if sample is negative, +1 if positive, 0 otherwise.
 */
SAMPLE_T utils_signum(SAMPLE_T sample) {
    if (sample > 0) {
        return 1.0;
    } else if (sample < 0) {
        return -1.0;
    } else {
        return 0.0;
    }
}

// =============================================================================

/*
(gcc -DUTILS_STANDALONE_TEST -Wall -g -o utils_test utils.c \
&& ./utils_test \
&& rm -rf ./utils_test ./utils_test.dSYM)
*/

#ifdef UTILS_STANDALONE_TEST

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#define ASSERT assert

bool approx_eq(float x, float y, float epsilon) {
    return (fabs(x-y) < epsilon);
}

int main(void) {
  printf("Beginning standalone tests...\n");

  ASSERT(approx_eq(utils_db_to_gain(-10), 0.1, 0.001));
  ASSERT(approx_eq(utils_db_to_gain(0), 1.0, 0.001));
  ASSERT(approx_eq(utils_db_to_gain(10), 10.0, 0.001));

  ASSERT(approx_eq(utils_gain_to_db(0.1), -10.0, 0.001));
  ASSERT(approx_eq(utils_gain_to_db(1.0), 0.0, 0.001));
  ASSERT(approx_eq(utils_gain_to_db(10.0), 10.0, 0.001));

  ASSERT(utils_pitch_to_hz(69) == 440.0);
  ASSERT(utils_pitch_to_hz(69+12) == 880.0);
  ASSERT(utils_pitch_to_hz(69-12) == 220.0);

  ASSERT(utils_hz_to_pitch(440.0) == 69);
  ASSERT(utils_hz_to_pitch(880.0) == 69+12);
  ASSERT(utils_hz_to_pitch(220.0) == 69-12);

  ASSERT(utils_signum(100.0) == 1.0);
  ASSERT(utils_signum(0.0) == 0.0);
  ASSERT(utils_signum(-100.0) == -1.0);

  printf("...done\n");
}

#endif
