#ifndef STUDENT_MODEL_H
#define STUDENT_MODEL_H

#include "sorm/model.h"

class Student: public sorm::Model {
    SORM_MODEL("student");
public:
    Student();
    ~Student();

public:
    int id;
    char *name;
    char *stuno;
};

#endif