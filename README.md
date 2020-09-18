# yaacrl

This is a Shazam-like library, aimed to recognize the audiotracks.
Contains logic for both fingerprinting and PostgreSQL storage.



Run with:
```
g++ main.cpp lib.cpp\
    spectrogram.cpp fingerprint.cpp\
    MurmurHash3.cpp kissfft/kiss_fft.c\
    -lhiredis -o main.out && ./main.out
```