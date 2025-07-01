#pragma once
#include <filesystem>
#include <cstring>
#include <iostream>
#include <unistd.h>
struct b_item {
    std::string name;
    std::filesystem::file_type type; 
    b_item(const char*name){
        std::filesystem::path p(name);
        std::error_code ec;
        std::filesystem::file_status st = std::filesystem::status(p,ec);
        if(ec) {
            std::cout << "Error getting status of " << p << '\n';
            exit(1);
        }
        this->type = st.type();
        this->name = name;
    };
};
