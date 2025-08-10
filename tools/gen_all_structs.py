# tools/gen_all_structs.py
# 목적: 지정한 헤더(예: model.hpp) 안에 "정의된" struct/class만 찾아
#       BOOST_DESCRIBE_STRUCT(...) 라인을 생성해 describe_all.gen.hpp로 출력

import os
import sys
import argparse
from clang.cindex import Index, Config, CursorKind

def setup_libclang(path_from_cli: str | None):
    # 1) --libclang 인자 우선
    if path_from_cli and os.path.isfile(path_from_cli):
        Config.set_library_file(path_from_cli)
        return
    # 2) 환경변수 LIBCLANG_PATH
    env = os.environ.get("LIBCLANG_PATH")
    if env and os.path.isfile(env):
        Config.set_library_file(env)
        return
    # 3) 흔한 설치 경로 몇 곳 시도(Windows)
    candidates = [
        rf"C:\Users\{os.environ.get('USERNAME','')}\scoop\apps\llvm\current\bin\libclang.dll",
        r"C:\Program Files\LLVM\bin\libclang.dll",
        r"C:\Program Files (x86)\LLVM\bin\libclang.dll",
    ]
    for c in candidates:
        if os.path.isfile(c):
            Config.set_library_file(c)
            return
    # 다른 OS라면 시스템 기본 탐색에 맡김 (설치 안돼 있으면 이후에서 예외 발생)

def parse_args():
    ap = argparse.ArgumentParser()
    ap.add_argument("--libclang", default=None, help="libclang.dll 전체 경로 (선택)")
    ap.add_argument("header", help="스캔할 헤더 파일 경로 (예: model.hpp)")
    return ap.parse_args()

def norm(p: str) -> str:
    return os.path.normcase(os.path.normpath(os.path.abspath(p)))

def samefile(a: str, b: str) -> bool:
    try:
        return norm(a) == norm(b)
    except Exception:
        return False

def in_this_header(node, header_abs: str) -> bool:
    loc = node.location
    if not loc or not loc.file:
        return False
    return samefile(loc.file.name, header_abs)

def iter_nodes(root):
    # DFS
    stack = [root]
    while stack:
        n = stack.pop()
        yield n
        stack.extend(reversed(list(n.get_children())))

def is_target_record(c):
    if c.kind not in (CursorKind.STRUCT_DECL, CursorKind.CLASS_DECL):
        return False
    if not c.is_definition():
        return False
    if not c.spelling:
        return False
    # 시스템/런타임 내부 심볼 제외
    if c.spelling.startswith("_"):
        return False
    return True

def collect_field_names(c):
    names = []
    for f in c.get_children():
        if f.kind == CursorKind.FIELD_DECL and f.spelling:
            names.append(f.spelling)
    return names

def main():
    args = parse_args()
    setup_libclang(args.libclang)

    header_abs = norm(args.header)
    if not os.path.isfile(header_abs):
        print(f"// error: header not found: {args.header}", file=sys.stderr)
        sys.exit(2)

    index = Index.create()
    # include 경로 힌트: 헤더가 있는 폴더 우선 포함
    header_dir = os.path.dirname(header_abs)
    tu = index.parse(
        args.header,
        args=[
            "-x", "c++",
            "-std=c++20",
            f"-I{header_dir}",
            "-I.",  # 프로젝트 루트에서 실행될 수도 있으니
        ],
    )

    # 출력 시작
    print("#pragma once")
    print("#include <boost/describe.hpp>")

    seen = set()
    for n in iter_nodes(tu.cursor):
        if not is_target_record(n):
            continue
        if not in_this_header(n, header_abs):
            continue
        name = n.spelling
        if name in seen:
            continue
        fields = collect_field_names(n)
        if not fields:
            continue
        seen.add(name)
        joined = ", ".join(fields)
        print(f"BOOST_DESCRIBE_STRUCT({name}, (), ({joined}))")

if __name__ == "__main__":
    main()
