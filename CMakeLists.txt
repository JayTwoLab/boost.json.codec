cmake_minimum_required(VERSION 3.30)
project(boost_json_autodescribe_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------------------------------
# 경고 억제를 원하면(선택): NEW로 고정
#   - 모듈 모드 FindBoost 제거를 명시적으로 수용
# --------------------------------------------------------------------------
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# --------------------------------------------------------------------------
# Windows + Visual C++
# --------------------------------------------------------------------------
if (MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# --------------------------------------------------------------------------
# Windows + MinGW64
#  - 중요: toolchain은 가급적 CMake 최초 구성 단계에서 -DCMAKE_TOOLCHAIN_FILE=... 로 넘기세요.
#          (CMakeLists 안에서 바꾸면 이미 설정된 후라 무시되는 경우가 많습니다)
# --------------------------------------------------------------------------
if (WIN32 AND MINGW)
  message(STATUS "Detected Windows (MinGW)")

  # vcpkg 설치 루트 경로 변수 (보통 사용자 계정 홈 경로)
  # set(VCPKG_ROOT "C:/Users/j2dol")
  set(VCPKG_ROOT $ENV{USERPROFILE})

  # vcpkg toolchain 설정 (MingW64 static)
  set(CMAKE_PREFIX_PATH    "${VCPKG_ROOT}/vcpkg/installed/x64-mingw-static")
  set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()


# --------------------------------------------------------------------------
# Linux (배포판 감지)
# --------------------------------------------------------------------------
if(UNIX AND NOT WIN32)
  message(STATUS "Detected Linux") # sudo apt install build-essential cmake
  if (EXISTS "/etc/os-release")
    file(READ "/etc/os-release" OS_RELEASE_CONTENT)
    if (OS_RELEASE_CONTENT MATCHES "ID=ubuntu")
      message(STATUS "Detected Ubuntu")
    elseif (OS_RELEASE_CONTENT MATCHES "ID=fedora")
      message(STATUS "Detected Fedora")
    elseif (OS_RELEASE_CONTENT MATCHES "ID=rhel")
      message(STATUS "Detected Red Hat Enterprise Linux")
    else()
      message(STATUS "Detected Other Linux")
    endif()
  endif()
endif()

# --------------------------------------------------------------------------
# Boost(JSON) - CONFIG 모드로 찾기 (CMake 3.30+ 대응)
#   - vcpkg/공식 Boost 모두 Boost::json 타깃을 제공합니다.
#   - 헤더온리(preprocessor/describe/mp11)는 COMPONENTS에 넣을 필요가 없습니다.
# --------------------------------------------------------------------------
find_package(Boost 1.75 CONFIG REQUIRED COMPONENTS json)

add_executable(demo
  main.cpp
  AutoDescribe.hpp
  JsonCodec.hpp
)

# 소스 디렉터리만 포함 (Boost 헤더 경로는 타깃 링크가 자동 전파)
target_include_directories(demo PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# 타깃 링크는 근대식 방식으로
target_link_libraries(demo PRIVATE
  Boost::json
)
