// pll_simple_example.c : simulation of a phase-locked loop in 50 lines
// [update] 14 Jan. 2017: updated loop filter to be consistent with literature
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>

int main() {
    // parameters and simulation options
    float        phase_in      =  3.0f;    // carrier phase offset
    float        frequency_in  = -0.20;    // carrier frequency offset
    float        alpha         =  0.05f;   // phase adjustment factor
    // amplitude is included to show that PLL is independent of input level
    float        amplitude     =  0.5f;    // amplitude of input signal
    unsigned int n             =  400;     // number of samples

    // initialize states
    float beta          = 0.5*alpha*alpha; // frequency adjustment factor
    float phase_out     = 0.0f;            // output signal phase
    float frequency_out = 0.0f;            // output signal frequency

    // print line legend to standard output
    printf("%s, %s, %s, %s, %s, %s, %s, %s\n",
           "index", "real(in)", "imag(in)", "real(out)", "imag(out)", "error", "f_in", "f_out");

    // run basic simulation
    int i;
    for (i=0; i<n; i++) {
        // compute input and output signals
        float complex signal_in  = cexpf(_Complex_I * phase_in) * amplitude;
        float complex signal_out = cexpf(_Complex_I * phase_out);

        // compute phase error estimate
        float phase_error = cargf( signal_in * conjf(signal_out) );

        // print results to standard output for plotting
        printf("%u, %12.8f, %12.8f, %12.8f, %12.8f, %12.8f, %12.8f, %12.8f\n",
                  i,
                  crealf(signal_in),  cimagf(signal_in),
                  crealf(signal_out), cimagf(signal_out),
                  phase_error,
                  frequency_in,
                  frequency_out
              );

        // apply loop filter and correct output phase and frequency
        phase_out     += alpha * phase_error;    // adjust phase
        frequency_out +=  beta * phase_error;    // adjust frequency

        // increment input and output phase values
        phase_in  += frequency_in;
        phase_out += frequency_out;
    }
    return 0;
}
