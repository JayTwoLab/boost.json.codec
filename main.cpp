// main.cpp

#include "model.hpp"
#include "AutoDescribe.hpp"
#include "describe_all.gen.hpp"   // describe first
#include "JsonCodec.hpp"          // then JsonCodec

#include <iostream>
#include <string>
#include <vector>

int main() {
    try {
        hello::world::User u1{
            "Hong Gil-dong",
            29,
            true,
            hello::world::Profile{"hong@example.com", 98.5},
            {
                hello::world::Project{"VisionAI", {"rtsp", "opencv", "tracking"}},
                hello::world::Project{"Backend",  {"cpp", "asio", "protobuf"}}
            }
        };

        std::string s1 = JsonCodec::toString(u1, true);
        std::cout << "[User -> JSON]\n" << s1 << "\n\n";

        const char* js = R"({
            "name": "Lee Mong-ryong",
            "age": 21,
            "admin": false,
            "profile": { "email": "lee@example.com", "score": 91.2 },
            "projects": [
                { "title": "Render", "tags": ["vulkan", "pbr"] },
                { "title": "Network", "tags": ["cpp", "boost", "http"] }
            ]
        })";

        hello::world::User u2 =
            JsonCodec::fromString<hello::world::User>(js, JsonCodec::MissingPolicy::Strict);

        std::cout << "[JSON -> User] "
                  << "name="   << u2.name
                  << ", age="  << u2.age
                  << ", admin="<< (u2.admin ? "true":"false")
                  << ", email="<< u2.profile.email
                  << ", score="<< u2.profile.score
                  << ", projects=" << u2.projects.size() << " items\n";
        if (!u2.projects.empty()) {
            std::cout << "  First project: " << u2.projects[0].title << "\n";
        }

        JsonCodec::saveFile("user.json", u1, true);

        hello::world::User u3 =
            JsonCodec::loadFile<hello::world::User>("user.json", JsonCodec::MissingPolicy::Lenient);

        std::cout << "[File Load] name=" << u3.name
                  << ", projects=" << u3.projects.size() << " items\n";

        NonNamespaceProfile nnp1;

    } catch (const std::exception& e) {
        std::cerr << "[Error] " << e.what() << "\n";
        return 1;
    }
    return 0;
}
