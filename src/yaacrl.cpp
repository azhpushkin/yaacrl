#include <string>
#include <map>
#include <iostream>

#include "vendor/sqlite3.h"
#include "vendor/AudioFile.h"

#include "fingerprint.h"
#include "spectrogram.h"
#include "yaacrl.h"

#define CONFIDENCE 2  // at least 2 simult for a match
#define DB_CON (sqlite3*)db
#define PRINT_CERR_AND_ABORT(msg) { std::cerr << msg << sqlite3_errmsg(DB_CON) << std::endl; std::abort(); }

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

Storage::Storage(std::string file) {
    int rc;
    char *zErrMsg;
    std::string sql;
    
    rc = sqlite3_open(file.c_str(), (sqlite3**)(&db));
    if ( rc ) {
        std::cerr << "Error creating SQLite database: " << sqlite3_errmsg(DB_CON) << std::endl;
        std::abort();
    }

    sql = R"(
        create table if not exists fingerprints (
            hash BLOB NOT NULL,
            offset INT NOT NULL,
            song TEXT NOT NULL
        )
    )";
    rc = sqlite3_exec(DB_CON, sql.c_str(), NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        std::cerr << "Error: " << zErrMsg << std::endl;
    }

    sql = R"(
        create index if not exists hash_index on fingerprints(hash);
    )";
    rc = sqlite3_exec(DB_CON, sql.c_str(), NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ) {
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

    // Perform all inserts in a transaction
    rc = sqlite3_exec(DB_CON, "BEGIN TRANSACTION", NULL, NULL, NULL);
    if (rc != SQLITE_OK)
        PRINT_CERR_AND_ABORT("Cannot begin transaction: ")


    sqlite3_stmt *stmt = NULL;
    rc = sqlite3_prepare(
        DB_CON,
        "INSERT INTO fingerprints (hash, offset, song) VALUES(?, ?, ?)",
        -1, &stmt, NULL
    );
    if (rc != SQLITE_OK)
        PRINT_CERR_AND_ABORT("Error preparing insert: ")
    
    for (auto const& hash:  fp.hashes) {
        sqlite3_bind_blob(stmt, 1, hash.hash.data(), HASH_SIZE, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, hash.offset);
        sqlite3_bind_text(stmt, 3, fp.name.c_str(), fp.name.size(), SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Bad bind to insert: " << sqlite3_errmsg(DB_CON) << std::endl;
        }
        
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);

    rc = sqlite3_exec(DB_CON, "END TRANSACTION", NULL, NULL, NULL);
    if (rc != SQLITE_OK)
        PRINT_CERR_AND_ABORT("Error commiting transaction: ")
}


std::map<std::string, float> Storage::get_matches(Fingerprint& fp) {
    int rc;

    std::string sql = R"(
        create table if not exists ttemp (
            hash BLOB NOT NULL,
            offset INT NOT NULL,
            song TEXT NOT NULL
        );
    )";
    rc = sqlite3_exec(DB_CON, sql.c_str(), NULL, NULL, NULL);
    if( rc != SQLITE_OK )
        PRINT_CERR_AND_ABORT("Error creating temporary table: ")

    sqlite3_stmt *stmt = NULL;
    rc = sqlite3_prepare(
        DB_CON,
        "INSERT INTO ttemp (hash, offset, song) VALUES(?, ?, ?)",
        -1, &stmt, NULL
    );
    if (rc != SQLITE_OK)
        PRINT_CERR_AND_ABORT("Error preparing insert: ")
    
    rc = sqlite3_exec(DB_CON, "BEGIN TRANSACTION", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        PRINT_CERR_AND_ABORT("Error starting transaction: ")
    }

    for (auto const& hash:  fp.hashes) {
        sqlite3_bind_blob(stmt, 1, hash.hash.data(), HASH_SIZE, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, hash.offset);
        sqlite3_bind_text(stmt, 3, fp.name.c_str(), fp.name.size(), SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Insert into temp failed: " << sqlite3_errmsg(DB_CON) << std::endl;
        }   
            
        sqlite3_reset(stmt);

    }
    rc = sqlite3_exec(DB_CON, "END TRANSACTION", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        PRINT_CERR_AND_ABORT("Error committing transaction: ")
    }
    sqlite3_finalize(stmt);


    rc = sqlite3_prepare(
        DB_CON,
        R"(
             with matches as (
                select
                    fp.song as song,
                    (fp.offset - ttemp.offset) as offset_diff
                from fingerprints fp
                join ttemp on ttemp.hash = fp.hash
                where offset_diff > 0
            ),
            grouped_matches as (
                select
                    song,
                    offset_diff,
                    count(*) as total
                from matches
                group by song, offset_diff
                having total >= 2
            )
            select song, total * 1.0 / (select count(*) from matches) from grouped_matches          
        )",
        -1, &stmt, NULL
    );
    if (rc != SQLITE_OK)
        PRINT_CERR_AND_ABORT("Error selecting matches: ")

    
    std::map<std::string, float> res;
    while ( sqlite3_step(stmt) == SQLITE_ROW) {
        std::string song((const char *)sqlite3_column_text(stmt, 0));
        res[song] = (float)sqlite3_column_double(stmt, 1);
    }
    sqlite3_finalize(stmt);

    rc = sqlite3_exec(DB_CON, "drop table ttemp", NULL, NULL, NULL);
    if( rc != SQLITE_OK ) {
        std::cerr << "Error dropping temp: " << sqlite3_errmsg(DB_CON) << std::endl;
    }
    return res;
}
