cmake_minimum_required(VERSION 3.30)
project(boost_json_autodescribe_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake 3.30의 Boost 모듈 정책
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# vcpkg 툴체인은 CMake 최초 Configure 시 지정 권장:
#   -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
#   -DVCPKG_TARGET_TRIPLET=x64-mingw-static

# Boost::json (CONFIG)
find_package(Boost 1.75 CONFIG REQUIRED COMPONENTS json)

# Python3 (코드 생성용)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# scoop/LLVM 경로 자동 감지 → libclang.dll
set(_default_libclang "")
if (WIN32)
  set(_candidates
    "$ENV{USERPROFILE}/scoop/apps/llvm/current/bin/libclang.dll"
    "C:/Program Files/LLVM/bin/libclang.dll"
    "C:/Program Files (x86)/LLVM/bin/libclang.dll"
  )
  foreach(p IN LISTS _candidates)
    file(TO_CMAKE_PATH "${p}" _p)
    if (EXISTS "${_p}")
      set(_default_libclang "${_p}")
      break()
    endif()
  endforeach()
endif()
set(LIBCLANG_DLL "${_default_libclang}" CACHE FILEPATH "Path to libclang.dll (scoop/LLVM)")

# 코드 생성: 각 구조체별 .gen.hpp (기존 struct를 describe만 추가)
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/Project.gen.hpp
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/gen_fields.py
          --libclang "${LIBCLANG_DLL}"
          ${CMAKE_SOURCE_DIR}/model.hpp Project
          > ${CMAKE_BINARY_DIR}/Project.gen.hpp
  DEPENDS ${CMAKE_SOURCE_DIR}/model.hpp ${CMAKE_SOURCE_DIR}/tools/gen_fields.py
  VERBATIM
)
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/Profile.gen.hpp
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/gen_fields.py
          --libclang "${LIBCLANG_DLL}"
          ${CMAKE_SOURCE_DIR}/model.hpp Profile
          > ${CMAKE_BINARY_DIR}/Profile.gen.hpp
  DEPENDS ${CMAKE_SOURCE_DIR}/model.hpp ${CMAKE_SOURCE_DIR}/tools/gen_fields.py
  VERBATIM
)
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/User.gen.hpp
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/gen_fields.py
          --libclang "${LIBCLANG_DLL}"
          ${CMAKE_SOURCE_DIR}/model.hpp User
          > ${CMAKE_BINARY_DIR}/User.gen.hpp
  DEPENDS ${CMAKE_SOURCE_DIR}/model.hpp ${CMAKE_SOURCE_DIR}/tools/gen_fields.py
  VERBATIM
)

# 선택: 생성만 별도로 실행하고 싶을 때 사용할 집합 타깃
add_custom_target(gen_fields ALL
  DEPENDS
    ${CMAKE_BINARY_DIR}/Project.gen.hpp
    ${CMAKE_BINARY_DIR}/Profile.gen.hpp
    ${CMAKE_BINARY_DIR}/User.gen.hpp
)

# 실행 파일: 생성된 헤더 포함 → 자동 의존성 처리
add_executable(demo
  main.cpp
  AutoDescribe.hpp
  JsonCodec.hpp
  model.hpp
  ${CMAKE_BINARY_DIR}/Project.gen.hpp
  ${CMAKE_BINARY_DIR}/Profile.gen.hpp
  ${CMAKE_BINARY_DIR}/User.gen.hpp
)

target_include_directories(demo PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)

target_link_libraries(demo PRIVATE Boost::json)

if (MINGW)
  target_compile_options(demo PRIVATE -Wall -Wextra -Wpedantic -Wno-variadic-macros)
  # 정적 링크를 원하시면 아래를 해제
  # target_link_options(demo PRIVATE -static-libstdc++ -static-libgcc)
endif()
