# yaacrl

This is a Shazam-like library, aimed to recognize the audiotracks.
Contains logic for both fingerprinting and PostgreSQL storage.



Run with:
```
g++ main.cpp spectrogram.cpp kissfft/kiss_fft.c -o main.out && ./main.out
```