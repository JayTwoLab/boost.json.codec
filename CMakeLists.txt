cmake_minimum_required(VERSION 3.30)
project(boost_json_autodescribe_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# ---- vcpkg (Keep the requested MinGW block as is) ----
if (WIN32 AND MSVC)
  # Add MSVC-specific settings here if needed
endif()

if (WIN32 AND MINGW)
  message(STATUS "Detected Windows (MinGW)")
  set(VCPKG_ROOT $ENV{USERPROFILE})
  set(CMAKE_PREFIX_PATH    "${VCPKG_ROOT}/vcpkg/installed/x64-mingw-static")
  set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# (Optional) Detect Linux
if(UNIX AND NOT WIN32)
  message(STATUS "Detected Linux")
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

# ---- Python / libclang ----
find_package(Python3 COMPONENTS Interpreter REQUIRED)

if (WIN32 AND NOT DEFINED LIBCLANG_DLL)
  if (EXISTS "$ENV{USERPROFILE}/scoop/apps/llvm/current/bin/libclang.dll")
    set(LIBCLANG_DLL "$ENV{USERPROFILE}/scoop/apps/llvm/current/bin/libclang.dll")
  elseif (EXISTS "C:/Program Files/LLVM/bin/libclang.dll")
    set(LIBCLANG_DLL "C:/Program Files/LLVM/bin/libclang.dll")
  endif()
endif()

if (WIN32 AND NOT LIBCLANG_DLL)
  message(FATAL_ERROR "Please specify the LIBCLANG_DLL path. Example: -DLIBCLANG_DLL=C:/Users/<you>/scoop/apps/llvm/current/bin/libclang.dll")
endif()

# ---- Boost(JSON) ----
find_package(Boost 1.75 CONFIG REQUIRED COMPONENTS json)

# ---- Paths / Files ----
set(SRC_DIR      "${CMAKE_CURRENT_SOURCE_DIR}")
set(TOOL_DIR     "${SRC_DIR}/tools")
set(MODEL_HEADER "${SRC_DIR}/model.hpp")
# Create in the source tree as requested
set(GEN_HEADER   "${SRC_DIR}/describe_all.gen.hpp")

# Mark generated files
set_source_files_properties("${GEN_HEADER}" PROPERTIES GENERATED TRUE)

# ---- Generation rule: define OUTPUT only once ----
add_custom_command(
  OUTPUT "${GEN_HEADER}"
  COMMAND ${CMAKE_COMMAND} -E env PYTHONUTF8=1
          "${Python3_EXECUTABLE}" "${TOOL_DIR}/gen_all_structs.py"
            --libclang "${LIBCLANG_DLL}"
            --out "${GEN_HEADER}"
            --std c++20
            -I "${SRC_DIR}"
            "${MODEL_HEADER}"
  DEPENDS "${MODEL_HEADER}" "${TOOL_DIR}/gen_all_structs.py"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "Generating describe_all.gen.hpp from model.hpp"
  VERBATIM
)

# This target depends only on the generated file result
add_custom_target(gen_describe DEPENDS "${GEN_HEADER}")

# ---- Executable ----
add_executable(demo
  "${SRC_DIR}/main.cpp"
  "${SRC_DIR}/AutoDescribe.hpp"
  "${SRC_DIR}/JsonCodec.hpp"
  "${SRC_DIR}/model.hpp"
  "${GEN_HEADER}"
)

add_dependencies(demo gen_describe)
target_link_libraries(demo PRIVATE Boost::json)

if (MSVC)
  target_compile_options(demo PRIVATE /W4 /permissive-)
else()
  target_compile_options(demo PRIVATE -Wall -Wextra -Wpedantic -Wno-variadic-macros)
endif()

# ---- Expose IDE-only files ----
set(IDE_ONLY_FILES
  "${TOOL_DIR}/gen_all_structs.py"
  "${TOOL_DIR}/gen_fields.py"
  "${TOOL_DIR}/make_describe.cmd"
  "${TOOL_DIR}/make_describe.ps1"
  "${TOOL_DIR}/make_describe.sh"
)
target_sources(demo PRIVATE ${IDE_ONLY_FILES})
foreach(f IN LISTS IDE_ONLY_FILES)
  if (EXISTS "${f}")
    set_source_files_properties("${f}" PROPERTIES HEADER_FILE_ONLY TRUE)
  endif()
endforeach()
