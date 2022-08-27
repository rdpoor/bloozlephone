#ifndef _UTILS_H_
#define _UTILS_H_

#ifndef SAMPLE_T
#define SAMPLE_T float
#endif

// pow(10, dbGain / 40);
SAMPLE_T utils_db_to_gain(SAMPLE_T db);

SAMPLE_T utils_gain_to_db(SAMPLE_T gain);

/**
 * @brief convert midi-style pitch to frequency
 * MIDI pitch 69 = 440Hz.
 * semitones = pitch - 69
 * return 440 * pow(2, semitones / 12)
 */
SAMPLE_T utils_pitch_to_hz(SAMPLE_T midi_pitch);

SAMPLE_T utils_hz_to_pitch(SAMPLE_T hz);

/**
 * @brief Return -1 if sample is negative, +1 if positive, 0 otherwise.
 */
SAMPLE_T utils_signum(SAMPLE_T sample);

#endif // #define _UTILS_H_
