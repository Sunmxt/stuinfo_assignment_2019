#include <glog/logging.h>
#include <string>
#include "model.h"

using namespace sorm;

Model::Model()
:_sorm_fields(0), _sorm_num_of_fields(0), inited(false)
{}

Model::~Model() {
    if (inited.exchange(false)) {
        sorm_clean_up();
    }
}

void Model::sorm_init_metadata() {
    if (inited.exchange(true)) return;

    const field_metadata *metas = sorm_meta();
    _sorm_num_of_fields = 0;
    if (metas) {
        const field_metadata *meta = metas;
        while(meta && meta -> name[0] != '\0') {
            _sorm_num_of_fields ++;
            meta++;
        }
        _sorm_fields = new Field[_sorm_num_of_fields];
        for(int i = _sorm_num_of_fields; i; i--) {
            meta = metas + i - 1;
            Field *field = _sorm_fields + i - 1;
            _sorm_field_by_name[std::string(meta -> name)] = field;
            field -> meta = meta;
        }
    }
}

void Model::sorm_clean_up() {
    if (_sorm_fields) {
        delete[] _sorm_fields;
    }
    _sorm_num_of_fields = 0;
}

Field* Model::sorm_primary_key_field(bool allow_duplicated) {
    Field *field = _sorm_fields;
    Field *primary = 0;

    // locate primary column.
    for(int i = _sorm_num_of_fields; i ; i--, field++) {
        if (field -> meta -> props & PrimaryKey) {
            if (primary) {
                LOG(WARNING) << "duplicated primary keys found: \"" 
                           << primary -> meta -> name
                           << "\" and \""
                           << field -> meta -> name
                           << "\"";
                if (!allow_duplicated) {
                    LOG(ERROR) << "duplicated primary keys is not allowed. return zero.";
                    return 0;
                }
            }
            primary = field;
        }
    }
    return primary;
}

Field::Field()
:ref(0), value_type(UNKNOWN_FIELD), meta(0)
{}

Field::~Field() {}


bool Field::is_zero_value() const {
    if (!ref) return true;

    switch (value_type) {
    case INT_MODEL_FIELD:
        return mapper_value_type_trait<INT_MODEL_FIELD>::IsZeroValue(*(int*)ref);

    case STL_STRING_FIELD:
        return mapper_value_type_trait<STL_STRING_FIELD>::IsZeroValue((*(std::string*)ref));

    default:
        LOG(ERROR) << "got unsupported type " << value_type;
    }

    return true;
}