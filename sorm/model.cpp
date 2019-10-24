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