#include <iostream>
#include <memory>
#include <glog/logging.h>
#include "sorm/model.h"
#include "sorm/indexer.h"
#include "student.h"

using namespace std;

int main() {
    google::InitGoogleLogging("XXX");
    google::SetCommandLineOption("logtostderr", "true");

    sorm::SQLite3Indexer idxer;
    Student stu;
    idxer.open("stuinfo.dat");
    idxer.init_model(stu);
    idxer.close();
    return 0;
}