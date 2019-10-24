#include "student.h"

const sorm::field_metadata Student::_sorm_meta[] = {
    {"id", sorm::PrimaryKey | sorm::Unique | sorm::NotNull },
    {"name", sorm::NotNull},
    {"stuno", sorm::NotNull},
    {"", 0},
};

Student::Student() {
    id = 0;
    name = 0;
    stuno = 0;

    sorm_bind("id", id);
    sorm_bind("name", name);
    sorm_bind("stuno", stuno);
}


Student::~Student() {
}