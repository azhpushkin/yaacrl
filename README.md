# yaacrl

This is a Shazam-like library, aimed to recognize the audiotracks.
It contains logic that both performs audio fingerprinting
and storage / 

**NOTE**: I use this projects as a playground, so there are 
quirks here and there. For instance, there is neither configuration of Storage
for fingerprints nor any guarantees of persistense about those, as for now.


There are CMake targets for static and shared library.
Both targets perform linking to shared libraries `libdl` and `libpthread` automatically.

TODOs:
- [x] add mp3 support
- [x] merge test programs to single one, support both mp3 and wav
- [ ] abstract class for MP3 and WAV
- [ ] make configurable TOP amount
- [ ] make Storage thread-safe
- [ ] make (e.g. drop ttemp table)
- [ ] separate storage.cpp out of yaacrl.cpp

- [ ] optional name of the song on import (gui - cut only filename)
- [ ] config of fingerprinting 
- [ ] test audios with different frequencies, find a way to make them work
- [ ] test and improve performance
- [ ] check how MP3File and WAVFile work when wrong file format passed

