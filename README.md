# Boost.JSON AutoDescribe Demo

[한국어 설명 보기](README.ko.md)

## Overview
This project demonstrates an automatic conversion system between C++ structures and JSON using **Boost.JSON** and **Boost.Describe**.  
By defining the "field sequence" only once, it can automatically generate both the struct definition and `BOOST_DESCRIBE_STRUCT` declarations, reducing redundancy and improving maintainability.

## Features
- **Automatic Struct Description**  
  Uses `AutoDescribe.hpp` to generate struct definitions and `BOOST_DESCRIBE_STRUCT` at the same time.

- **JSON Serialization / Deserialization**  
  `JsonCodec.hpp` provides easy-to-use functions to convert described structs to and from JSON using Boost.JSON.

- **Pretty Printing**  
  Built-in pretty printer for better readability.

- **File I/O**  
  Save and load JSON data directly to and from files.

- **Clang-based Code Generation**  
  Python tools (`gen_fields.py`, `gen_all_structs.py`) use libclang to scan headers and generate `BOOST_DESCRIBE_STRUCT` automatically.

## Dependencies
- **Boost** >= 1.75 (Tested with Boost 1.88)
- **Python 3** with `clang` bindings
- **libclang** (LLVM)

## Building
```bash
cmake -B build -S .   -DCMAKE_BUILD_TYPE=Release   -DLIBCLANG_DLL=/path/to/libclang.dll
cmake --build build
```

## Example
```cpp
hello::world::User user{"John Doe", 30, true, {"john@example.com", 95.5}, {}};
std::string json_str = JsonCodec::toString(user, true);
std::cout << json_str << std::endl;

hello::world::User loaded = JsonCodec::fromString<hello::world::User>(json_str);
```

## License
This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
