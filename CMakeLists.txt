# CMakeLists.txt
cmake_minimum_required(VERSION 3.30)
project(boost_json_autodescribe_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Boost FindPackage 모듈 제거 정책(경고 억제)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# -----------------------------
# Windows + MSVC (필요 시 유지)
# -----------------------------
if (WIN32 AND MSVC)
  set(VCPKG_ROOT $ENV{USERPROFILE})
  set(CMAKE_PREFIX_PATH    "${VCPKG_ROOT}/vcpkg/installed/x64-windows")
  set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# ---------------------------------------
# Windows + MinGW (절대 삭제하지 말 것)
# ---------------------------------------
if (WIN32 AND MINGW)
  message(STATUS "Detected Windows (MinGW)")
  set(VCPKG_ROOT $ENV{USERPROFILE})
  set(CMAKE_PREFIX_PATH    "${VCPKG_ROOT}/vcpkg/installed/x64-mingw-static")
  set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# -----------------------------
# Linux 정보 출력(선택적)
# -----------------------------
if(UNIX AND NOT WIN32)
  message(STATUS "Detected Linux")
  if (EXISTS "/etc/os-release")
    file(READ "/etc/os-release" OS_RELEASE_CONTENT)
    if (OS_RELEASE_CONTENT MATCHES "ID=ubuntu")
      message(STATUS "Detected Ubuntu")
    elseif (OS_RELEASE_CONTENT MATCHES "ID=fedora")
      message(STATUS "Detected Fedora")
    elseif (OS_RELEASE_CONTENT MATCHES "ID=rhel")
      message(STATUS "Detected RHEL")
    endif()
  endif()
endif()

# -----------------------------
# Python3 (gen 스크립트 실행용)
# -----------------------------
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# -----------------------------
# libclang 경로 (선택 입력)
# - 환경변수 LIBCLANG_PATH가 있으면 우선
# - 없으면 CMake 변수 LIBCLANG_DLL 로 지정 가능
# -----------------------------
set(_LIBCLANG_FROM_ENV "$ENV{LIBCLANG_PATH}")
if(NOT LIBCLANG_DLL AND _LIBCLANG_FROM_ENV)
  set(LIBCLANG_DLL "${_LIBCLANG_FROM_ENV}")
endif()
message(STATUS "LIBCLANG_DLL='${LIBCLANG_DLL}'")

# -----------------------------
# Boost::json (CONFIG 모드)
# -----------------------------
find_package(Boost 1.75 CONFIG REQUIRED COMPONENTS json)

# -----------------------------
# 코드 생성: describe_all.gen.hpp
#  - model.hpp에 '정의된' struct만 기술
#  - 생성물을 소스 디렉터리에 둠
# -----------------------------
set(DESCRIBE_OUT ${CMAKE_CURRENT_SOURCE_DIR}/describe_all.gen.hpp)

add_custom_command(
  OUTPUT  ${DESCRIBE_OUT}
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/gen_all_structs.py
          --libclang "${LIBCLANG_DLL}"
          ${CMAKE_SOURCE_DIR}/model.hpp
          > ${DESCRIBE_OUT}
  DEPENDS ${CMAKE_SOURCE_DIR}/model.hpp
          ${CMAKE_SOURCE_DIR}/tools/gen_all_structs.py
  VERBATIM
)

add_custom_target(gen_describe ALL
  DEPENDS ${DESCRIBE_OUT}
)

# -----------------------------
# 실행 파일
# -----------------------------
add_executable(demo
  main.cpp
  model.hpp
  AutoDescribe.hpp
  JsonCodec.hpp
  ${DESCRIBE_OUT}          # IDE 노출용(실제 빌드에는 영향 없음)
)

add_dependencies(demo gen_describe)

target_include_directories(demo PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(demo PRIVATE
  Boost::json
)

# 경고 옵션(원하시면 조정)
if (MSVC)
  target_compile_options(demo PRIVATE /W4)
else()
  target_compile_options(demo PRIVATE -Wall -Wextra -Wpedantic -Wno-variadic-macros)
endif()
