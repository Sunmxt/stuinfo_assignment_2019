#ifndef SORM_QUERY_H
#define SORM_QUERY_H

#include <type_traits>
#include <cstdarg>
#include "model.h"

namespace sorm {
    template<class _m> class _is_sorm_model {
    public:
        typedef char one;
        typedef struct {char c[2];} to;
        template<class _t> static to tst(typename _t::_sorm_model_tag);
        template<class _t> static one tst(...);
        enum {value = sizeof(tst<_m>(0)) == sizeof(to)};
    };

    typedef std::vector<std::pair<char*, std::pair<char*, std::string>>> _where_cont;
    typedef enum {
        UNKNOWN, SELECT
    }_op_verb;
    typedef std::vector<std::string> _col;

    
    template<class _escaper> class DefaultGenerator {
    public:
        template<class _model> std::string operator() (const _where_cont& where, const _op_verb& v, const _col& columns) {
            std::string sql;
            switch (v) {
            case SELECT:
                _select(sql, _model().sorm_table_name(), where, columns); break;
            case UNKNOWN:
                break;
            }
            return sql;
        }

        void _select(std::string &sql, const char* name, const _where_cont& where, const _col& columns) {
            sql += "select ";
            _escaper escape;
            if (columns.size() > 0) {
                _col::const_iterator cit, cen;
                for(cit = columns.begin(), cen = columns.end(); cit != cen; cit++) {
                    sql += "`" + escape(*cit) + "`";
                    if(cit + 1 != cen) {
                        sql += ",";
                    }
                }
            } else {
                sql += "*";
            }
            sql += " from `";
            sql += escape(name);
            sql += "`";
            if (where.size() > 0) {
                sql += " where (";
                _where_cont::const_iterator it, en;
                for(it = where.begin(), en = where.end(); it != en; it++) {
                    char *col = it -> first;
                    char *op = it -> second.first;
                    const std::string &exp = it -> second.second;
                    if (!col || strlen(col) < 1) {
                        sql += exp;
                    }
                    if (it + 1 != en) {
                        sql += ") and (";
                    } else {
                        sql += ")";
                    }
                }
            }
        }
    };

    template<class _model>
    class Query {
    public:
        Query()
        :verb(UNKNOWN)
        {
            static_assert(_is_sorm_model<_model>::value, "not a sorm model.");
        }
        ~Query() { clean_up(); }

        Query<_model>& where(std::string&& cond) {
            if (cond.size() < 1) return *this;

            where_clause.push_back(
                make_pair(
                    (char*)0,
                    make_pair((char*)0, std::move(cond))
                )
            );
            return *this;
        }

        Query<_model>& where(std::string& cond) {
            return where(std::move(cond));
        }

        Query<_model>& select(const char* c = 0) {
            verb = SELECT;
            if (!c || strlen(c) < 1 ) return *this;
            cols.push_back(std::string(c));
            return *this;
        }

        const std::vector<std::pair<char*, std::pair<char*, std::string>>>& get_where_clause() const {
            return where_clause;
        }

        std::string table_name() const {
            return std::string(_model().sorm_table_name());
        }

        template<class _generator> std::string expr() const {
            return _generator().template operator()<_model>(where_clause, verb, cols);
        }

    private:
        void clean_up() {};

    protected:
        _where_cont where_clause;
        _op_verb verb;
        _col cols;
    };
}

#endif