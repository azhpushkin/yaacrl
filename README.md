# yaacrl

This is a Shazam-like library, aimed to recognize the audiotracks.
Contains logic for both fingerprinting and storage (via sqlite).

**NOTE**: I use this projects as a playground, so there are 
quirks here and there. For instance, there is neither configuration of Storage
for fingerprints nor any guarantees of persistense about those, as for now.


There are CMake targets for static and shared library.
Both targets perform linking to shared libraries `libdl` and `libpthread` automatically.
