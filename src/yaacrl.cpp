#include <string>
#include <map>
#include <iostream>

#include "vendor/sqlite3.h"

#include "fingerprint.h"
#include "spectrogram.h"
#include "yaacrl.h"


using namespace yaacrl;


/* I bet you have not expected to see ASCII cat over here:

 ,_     _
 |\\_,-~/
 / _  _ |    ,--.
(  @  @ )   / ,-'
 \  _T_/-._( (
 /         `. \
|         _  \ |
 \ \ ,  /      |
  || |-_\__   /
 ((_/`(____,-'
*/



#define CONFIDENCE 2  // at least 2 simult for a match
#define DB_CON (sqlite3*)db
#define LOG_ERR(msg) { yaacrl_log_message(LogLevel::ERROR, std::string(msg) + std::string(sqlite3_errmsg(DB_CON))); }
#define BUILD_FINGERPRINT_FROM_FILE(ClassName) \
    Fingerprint::Fingerprint(const ClassName& file) { \
        this->process(file.samples); \
        yaacrl_log_message(LogLevel::INFO, std::string("Successfully processed ") + file.path); \
    }



BUILD_FINGERPRINT_FROM_FILE(MP3File)
BUILD_FINGERPRINT_FROM_FILE(WAVFile)


void Fingerprint::process(ChannelSamples samples) {
    for (int i = 0; i < samples.size(); i++) {
        std::cout << "found " << samples[i].size() << " samples" << std::endl;
        auto spec = gen_spectrogram(samples[i]);
        auto peaks = find_peaks(spec);
        auto hashes = generate_hashes(peaks);

        this->peaks.reserve(this->peaks.size() + distance(peaks.begin(), peaks.end()));
        this->peaks.insert(this->peaks.end(), peaks.begin(), peaks.end());

        this->hashes.reserve(this->hashes.size() + distance(hashes.begin(), hashes.end()));
        this->hashes.insert(this->hashes.end(), hashes.begin(), hashes.end());
    }
}


Storage::Storage(std::string file) {
    int rc;
    char *zErrMsg;
    std::string sql;
    
    rc = sqlite3_open(file.c_str(), (sqlite3**)(&db));
    if ( rc ) {
        yaacrl_log_message(LogLevel::ERROR, std::string("Error creating SQLite database: ") + std::string(sqlite3_errmsg(DB_CON)));
    }

    sql = R"(
        create table if not exists songs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL
        )
    )";
    rc = sqlite3_exec(DB_CON, sql.c_str(), NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        yaacrl_log_message(LogLevel::ERROR, std::string("Error creating songs table"));
    }

    sql = R"(
        create table if not exists fingerprints (
            hash BLOB NOT NULL,
            offset INT NOT NULL,
            song_id INT NOT NULL,
            FOREIGN KEY(song_id) REFERENCES songs(id)
        )
    )";
    rc = sqlite3_exec(DB_CON, sql.c_str(), NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        yaacrl_log_message(LogLevel::ERROR, std::string("Error creating fingerprints table"));
    }

    sql = R"(
        create index if not exists hash_index on fingerprints(hash);
    )";
    rc = sqlite3_exec(DB_CON, sql.c_str(), NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ) {
        yaacrl_log_message(LogLevel::ERROR, std::string("Error creating index"));
    }

    yaacrl_log_message(LogLevel::INFO, std::string("SQLite database setup completed successfully"));
}

Storage::~Storage() {
    sqlite3_close(DB_CON);
}

void Storage::store_fingerprint(Fingerprint&& fp, std::string name) {
    store_fingerprint(fp, name);
}

