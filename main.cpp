#include <iostream>
#include <memory>
#include <glog/logging.h>
#include "sorm/model.h"
#include "sorm/indexer.h"
#include "sorm/context.h"
#include "student.h"

using namespace std;

int main() {
    google::InitGoogleLogging("XXX");
    google::SetCommandLineOption("logtostderr", "true");

    sorm::SQLite3Indexer idxer;
    Student stu;
    idxer.open("stuinfo.dat");
    idxer.init_model(stu);

    stu.name = "ccc";
    stu.stuno = "2016070904014";
    idxer.save(stu);

    idxer.close();
    return 0;
}