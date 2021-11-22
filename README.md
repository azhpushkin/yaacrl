## yaacrl

This is a Shazam-like library, aimed to recognize the audiotracks.
It contains logic that both performs audio fingerprinting and storing+matching of
fingerprints.

**NOTE**: I use this projects as a playground, so there are 
quirks here and there. For instance, there is neither configuration of Storage
for fingerprints nor any guarantees about Storage format and backwards compatibility.

However, if something is not working for you - don't hesitate to open an issue
or contact me in any other way. I'm always up to a little chat.

## How this works?

This project was inspired by 
[this excellent article](https://willdrevo.com/fingerprinting-and-audio-recognition-with-python/)
by Will Drevo. No way I'm explaining how this works better than he does.
There are certainly subtle differences in the algorithms but the general idea is pretty much
the same in both `dejavu` and `yaacrl`.

Fingerprint are stored in the SQLite file, which might get pretty big for large libraries.
I've got up to 800Mb for 100 fingerprinted songs, and half of the database was spent on
fingerprints index.


## Demo

If you want to play and test this library out,
you might be as well interested in two related projects:
* [pyaacrl](https://github.com/azhpushkin/pyaacrl) - 
    Python wrapper for this library. To try it out you don't even need to clone anything, just use

    `pip install git+https://github.com/azhpushkin/pyaacrl/`
* [pyaacrl-demo](https://github.com/azhpushkin/pyaacrl-demo) - GUI for pyaacrl, powered by
    PySide2 (Qt5) and few other libraries. This one is even simpler to use, because you
    can record an audio from the mic with it. Go try it out!

If, for some reason, you are still up to messing with C++ and CMake - 
there is a `yaacrl-test` target created exactly for this purpose.
This target compiles to an executable capable of both fingerprinting and matching a fragment.

Note that only MP3 and WAV are supported now, and I've only tested 44100 sampling frequency
(44100 is the most popular one so this is fine for now I guess).

Creating a library of fingerprints out of folder with audio:
```bash
# library.db will be created if not exists
$ ./yaacrl-test fingerprint library.sqlite /path/to/folder/with/songs
[INFO] SQLite database setup completed successfully
Starting..
[DEBUG] Loaded WAVFile: /path/to/folder/with/songs/song-1.wav
[INFO] Fingeprints generated for /path/to/folder/with/songs/song-1.wav
[INFO] Successfully stored /path/to/folder/with/songs/song-1.wav
[DEBUG] Loaded MP3File: /path/to/folder/with/songs/song-2.mp3
[INFO] Fingeprints generated for /path/to/folder/with/songs/song-2.mp3
[INFO] Successfully stored /path/to/folder/with/songs/song-2.mp3
Total songs in DB now: 4
```

Matching an audio clip
```bash
$ ./yaacrl-test match library.sqlite /path/to/audio.mp3
[INFO] SQLite database setup completed successfully
Check "lol.mp3"
[DEBUG] Loaded MP3File: /path/to/audio.mp3
[INFO] Fingeprints generated for /home/maqquettex/lol.mp3
[INFO] Matches lookup processed
  ->  Match results: 
    * 006 song-2.mp3: 20.8%
```

You can also manually browse a database file just as any other SQLite database.
Here is the schema:
```sql
CREATE TABLE songs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL
);

CREATE TABLE fingerprints (
    hash BLOB NOT NULL,
    offset INT NOT NULL,
    song_id INT NOT NULL,
    FOREIGN KEY(song_id) REFERENCES songs(id)
);

CREATE INDEX hash_index on fingerprints(hash);
```





### TODOs to look into
- [x] add mp3 support
- [x] merge test programs to single one, support both mp3 and wav
- [ ] implement multiple storages (add postgresql) - not sure though
- [ ] abstract class for MP3 and WAV
- [ ] improve recognition, avoid using temporary table for matching
- [ ] make storage thread-safe (currently it is not)
- [ ] add possibility to configure yaacrl (probably macroses ??)
- [ ] test how audio with different sample frequencies work together (probably need to fix this)
- [ ] test and improve performance
- [ ] check how MP3File and WAVFile work when wrong file format passed