void Storage::store_fingerprint(Fingerprint& fp, std::string name) {
    int rc;

    // Perform all inserts in a transaction
    rc = sqlite3_exec(DB_CON, "BEGIN TRANSACTION", NULL, NULL, NULL);
    if (rc != SQLITE_OK)
        LOG_ERR("Cannot begin transaction: ")

    // INSERT SONG
    sqlite3_stmt *stmt = NULL;
    rc = sqlite3_prepare(
        DB_CON,
        "INSERT INTO songs (name) VALUES(?)",
        -1, &stmt, NULL
    );
    if (rc != SQLITE_OK)
        LOG_ERR("Error preparing song insert: ")
    
    sqlite3_bind_text(stmt, 1, name.c_str(), name.size(), SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERR("Bad bind to insert: ");
    }
    sqlite3_finalize(stmt);
    
    
    stmt = NULL;
    rc = sqlite3_prepare(
        DB_CON, "select id from songs where name = ?",
        -1, &stmt, NULL
    );
    sqlite3_bind_text(stmt, 1, name.c_str(), name.size(), SQLITE_STATIC);
    if (rc != SQLITE_OK)
        LOG_ERR("Error selecting song id: ")
    
    sqlite3_step(stmt);
    int song_id = sqlite3_column_int(stmt, 0);
    std::cout << "Song ID is " << song_id << std::endl;
    sqlite3_finalize(stmt);
    
    
    
    // INSERT FINGERPRINTS
    stmt = NULL;
    rc = sqlite3_prepare(
        DB_CON,
        "INSERT INTO fingerprints (hash, offset, song_id) VALUES(?, ?, ?)",
        -1, &stmt, NULL
    );
    if (rc != SQLITE_OK)
        LOG_ERR("Error preparing insert: ")
    
    for (auto const& hash:  fp.hashes) {
        sqlite3_bind_blob(stmt, 1, hash.hash_data, sizeof(hash.hash_data), SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, hash.offset);
        sqlite3_bind_int(stmt, 3, song_id);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            LOG_ERR("Bad bind to insert: ");
        }
        
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);

    rc = sqlite3_exec(DB_CON, "END TRANSACTION", NULL, NULL, NULL);
    if (rc != SQLITE_OK)
        LOG_ERR("Error commiting transaction: ")

    yaacrl_log_message(LogLevel::INFO, std::string("Successfully fingerprinted") + name);
}


std::map<std::string, float> Storage::get_matches(Fingerprint& fp) {
    int rc;

    std::string sql = R"(
        create table if not exists ttemp (
            hash BLOB NOT NULL,
            offset INT NOT NULL
        );
    )";
    rc = sqlite3_exec(DB_CON, sql.c_str(), NULL, NULL, NULL);
    if( rc != SQLITE_OK )
        LOG_ERR("Error creating temporary table: ")

    sqlite3_stmt *stmt = NULL;
    rc = sqlite3_prepare(
        DB_CON,
        "INSERT INTO ttemp (hash, offset) VALUES(?, ?)",
        -1, &stmt, NULL
    );
    if (rc != SQLITE_OK)
        LOG_ERR("Error preparing insert: ")
    
    rc = sqlite3_exec(DB_CON, "BEGIN TRANSACTION", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERR("Error starting transaction: ")
    }

    for (auto const& hash:  fp.hashes) {
        sqlite3_bind_blob(stmt, 1, hash.hash_data, sizeof(hash.hash_data), SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, hash.offset);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Insert into temp failed: " << sqlite3_errmsg(DB_CON) << std::endl;
        }   
            
        sqlite3_reset(stmt);

    }
    rc = sqlite3_exec(DB_CON, "END TRANSACTION", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERR("Error committing transaction: ")
    }
    sqlite3_finalize(stmt);


    rc = sqlite3_prepare(
        DB_CON,
        R"(
             with matches as (
                select
                    fp.song_id as song,
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
                order by total desc
				limit 10
            )
            select cast(song as text), sum(total) as total_matches
            from grouped_matches
            group by song
			order by total_matches desc    
        )",
        -1, &stmt, NULL
    );
    if (rc != SQLITE_OK)
        LOG_ERR("Error selecting matches: ")

    
    std::map<std::string, float> res;
    while ( sqlite3_step(stmt) == SQLITE_ROW) {
        std::string song((const char *)sqlite3_column_text(stmt, 0));
        res[song] = ((float)sqlite3_column_int(stmt, 1) / (float)fp.hashes.size());
    }
    sqlite3_finalize(stmt);

    rc = sqlite3_exec(DB_CON, "drop table ttemp", NULL, NULL, NULL);
    if( rc != SQLITE_OK ) {
        LOG_ERR("Error dropping temp:");
    }
    yaacrl_log_message(LogLevel::INFO, std::string("Matches lookup processed"));

    return res;
}
