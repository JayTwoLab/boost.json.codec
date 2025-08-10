// main.cpp

#include "model.hpp"
#include "AutoDescribe.hpp"
#include "JsonCodec.hpp"

#include <iostream>
#include <string>
#include <vector>

// 생성된 describe/시퀀스 매크로는 마지막에 포함
#include "describe_all.gen.hpp"

int main() {
    try {
        User u1{
            "홍길동",
            29,
            true,
            Profile{"hong@example.com", 98.5},
            {
                Project{"VisionAI", {"rtsp", "opencv", "tracking"}},
                Project{"Backend",  {"cpp", "asio", "protobuf"}}
            }
        };

        std::string s1 = JsonCodec::toString(u1, true);
        std::cout << "[User -> JSON]\n" << s1 << "\n\n";

        const char* js = R"({
            "name": "이몽룡",
            "age": 21,
            "admin": false,
            "profile": { "email": "lee@example.com", "score": 91.2 },
            "projects": [
                { "title": "Render", "tags": ["vulkan", "pbr"] },
                { "title": "Network", "tags": ["cpp", "boost", "http"] }
            ]
        })";
        User u2 = JsonCodec::fromString<User>(js, JsonCodec::MissingPolicy::Strict);
        std::cout << "[JSON -> User] "
                  << "name="   << u2.name
                  << ", age="  << u2.age
                  << ", admin="<< (u2.admin ? "true":"false")
                  << ", email="<< u2.profile.email
                  << ", score="<< u2.profile.score
                  << ", projects=" << u2.projects.size() << "개\n";
        if (!u2.projects.empty()) {
            std::cout << "  첫 프로젝트: " << u2.projects[0].title << "\n";
        }

        JsonCodec::saveFile("user.json", u1, true);
        User u3 = JsonCodec::loadFile<User>("user.json", JsonCodec::MissingPolicy::Lenient);
        std::cout << "[파일 로드] name=" << u3.name
                  << ", projects=" << u3.projects.size() << "개\n";

    } catch (const std::exception& e) {
        std::cerr << "[오류] " << e.what() << "\n";
        return 1;
    }
    return 0;
}
