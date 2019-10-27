#ifndef STUDENT_MODEL_H
#define STUDENT_MODEL_H

#include "sorm/model.h"
#include <string>

class Student: public sorm::Model {
    SORM_MODEL(student);

public:
    Student();
    Student(const Student& v);

public:
    int id;
    std::string name;
    std::string stuno;
    int credit;
    std::string tel;
    int gender;
    std::string email;
};

#endif