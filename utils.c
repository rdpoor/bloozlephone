#include "utils.h"

#ifndef SAMPLE_T
#define SAMPLE_T float
#endif

// pow(10, dbGain / 40);
SAMPLE_T utils_db_to_gain(SAMPLE_T db) {
    return 0.0;
}

SAMPLE_T utils_gain_to_db(SAMPLE_T gain) {
    return 0.0;
}

/**
 * @brief convert midi-style pitch to frequency
 * MIDI pitch 69 = 440Hz.
 * semitones = pitch - 69
 * return 440 * pow(2, semitones / 12)
 */
SAMPLE_T utils_pitch_to_hz(SAMPLE_T midi_pitch) {
    return 0.0;
}

SAMPLE_T utils_hz_to_pitch(SAMPLE_T hz) {
    return 0.0;
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
