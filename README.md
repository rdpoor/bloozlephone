# bloozlephone
A physio-digital musical instrument custom made for a bloozle

## Developer Notes

Using Adafruit Voice Bonnet giving good results: pass through from mic to
speaker output using Python pyaudio package gives decent low-latency.  But
to do any real real-time processing, I'll amost certainly have to drop down
to pure C level, for which I should use PortAudio (c.f.).  


## Threads to follow

## PortAudio
http://www.portaudio.com/docs/v19-doxydocs/compile_linux.html

Pitch detection:

Yin: http://mroy.chez-alice.fr/yin/index.html
http://recherche.ircam.fr/equipes/pcm/cheveign/pss/2002_JASA_YIN.pdf

AMDF:
https://www.instructables.com/Arduino-Pitch-Detection-Algorithm-AMDF/


Arduino Frequency Detection:
interface.khm.de/index.php/lab/experiments/arduino-frequency-counter-library/

Brute force Autocorrelation with peak detection:
https://www.instructables.com/Reliable-Frequency-Detection-Using-DSP-Techniques/
