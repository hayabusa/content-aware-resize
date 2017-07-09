//
// Created by nikita on 04.07.17.
//

#ifndef CONTENTAWARERESIZE_CONFIG_H
#define CONTENTAWARERESIZE_CONFIG_H


#include <string>
#include <map>
#include <memory>
#include <boost/program_options/variables_map.hpp>
#include "singleton.h"

class Config : public Singleton<Config> {
    friend class Singleton<Config>;

public:
    Config();

    ~Config();

    Config(const Config &) = delete;

    Config &operator=(const Config &) = delete;

    std::string operator[](const std::string &parameter);

//private:
    std::unique_ptr<std::map<std::string, std::string>> data;

    void parse_config();

    void update_from(const boost::program_options::variables_map &vm);

    void print();
};


#endif //CONTENTAWARERESIZE_CONFIG_H
