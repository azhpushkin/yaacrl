# yaacrl

This is a Shazam-like library, aimed to recognize the audiotracks.
Contains logic for both fingerprinting and storage (via sqlite).

**NOTE**: I use this projects as a playground, so there are 
quirks here and there. For instance, there is neither configuration of Storage
for fingerprints nor any guarantees of persistense about those, as for now.


There are CMake targets for static and shared library.
Both targets perform linking to shared libraries `libdl` and `libpthread` automatically.

TODOs:
- [ ] make configurable TOP amount
- [ ] make Storage thread-safe
- [ ] make recognition more robust (e.g. drop ttemp table)
- [ ] separate storage.cpp out of yaacrl.cpp
- [x] add mp3 support
- [ ] optional name of the song on import (gui - cut only filename)
- [ ] config of fingerprinting 
- [ ] test audios with different frequencies, find a way to make them work
- [ ] check disabled code
- [ ] test performance (there are some improvements for sure)
- [ ] Add to readme commands to test this out
- [ ] check licensing :(
- [ ] check how MP3File and WAVFile work when wrong file format passed

