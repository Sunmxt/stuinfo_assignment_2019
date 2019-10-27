#include <string>
#include <cstring>
#include <sqlite3.h>
#include <glog/logging.h>
#include "indexer.h"

using namespace sorm;

SQLite3Indexer::SQLite3Indexer()
:db(0)
{}

SQLite3Indexer::~SQLite3Indexer() {
    if ( SQLITE_OK != close() ) {
        LOG(ERROR) << "cannot close sqlite3 database.";
    }
}

int SQLite3Indexer::open(const char *dsn) {
    if (db) return SQLITE_OK;
    return sqlite3_open(dsn, &db);
}

int SQLite3Indexer::close() {
    if(!db) return SQLITE_OK;
    int ret = sqlite3_close(db);
    if ( SQLITE_OK == ret ) {
        db = 0;
    }
    return ret;
}

int SQLite3Indexer::create_table_according_to_metadata(Model *m) {
    const char *table_name = m -> sorm_table_name();
    char* errmsg;

    std::string sql("create table if not exists `");
    sql += SQLite3Indexer::Escaper()(table_name).c_str();
    sql += "`(";

    const Field *field = m -> sorm_fields();

    for(int i = m -> sorm_num_of_fields(); i ; i--, field++) {
        sql += SQLite3Indexer::Escaper()(field -> meta -> name);

        switch (field -> value_type) {
        case INT_MODEL_FIELD:
            sql += " integer"; break;

        case STL_STRING_FIELD:
            sql += " varchar(128)"; break;

        default:
            LOG(ERROR) << "unsupported model field type: " << (unsigned int)(field -> value_type);
            return SQLITE_ERROR;
        }

        uint32_t props = field -> meta -> props;
        while (props) {
            uint32_t prop = props & (((props - 1) << 1) + 1);
            switch (prop) {
            case Unique:
                sql += " unique"; break;
            case PrimaryKey:
                sql += " primary key"; break;
            case NotNull:
                sql += " not null"; break;
            case AutoIncrement:
                sql += " auto increment"; break;
            }
            props &= ~prop;
        }
        if(i > 1) {
            sql += ", ";
        }
    }
    sql += ");";
    return _sqlite_exec(sql.c_str());
}

int SQLite3Indexer::create_table_according_to_metadata(Model &m) {
    return create_table_according_to_metadata(&m);
}

int SQLite3Indexer::save(Model &m) {
    // locate primary column.
    Field *primary = m.sorm_primary_key_field();
    if (!primary || primary -> is_zero_value()) {
        return insert(m, primary);
    }
    return _update_all(m, primary);
}

int SQLite3Indexer::insert(Model &m, Field *primary) {
    std::string sql("insert into `");
    sql += SQLite3Indexer::Escaper()(m.sorm_table_name()) + "`(";

    const Field *field = m.sorm_fields();
    for(int i = m.sorm_num_of_fields(); i; i--, field++) {
        if (field -> meta -> props & PrimaryKey) continue;

        sql += field -> meta -> name;
        if (i > 1) {
            sql += ",";
        } else {
            sql += ") values (";
        }
    }
    field = m.sorm_fields();
    for(int i = m.sorm_num_of_fields(); i; i--, field++) {
        if (field -> meta -> props & PrimaryKey) continue;

        sql += field -> string(SQLite3Indexer::Escaper());

        if (i > 1) {
            sql += ",";
        } else {
            sql += ")";
        }
    }
    int retval = _sqlite_exec(sql.c_str());
    if (primary) {
        long long int row_id = sqlite3_last_insert_rowid(db);
        primary -> parse(row_id);
    }
    return retval;
}

int SQLite3Indexer::_update_all(Model &m, Field* primary) {
    if (m.sorm_num_of_fields() < 2) {
        return SQLITE_OK;
    }
    std::string sql("update `");
    sql += SQLite3Indexer::Escaper()(m.sorm_table_name()) + "` set `";

    Field *field = m.sorm_fields();
    for(int i = m.sorm_num_of_fields(); i; i--, field++) {
        if (field -> meta -> props & PrimaryKey) {
            continue;
        }
        sql += field -> meta -> name;
        sql += "` = ";
        sql += field -> string(SQLite3Indexer::Escaper());
        if (i > 1) {
            sql += ",`";
        }
    }
    sql += " where `";
    sql += SQLite3Indexer::Escaper()(primary -> meta -> name);
    sql += "` = ";
    sql += primary -> string(SQLite3Indexer::Escaper());
    return _sqlite_exec(sql.c_str());
}   

int SQLite3Indexer::_sqlite_exec(const char *sql, int (*callback)(void*,int,char**,char**), void *arg) {
    char *errmsg = 0;
    LOG(INFO) << "execute sqlite: " << sql;
    int retval = sqlite3_exec(db, sql, callback, arg, &errmsg);
    if (SQLITE_OK != retval) {
        LOG(ERROR) << "sqlite3 error" << retval << " : " << errmsg;
    }
    if (errmsg) {
        sqlite3_free(errmsg);
    }
    return retval;
}