#include <glog/logging.h>
#include <string>
#include "model.h"

using namespace sorm;

Model::Model()
:_sorm_fields(0), _sorm_num_of_fields(0)
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
        delete _sorm_fields;
    }
    _sorm_num_of_fields = 0;
}

const Field* Model::sorm_primary_key_field(bool allow_duplicated) {
    const Field *field = _sorm_fields;
    const Field *primary = 0;

    // locate primary column.
    for(int i = _sorm_num_of_fields; i ; i--) {
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

std::string Field::string() const {
    if (!ref) return std::string();

    switch (value_type) {
    case INT_MODEL_FIELD:
        return mapper_value_type_trait<INT_MODEL_FIELD>::ToString(*(int*)ref);

    case CHAR_STRING_FIELD:
        return "`" + mapper_value_type_trait<CHAR_STRING_FIELD>::ToString((char*)ref) + "`";

    default:
        LOG(ERROR) << "cannot convert unsupported type " << value_type << " to sql string.";
    }

    return std::string();
}

bool Field::is_zero_value() const {
    if (!ref) return true;

    switch (value_type) {
    case INT_MODEL_FIELD:
        return mapper_value_type_trait<INT_MODEL_FIELD>::IsZeroValue(*(int*)ref);

    case CHAR_STRING_FIELD:
        return mapper_value_type_trait<CHAR_STRING_FIELD>::IsZeroValue((char*)ref);

    default:
        LOG(ERROR) << "got unsupported type " << value_type;
    }

    return true;
}