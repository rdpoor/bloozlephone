/** @file sinosc.c
    @brief Implementation of a sinosc filter
    @author R.D.Poor  rdpoor@gmail.com
*/


#include "sinosc.h"
#include <math.h>
#include <stddef.h>

// =============================================================================

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =============================================================================

sinosc_t *sinosc_init(sinosc_t *s, SAMPLE_T srate) {
    s->omega = 2.0 * M_PI / srate;
    s->theta = 0.0;
    s->prev = 0.0;
    return s;
}

SAMPLE_T sinosc_step(sinosc_t *s, SAMPLE_T frequency_hz, SAMPLE_T amplitude) {
    s->prev = amplitude * sin(s->theta);
    s->theta += frequency_hz * s->omega;
    return s->prev;
}

SAMPLE_T sinosc_prev(sinosc_t *s) {
    return s->prev;
}
