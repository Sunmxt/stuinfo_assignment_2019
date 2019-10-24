#ifndef SORM_MODEL_H
#define SORM_MODEL_H

#include <cstdint>
#include <type_traits>
#include <map>
#include <string>
#include <atomic>


namespace sorm {
    typedef struct _field_metadata { 
        const char* name;
        const uint32_t props;
    }field_metadata;

    enum {
        UNKNOWN_FIELD,
        INT_MODEL_FIELD,
        CHAR_STRING_FIELD
    };

    const uint32_t Unique = 1;
    const uint32_t PrimaryKey = 1 << 1;
    const uint32_t NotNull = 1 << 2;

    template<class _ty> class model_mapper_supported_type {
    public:
        static const int value = 0;
        static const unsigned char value_type = UNKNOWN_FIELD;
    };

    template<> class model_mapper_supported_type<int> {
    public:
        static const int value = 1;
        static const unsigned char value_type = INT_MODEL_FIELD;
    };

    template<> class model_mapper_supported_type<char*> {
    public:
        static const unsigned char value_type = CHAR_STRING_FIELD;
        static const int value = 1;
    };

    #define SORM_MODEL(table_name)\
    public:\
        static const sorm::field_metadata _sorm_meta[];\
        inline const char *sorm_table_name() const { return #table_name; }\
        inline const sorm::field_metadata *sorm_meta() const { return _sorm_meta; } \
    private:

    class Field {
    public:
        void* ref;
        unsigned char value_type;
        const field_metadata *meta;

        template<class _ty> void bind(_ty& v) { 
            static_assert(model_mapper_supported_type<_ty>::value, "the type of reference is not supported by model mapper.");
            ref = (void*)&v;
            value_type = model_mapper_supported_type<_ty>::value_type;
        }
    };

    class Model {
    public:
        Model();
        ~Model();

        virtual const char *sorm_table_name() const = 0;
        virtual const field_metadata *sorm_meta() const = 0;

        template<class _ty> void sorm_bind(const char *name, _ty v) {
            sorm_init_metadata();

            std::string n(name);
            Field *f = _sorm_field_by_name[n];
            if (!f) {
                n.assign("sorm bind to an undefined field \"");
                n += name;
                n += "\"";
            }
            f -> bind(v);
        }
        inline const Field *sorm_fields() const { return _sorm_fields; }
        inline int sorm_num_of_fields() const {return _sorm_num_of_fields;}

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