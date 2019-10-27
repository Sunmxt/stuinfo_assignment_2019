#include "student.h"

const sorm::field_metadata Student::_sorm_meta[] = {
    {"id", sorm::PrimaryKey | sorm::Unique | sorm::NotNull },
    {"name", sorm::NotNull},
    {"stuno", sorm::NotNull},
    {"credit", sorm::NotNull},
    {"tel", sorm::NotNull},
    {"gender", sorm::NotNull},
    {"email", sorm::NotNull},
    {"", 0},
};

Student::Student()
:id(0), gender(0)
{
    sorm_bind("id", id);
    sorm_bind("name", name);
    sorm_bind("stuno", stuno);
    sorm_bind("credit", credit);
    sorm_bind("tel", tel);
    sorm_bind("gender", gender);
    sorm_bind("email", email);
}

Student::Student(const Student& v) 
:Student() {
    id = v.id;
    name = v.name;
    stuno = v.stuno;
    credit = v.credit;
    tel = v.tel;
    gender = v.gender;
    email = v.email;
}