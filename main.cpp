#include <iostream>
#include <memory>
#include <glog/logging.h>
#include "sorm/model.h"
#include "sorm/indexer.h"
#include "sorm/query.h"
#include "student.h"


class Application {
public:
    void get_user_command(std::vector<std::string>& s);

    void cmd_user_register_new_student(std::vector<std::string>& cmds);
    // register name=myname stuno=2016070904012 gender=male

    void cmd_show_student(std::vector<std::string>& cmds);
    // show
    // show stuno=2016070904018
    // show tel=13538291833
    // show id>18
    // ...

    int run();

protected:
    sorm::SQLite3Indexer idxer;

};

void Application::get_user_command(std::vector<std::string>& s) {
    std::string cmdline;
    std::cout << "\n > ";
    std::getline(std::cin, cmdline);
    #define SPACED ((unsigned char)1)
    #define QUO_1 ((unsigned char)2)
    #define QUO_2 ((unsigned char)4)
    #define SKIP ((unsigned char)8)

    unsigned char flags = SPACED;
    const char *part_begin = cmdline.c_str();
    const char *c = part_begin;
    std::string part;

    for(; c[0] != '\0'; c++) {
        if (flags & SKIP) {
            flags &= ~SKIP;
            continue;
        }
        if ( c < part_begin ) continue;
        if (c[0] == '\\') {
            flags |= SKIP;
            part += std::string(part_begin, c);
            part_begin = c+1;
            continue;
        }
        if (flags & QUO_1) {
            if (c[0] == '\'') {
                part += std::string(part_begin, c);
                part_begin = c + 1;
                flags &= ~QUO_1;
            }
            continue;
        }
        if (flags & QUO_2) {
            if (c[0] == '"') {
                part += std::string(part_begin, c);
                part_begin = c + 1;
                flags &= ~QUO_2;
            }
            continue;
        }
        switch (c[0]) {
        case ' ':
        case '\t':
            if (!(flags & SPACED)) {
                part += std::string(part_begin, c);
                s.push_back(part);
                part.resize(0);
                part_begin = c;
            };
            part_begin ++;
            flags |= SPACED; break;

        case '\'':
            part += std::string(part_begin, c);
            part_begin = c + 1;
            flags |= QUO_1; break;
            
        case '"':
            part += std::string(part_begin, c);
            part_begin = c + 1;
            flags |= QUO_2; break;

        default:
            flags &= ~SPACED;
        }
    }
    if (!(flags & SPACED)) {
        part += std::string(part_begin, c);
        s.push_back(part);
    }
}

int Application::run() {
    idxer.open("stuinfo.dat");
    idxer.init_model(Student());

    std::vector<std::string> cmds;
    while(1) {
        cmds.resize(0);
        get_user_command(cmds);
        if (cmds.size() < 1) {
            continue;
        }
        std::string &cmd = cmds[0];
        if (cmd.compare("exit") == 0 || cmd.compare("quit") == 0) {
            break;
        } else if (cmd.compare("register") == 0) {
            cmd_user_register_new_student(cmds);
        } else if (cmd.compare("show") == 0) {
            cmd_show_student(cmds);
        } else {
            std::cout << "unknown command: " << cmd << std::endl;
        }
    }
    return 0;
}

void Application::cmd_user_register_new_student(std::vector<std::string>& cmds) {
    if (cmds.size() < 2) {
        std::cout << "missing student data." << std::endl;
        return;
    }
    Student stu;
    std::vector<std::string>::iterator it = cmds.begin() + 1;
    std::vector<std::string>::iterator end = cmds.end();
    while(it != end) {
        const char *sp = strchr(it -> c_str(), '=');
        std::string &&name = std::string(it -> c_str(), sp);
        std::string &&val = std::string(sp+1, it -> c_str() + strlen(it -> c_str()));
        sorm::Field *field = stu.sorm_field_by_name(std::move(name));
        if (!field) {
            std::cout << "unknown field: " << name;
            return;
        }
        if (name.compare("gender")) {
            if (val.compare("male")) {
                stu.gender = 1;
            } else if (val.compare("female")) {
                stu.gender = 2;
            } else {
                std::cout << "unknown gender: " << val;
            }
        }
        field -> parse(val);
        it++;
    }
    idxer.save(stu);
    std::cout << "new student id: " << stu.id;
}

void Application::cmd_show_student(std::vector<std::string>& cmds) {
    if (cmds.size() < 1) {
        return;
    }

    std::vector<Student> stus;

    std::vector<std::string>::iterator it = cmds.begin() + 1;
    std::vector<std::string>::iterator end = cmds.end();
    std::string cond;
    while(it != end) {
        cond += *it;
        it++;
    }
    idxer.exec(sorm::Query<Student>().select().where(cond), stus);
    if (stus.size() < 1) {
        std::cout << "no result found.";
        return;
    } 
    std::vector<Student>::iterator cit, cend;
    for(cit = stus.begin(), cend = stus.end();cit != cend; cit++) {
        std::cout << "id=" << cit -> id << ", " << "name=" << cit -> name << ", " << "stuno=" << cit -> stuno << ", " << "credit=" << cit -> credit << ", "
                  << "tel=" << cit -> tel << ", " << "gender=" << cit -> gender << ", " << "email=" << cit -> email << std::endl;
    }
}

int main() {
    google::InitGoogleLogging("XXX");
    google::SetCommandLineOption("logtostderr", "true");

    Application app;
    return app.run();

    //std::vector<Student> stus;
    //idxer.exec(
    //    sorm::Query<Student>().where(
    //        sorm::Query<Grade>().select("stuno").where("math", ">=", 90),
    //        "or",
    //        sorm::Query<Grade>().select("stuno").where("computer", ">=", 90)
    //    ).order_by("stuno desc"),
    //    stus
    //);

    //idxer.save(stu);
    //LOG(INFO) << "row id: " << stu.id;
}