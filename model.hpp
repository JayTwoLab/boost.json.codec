#pragma once
#ifndef MODEL_HPP
#define MODEL_HPP

#include <string>
#include <vector>

namespace hello::world {

struct Project {
    std::string title = "";
    std::vector<std::string> tags = {};
};

struct Profile {
    std::string email = "";
    double score = 0.0;
};

struct User {
    std::string name = "";
    int age = 0;
    bool admin = false;
    Profile profile = Profile{};
    std::vector<Project> projects = std::vector<Project>{};
};

} // namespace

struct NonNamespaceProfile {
    std::string email = "";
    double score = 0.0;
};

#endif // MODEL_HPP
