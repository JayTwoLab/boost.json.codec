# Boost.JSON AutoDescribe 데모

[English](README.md)

## 개요
이 프로젝트는 **Boost.JSON**과 **Boost.Describe**를 이용하여 C++ 구조체와 JSON 간의 자동 변환을 구현한 예제입니다.  
"필드 시퀀스"를 한 번만 정의하면 구조체 정의와 `BOOST_DESCRIBE_STRUCT` 선언을 자동으로 생성하여 코드 중복을 줄이고 유지보수성을 향상시킵니다.

## 주요 기능
- **자동 구조체 기술**  
  `AutoDescribe.hpp`를 사용해 구조체 정의와 `BOOST_DESCRIBE_STRUCT`를 동시에 생성합니다.

- **JSON 직렬화 / 역직렬화**  
  `JsonCodec.hpp`를 통해 기술된 구조체를 Boost.JSON을 사용하여 손쉽게 JSON으로 변환하거나, JSON에서 구조체로 변환할 수 있습니다.

- **Pretty 출력**  
  가독성을 높이기 위한 내장 프리티 프린터 제공.

- **파일 입출력**  
  JSON 데이터를 파일에 저장하고 파일에서 불러오는 기능 제공.

- **Clang 기반 코드 생성**  
  Python 도구(`gen_fields.py`, `gen_all_structs.py`)가 libclang을 사용하여 헤더를 스캔하고 `BOOST_DESCRIBE_STRUCT`를 자동 생성합니다.

## 의존성
- **Boost** >= 1.75 (Boost 1.88에서 테스트됨)
- **Python 3** (`clang` 바인딩 포함)
- **libclang** (LLVM)

## 빌드 방법
```bash
cmake -B build -S .   -DCMAKE_BUILD_TYPE=Release   -DLIBCLANG_DLL=/path/to/libclang.dll
cmake --build build
```

## 예제
```cpp
hello::world::User user{"John Doe", 30, true, {"john@example.com", 95.5}, {}};
std::string json_str = JsonCodec::toString(user, true);
std::cout << json_str << std::endl;

hello::world::User loaded = JsonCodec::fromString<hello::world::User>(json_str);
```

## 라이선스
이 프로젝트는 MIT 라이선스를 따릅니다. 자세한 내용은 [LICENSE](LICENSE) 파일을 참고하세요.
