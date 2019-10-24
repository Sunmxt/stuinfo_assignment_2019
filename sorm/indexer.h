#ifndef SORM_INDEXER_H
#define SORM_INDEXER_H

#include <sqlite3.h>
#include "model.h"

namespace sorm {

    class SQLite3Indexer {
    public:
        static std::string Escape(const char *to_escape);

    public:
        SQLite3Indexer();
        ~SQLite3Indexer();

        int open(const char *dsn);
        int save(Model &m);
        int insert(Model &m);
        int init_model(Model& model) {
            return create_table_according_to_metadata(model);
        }


        int close();

    protected:
        int _update_all(Model &m, const Field* primary);
        int _sqlite_exec(const char *sql);

        int create_table_according_to_metadata(Model *m);
        int create_table_according_to_metadata(Model &m);

    protected:
        sqlite3* db;
    };

}

#endif