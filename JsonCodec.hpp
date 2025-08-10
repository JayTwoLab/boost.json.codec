// JsonCodec.hpp
// Boost.JSON + Boost.Describe wrapper (header-only, compatible with Boost 1.88)
#pragma once

#ifndef BOOST_JSON_CODEC_HPP
#define BOOST_JSON_CODEC_HPP

#include <type_traits>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

#include <boost/json.hpp>
#include <boost/describe.hpp>
#include <boost/mp11.hpp>

namespace json = boost::json;
namespace bd   = boost::describe;
namespace mp11 = boost::mp11;

// Boost 1.88: has_describe_members<T> (no accessor arguments)
template<class T>
using EnableIfDescribed = std::enable_if_t<bd::has_describe_members<T>::value>;

// Common serialization for described types (struct -> JSON)
template<class T, class = EnableIfDescribed<T>>
void tag_invoke(json::value_from_tag, json::value& jv, T const& t) {
    json::object obj;
    using Members = bd::describe_members<T, bd::mod_public>;
    mp11::mp_for_each<Members>([&](auto D){
        obj[D.name] = json::value_from(t.*(D.pointer));
    });
    jv = std::move(obj);
}

class JsonCodec {
public:
    enum class MissingPolicy { Strict, Lenient };

    // Simple pretty printer (string-based pretty print): small utility
    static std::string simple_pretty(std::string s, int indent = 2) {
        std::string out; out.reserve(s.size() + s.size()/4);
        int level = 0; bool in_str = false; bool esc = false;
        for (char c : s) {
            if (in_str) {
                out.push_back(c);
                if (esc) { esc = false; }
                else if (c == '\\') { esc = true; }
                else if (c == '"') { in_str = false; }
                continue;
            }
            switch (c) {
            case '"': in_str = true; out.push_back(c); break;
            case '{': case '[':
                out.push_back(c);
                out.push_back('\n');
                ++level;
                out.append(level*indent, ' ');
                break;
            case '}': case ']':
                out.push_back('\n');
                if (level > 0) --level;
                out.append(level*indent, ' ');
                out.push_back(c);
                break;
            case ',':
                out.push_back(c);
                out.push_back('\n');
                out.append(level*indent, ' ');
                break;
            case ':':
                out.push_back(c);
                out.push_back(' ');
                break;
            default:
                if (!(c==' ' || c=='\n' || c=='\t' || c=='\r')) out.push_back(c);
                break;
            }
        }
        return out;
    }

    template<class T, class = EnableIfDescribed<T>>
    static json::value toValue(const T& obj) {
        return json::value_from(obj);
    }

    template<class T, class = EnableIfDescribed<T>>
    static T fromValue(const json::value& jv, MissingPolicy policy = MissingPolicy::Strict) {
        if (!jv.is_object()) throw std::runtime_error("JSON root is not an object.");
        const json::object& obj = jv.as_object();

        T out{};
        using Members = bd::describe_members<T, bd::mod_public>;
        mp11::mp_for_each<Members>([&](auto D){
            using MemberType = std::remove_reference_t<decltype(std::declval<T&>().*D.pointer)>;
            auto it = obj.find(D.name);
            if (it == obj.end()) {
                if (policy == MissingPolicy::Strict)
                    throw std::runtime_error(std::string("Missing required key: ") + D.name);
                return; // Lenient: keep default value
            }
            try {
                (out.*(D.pointer)) = json::value_to<MemberType>(it->value());
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Field conversion failed: ") + D.name + " - " + e.what());
            }
        });
        return out;
    }

    template<class T, class = EnableIfDescribed<T>>
    static std::string toString(const T& obj, bool pretty = true) {
        std::string s = json::serialize(toValue(obj)); // compact
        return pretty ? simple_pretty(std::move(s)) : s;
    }

    template<class T, class = EnableIfDescribed<T>>
    static T fromString(const std::string& s, MissingPolicy policy = MissingPolicy::Strict) {
        boost::system::error_code ec;
        json::value v = json::parse(s, ec);
        if (ec) throw std::runtime_error("JSON parsing failed: " + ec.message());
        return fromValue<T>(v, policy);
    }

    template<class T, class = EnableIfDescribed<T>>
    static void saveFile(const std::string& path, const T& obj, bool pretty = true) {
        saveValue(path, toValue(obj), pretty);
    }

    template<class T, class = EnableIfDescribed<T>>
    static T loadFile(const std::string& path, MissingPolicy policy = MissingPolicy::Strict) {
        return fromValue<T>(loadValue(path), policy);
    }

    static std::string dump(const json::value& v, bool pretty = true) {
        std::string s = json::serialize(v); // compact
        return pretty ? simple_pretty(std::move(s)) : s;
    }

    static json::value parse(const std::string& s) {
        boost::system::error_code ec;
        json::value v = json::parse(s, ec);
        if (ec) throw std::runtime_error("JSON parsing failed: " + ec.message());
        return v;
    }

    static void saveValue(const std::string& path, const json::value& v, bool pretty = true) {
        std::ofstream ofs(path, std::ios::binary);
        if (!ofs) throw std::runtime_error("Cannot write to file: " + path);
        std::string s = dump(v, pretty);
        ofs.write(s.data(), static_cast<std::streamsize>(s.size()));
    }

    static json::value loadValue(const std::string& path) {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs) throw std::runtime_error("Cannot open file: " + path);
        std::string s((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        boost::system::error_code ec;
        json::value v = json::parse(s, ec);
        if (ec) throw std::runtime_error("JSON parsing failed: " + ec.message());
        return v;
    }
};

#endif
