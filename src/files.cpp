#include <iostream>
#include <vector>

#define MINIMP3_IMPLEMENTATION
#define MINIMP3_FLOAT_OUTPUT
#include "vendor/minimp3_ex.h"
#include "vendor/AudioFile.h"

#include "yaacrl.h"


using namespace yaacrl;


WAVFile::WAVFile(std::string path_): path(path_) {
    AudioFile<float> audioFile;

    audioFile.load (path);
    this->samples = std::move(audioFile.samples);

    yaacrl_log_message(LogLevel::DEBUG, std::string("Loaded WAVFile: ") + path);
}

MP3File::MP3File(std::string path_): path(path_) {
    mp3dec_t mp3d;
    mp3dec_file_info_t info;

    if (mp3dec_load(&mp3d, path.c_str(), &info, NULL, NULL))
    {
        // TODO: this looks bad
        yaacrl_log_message(LogLevel::ERROR, std::string("Opening MP3File is not supported yet!"));
        throw std::runtime_error("MP3File is not implemented yet!");
    }

    this->samples.resize(info.channels);

    for (auto i = 0; i < info.samples; ) {
        for (int j = 0; j < info.channels; j++) {
            this->samples[j].emplace_back(info.buffer[i++]);
        }
    }

    yaacrl_log_message(LogLevel::DEBUG, std::string("Loaded MP3File: ") + path);
}