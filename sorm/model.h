#ifndef SORM_MODEL_H
#define SORM_MODEL_H

#include <cstdint>
#include <type_traits>
#include <map>
#include <string>
#include <atomic>
#include <string>
#include <glog/logging.h>

namespace sorm {
    typedef struct _field_metadata { 
        const char* name;
        const uint32_t props;
    }field_metadata;

    enum {
        UNKNOWN_FIELD,
        INT_MODEL_FIELD,
        STL_STRING_FIELD
    };

    const uint32_t Unique = 1;
    const uint32_t PrimaryKey = 1 << 1;
    const uint32_t NotNull = 1 << 2;
    const uint32_t AutoIncrement = 1 << 3;

    template<uint32_t value_type> class mapper_value_type_trait {};

    template<> class mapper_value_type_trait<STL_STRING_FIELD> {
    public:
        static inline std::string& ToString(std::string &s) {
            return s;
        }
        static inline bool IsZeroValue(std::string &s) {
            return s.length() == 0;
        }
        static inline std::string& Parse(std::string& s) {
            return s;
        }
        static inline std::string&& Parse(std::string&& s) {
            return std::move(s);
        }
        static inline std::string Parse(const char *s) {
            return std::string(s);
        }
        static inline std::string Parse(long long int v) {
            return std::to_string(v);
        }
    };

    template<> class mapper_value_type_trait<INT_MODEL_FIELD> {
    public:
        static std::string ToString(int v) {
            return std::to_string(v);
        }
        static bool IsZeroValue(int v) {
            return v == 0;
        }
        static inline int Parse(int v) { return v; }
        static inline int Parse(long long int v) { return int(v); }
        static inline int Parse(std::string& v) {
            return std::atoi(v.c_str());
        }
        static inline int Parse(const char *v) {
            return std::atoi(v);
        }
    };

    template<class _ty> class model_mapper_supported_type {
    public:
        static const int value = 0;
        static const unsigned char value_type = UNKNOWN_FIELD;
    };

    template<> class model_mapper_supported_type<int> {
    public:
        static const int value = 1;
        static const unsigned char value_type = INT_MODEL_FIELD;
        typedef mapper_value_type_trait<INT_MODEL_FIELD> trait;
    };

    template<> class model_mapper_supported_type<std::string> {
    public:
        static const unsigned char value_type = STL_STRING_FIELD;
        static const int value = 1;
        typedef mapper_value_type_trait<STL_STRING_FIELD> trait;
    };

    #define SORM_MODEL(table_name)\
    public:\
        static const sorm::field_metadata _sorm_meta[];\
        inline const char *sorm_table_name() const { return #table_name; }\
        inline const sorm::field_metadata *sorm_meta() const { return _sorm_meta; } \
    private:

    class Field {
    public:
        Field();
        ~Field();

        void* ref;
        unsigned char value_type;
        const field_metadata *meta;

        template<class _ty> void bind(_ty& v) {
            static_assert(model_mapper_supported_type<_ty>::value, "the type of reference is not supported by model mapper.");
            ref = (void*)&v;
            value_type = model_mapper_supported_type<_ty>::value_type;
        }

        bool is_zero_value() const;

        template<class _ty> std::string string(_ty escaper) const {
            if (!ref) return std::string();

            switch (value_type) {
            case INT_MODEL_FIELD:
                return mapper_value_type_trait<INT_MODEL_FIELD>::ToString(*(int*)ref);

            case STL_STRING_FIELD:
                return "\"" + mapper_value_type_trait<STL_STRING_FIELD>::ToString(*((std::string*)ref)) + "\"";

            default:
                LOG(ERROR) << "cannot convert unsupported type " << value_type << " to sql string.";
            }

            return std::string();
        }

        template<class _ty> void parse(_ty &&s) {
            if (!ref) return;

            switch (value_type) {
            case INT_MODEL_FIELD:
                *((int*)ref) = mapper_value_type_trait<INT_MODEL_FIELD>::Parse(s); break;

            case STL_STRING_FIELD:
                *((std::string*)ref) = mapper_value_type_trait<STL_STRING_FIELD>::Parse(s); break;

            default:
                LOG(ERROR) << "cannot parse value for unsupported type " << value_type;
            }
        }

    };

    class Model {
    public:
        typedef int _sorm_model_tag;

    public:
        Model();
        ~Model();

        virtual const char *sorm_table_name() const = 0;
        virtual const field_metadata *sorm_meta() const = 0;

        template<class _ty> void sorm_bind(const char *name, _ty& v) {
            sorm_init_metadata();

            std::string n(name);
            Field *f = _sorm_field_by_name[n];
            if (!f) {
                n.assign("sorm bind to an undefined field \"");
                n += name;
                n += "\"";
                LOG(ERROR) << n.c_str();
                throw n.c_str();
            }
            f -> bind(v);
        }
        inline Field *sorm_fields() { sorm_init_metadata(); return _sorm_fields; }
        inline int sorm_num_of_fields() { sorm_init_metadata(); return _sorm_num_of_fields;}
        inline Field* sorm_field_by_name(const char *name) {
            sorm_init_metadata();
            return sorm_field_by_name(std::string(name));
        }
        inline Field* sorm_field_by_name(std::string&& name){
            sorm_init_metadata();
            std::map<std::string, Field*>::iterator it = _sorm_field_by_name.find(std::string(name));
            if (it != _sorm_field_by_name.end()) {
                return it -> second;
            }
            return 0;
        }

        Field* sorm_primary_key_field(bool allow_duplicated = false);

    private:
        std::atomic_bool inited;
        void sorm_init_metadata();
        void sorm_clean_up();

    protected:
        Field *_sorm_fields;
        std::map<std::string, Field*> _sorm_field_by_name;
        int _sorm_num_of_fields;
    };

}

#endif