#include <string>
#include <map>
#include <utility>
#include <iostream>
#include <cstring>
#include <algorithm>

#include "vendor/sqlite3.h"
#include "vendor/AudioFile.h"

#include "fingerprint.h"
#include "spectrogram.h"
#include "yaacrl.h"

#include "vendor/fmt.h"

#define CONFIDENCE 2  // at least 2 simult for a match
#define DB_CON (sqlite3*)db

using namespace yaacrl;

WAVFile::WAVFile(std::string path_): path(path_), name(path_) {}
WAVFile::WAVFile(std::string path_, std::string name_): path(path_), name(name_) {}

MP3File::MP3File(std::string path_): path(path_), name(path_) {}
MP3File::MP3File(std::string path_, std::string name_): path(path_), name(name_) {}


Fingerprint::Fingerprint(const WAVFile& file) {
    this->name = file.name;
    this->process(file.path);
}

// TODO: throw exception for mp3 file
Fingerprint::Fingerprint(const MP3File& file) {
    this->name = file.name;
    this->process(file.path);
}

void Fingerprint::process(std::string path) {
    AudioFile<float> audioFile;

    audioFile.load (path);

    if (1 != audioFile.getNumChannels()) {
        // TODO: change to logging
        std::cerr << "Detected number of channels is not supported!" << std::endl;
        std::abort();
    }
    
    this->spec = gen_spectrogram(audioFile.samples[0]);
    this->peaks = find_peaks(this->spec);
    this->hashes = generate_hashes(this->peaks);
}

Storage::Storage() {
    int rc;
    char *zErrMsg;
    std::string sql;
    
    rc = sqlite3_open("file.sqlite", (sqlite3**)(&db));
    if ( rc ) {
        std::cerr << "Error creating SQLite database: " << sqlite3_errmsg(DB_CON) << std::endl;
        std::abort();
    }

    sql = R"(
        create table fingerprints (
            hash BLOB NOT NULL,
            offset INT NOT NULL,
            song TEXT NOT NULL
        );
    )";
    rc = sqlite3_exec(DB_CON, sql.c_str(), NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        std::cerr << "Error: " << zErrMsg << std::endl;
    }

    sql = R"(
        create index hash_index on fingerprints(hash);
    )";
    rc = sqlite3_exec(DB_CON, sql.c_str(), NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        std::cerr << "Error: " << zErrMsg << std::endl;
    }
}

Storage::~Storage() {
    sqlite3_close(DB_CON);
}

void Storage::store_fingerprint(Fingerprint&& fp) {
    store_fingerprint(fp);
}

void Storage::store_fingerprint(Fingerprint& fp) {
    int rc;
    char* errMsg;

    sqlite3_exec(DB_CON, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    sqlite3_stmt *stmt = NULL;
    rc = sqlite3_prepare(
        DB_CON,
        "INSERT INTO fingerprints (hash, offset, song) VALUES(?, ?, ?)",
        -1, &stmt, NULL
    );
    if (rc != SQLITE_OK) {
        std::cerr << "prepare failed: " << sqlite3_errmsg(DB_CON) << std::endl;
    }
    int count = 0;
    for (auto const& hash:  fp.hashes) {
        sqlite3_bind_blob(stmt, 1, HASH_DATA(hash), HASH_SIZE, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, HASH_OFFSET(hash));
        sqlite3_bind_text(stmt, 3, fp.name.c_str(), fp.name.size(), SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
            std::cerr << "execution failed: " << sqlite3_errmsg(DB_CON) << std::endl;
        
        sqlite3_reset(stmt);

        count++;

        // auto sql_str = fmt::format(
        //     "insert into fingerprints (hash, offset, song) values ({}, {}, '{}')",
        //     HASH_DATA(hash), HASH_OFFSET(hash), fp.name
        // );

        // rc = sqlite3_exec(DB_CON, sql_str.c_str(), NULL, 0, &errMsg);
        // if( rc != SQLITE_OK ){
        //     std::cerr << "Error: " << errMsg << std::endl;
        // }
    }
    sqlite3_finalize(stmt);

    rc = sqlite3_exec(DB_CON, "END TRANSACTION;", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "commit failed: " << sqlite3_errmsg(DB_CON) << std::endl;
    }

}


// Probably should not be here, IDK
class Match {
public:
    std::string song_name;
    int offset;
};


std::map<std::string, float> Storage::get_matches(Fingerprint& fp) {
    char* errMsg;
    int rc;

    std::string sql = R"(
        create table if not exists ttemp (
            hash BLOB NOT NULL,
            offset INT NOT NULL,
            song TEXT NOT NULL
        );
    )";
    rc = sqlite3_exec(DB_CON, sql.c_str(), NULL, 0, &errMsg);
    if( rc != SQLITE_OK ){
        std::cerr << "Error: " << errMsg << std::endl;
    }

    sqlite3_stmt *stmt = NULL;
    rc = sqlite3_prepare(
        DB_CON,
        "INSERT INTO ttemp (hash, offset, song) VALUES(?, ?, ?)",
        -1, &stmt, NULL
    );
    if (rc != SQLITE_OK) {
        std::cerr << "prepare failed: " << sqlite3_errmsg(DB_CON) << std::endl;
    }
    
    sqlite3_exec(DB_CON, "BEGIN TRANSACTION", NULL, NULL, NULL);

    for (auto const& hash:  fp.hashes) {
        sqlite3_bind_blob(stmt, 1, HASH_DATA(hash), HASH_SIZE, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, HASH_OFFSET(hash));
        sqlite3_bind_text(stmt, 3, fp.name.c_str(), fp.name.size(), SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
            std::cerr << "execution failed: " << sqlite3_errmsg(DB_CON) << std::endl;
        sqlite3_reset(stmt);

    }
    sqlite3_exec(DB_CON, "END TRANSACTION", NULL, NULL, NULL);
    sqlite3_finalize(stmt);



    rc = sqlite3_prepare(
        DB_CON,
        R"(
            select
                fp.song,
                (fp.offset - ttemp.offset) as offset_diff
            from fingerprints fp
            join ttemp on ttemp.hash = fp.hash
        )",
        -1, &stmt, NULL
    );
    if (rc != SQLITE_OK)
        std::cerr << "1231232113execution failed: " << sqlite3_errmsg(DB_CON) << std::endl;

    std::vector<Match> matches;
    while ( sqlite3_step(stmt) == SQLITE_ROW) {
        Match new_match;
        new_match.song_name = std::string((const char *)sqlite3_column_text(stmt, 0));
        new_match.offset = sqlite3_column_int(stmt, 1);
        matches.push_back(new_match);
    }
    sqlite3_finalize(stmt);

    rc = sqlite3_exec(DB_CON, "drop table ttemp", NULL, 0, &errMsg);
    if( rc != SQLITE_OK ){
        std::cerr << "Error dropping database: " << errMsg << std::endl;
    }
    
    std::map<std::pair<int, std::string>, int> grouped_matches;
    for(auto& match: matches) {
        std::pair<int, std::string> pair = std::make_pair(match.offset, match.song_name);

        grouped_matches[pair]++;
    }

    std::map<std::string, int> songs_matches;
    for (auto& pair: grouped_matches) {
        auto amount = std::get<1>(pair);
        auto song = std::get<1>( std::get<0>(pair) );
        if (amount < CONFIDENCE) {
            continue; 
        }
        
        songs_matches[song] += amount;
    }

    std::map<std::string, float> res;
    for(auto& pair: songs_matches) {
        res[std::get<0>(pair)] = std::get<1>(pair) / (float) matches.size();
    }

    return res;
}
