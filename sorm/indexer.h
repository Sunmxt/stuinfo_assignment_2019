#ifndef SORM_INDEXER_H
#define SORM_INDEXER_H

#include <sqlite3.h>
#include "model.h"
#include "query.h"

namespace sorm {

    class SQLite3Indexer {
    public:
        static std::string Escape(const char* s);
        static std::string Escape(std::string& s);

        class Escaper {
        public:
            // 防注入以后再说
            std::string operator() (std::string s) {
                return s;
            }
            std::string operator() (const char *s) {
                return std::string(s);
            }
        };

    public:
        SQLite3Indexer();
        ~SQLite3Indexer();

        int open(const char *dsn);
        int save(Model &m);
        int insert(Model &m, Field* primary = 0);
        int init_model(Model&& model) {
            return create_table_according_to_metadata(model);
        }

        template<class _cont, class _mod = void> class reverse_mapper {
        public:
            static const int valid_container = 0;
        };

        template<class _mod> class reverse_mapper<std::vector<_mod>, _mod> {
        public:
            static const int valid_container = 1;
            typedef reverse_mapper<std::vector<_mod>, _mod> self_type;

            static int invoke(void *arg, int ncol, char** vs, char** cols) {
                if ( !cols || !vs || ncol < 1) return SQLITE_OK;
                self_type *mapper = (self_type*)(arg);

                mapper -> container -> push_back(_mod());
                _mod &elem =  mapper -> container -> back();

                for(int n = 0; n < ncol; n++) {
                    char* col = cols[n];
                    char* v = vs[n];

                    LOG_IF(WARNING, !col) << "sqlite3 mapper got null column name.";
                    if (!col || !v) continue;

                    Field *field = elem.sorm_field_by_name(col);
                    if (!field) {
                        LOG(ERROR) << "cannot find column \"" << col << "\" for reverse mapping.";
                        return SQLITE_ERROR;
                    }
                    field -> parse(v);
                }

                return SQLITE_OK;
            }

            reverse_mapper(std::vector<_mod> *_container)
            :container(_container) {}

            void *get_args() const { return (void*)this; }

            std::vector<_mod> *container;
        };


        template<class _q, class _mod> int exec(_q&& query, std::vector<_mod>& container) {
            typedef reverse_mapper<std::vector<_mod>, _mod> mapper_type;
            static_assert(mapper_type::valid_container, "invalid object container type.");

            std::string sql(query.template expr<DefaultGenerator<Escaper>>());
            mapper_type &&mapper = mapper_type(&container);
            return _sqlite_exec(sql.c_str(), mapper_type::invoke, mapper.get_args());
        }

        int close();

    protected:
        int _update_all(Model &m, Field* primary);
        int _sqlite_exec(const char *sql, int (*callback)(void*,int,char**,char**) = 0, void *arg = 0);

        int create_table_according_to_metadata(Model *m);
        int create_table_according_to_metadata(Model &m);

    protected:
        sqlite3* db;
    };

}

#endif