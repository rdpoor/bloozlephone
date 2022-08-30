#ifndef _SINOSC_H_
#define _SINOSC_H_

#ifndef SAMPLE_T
#define SAMPLE_T float
#endif

typedef struct {
    SAMPLE_T amplitude;
    SAMPLE_T theta;   // updated at every sample
    SAMPLE_T omega;   // 2.0 * M_PI / srate
    SAMPLE_T prev;    // previously computed sample
} sinosc_t;

sinosc_t *sinosc_init(sinosc_t *s, SAMPLE_T srate);

SAMPLE_T sinosc_step(sinosc_t *s, SAMPLE_T frequency_hz, SAMPLE_T amplitude);

SAMPLE_T sinosc_prev(sinosc_t *s);

#endif // #define _SINOSC_H_
