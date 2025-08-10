# tools/gen_fields.py
import sys, os, argparse
from clang.cindex import Index, Config, CursorKind

def setup_libclang(path_from_cli: str | None):
    if path_from_cli and os.path.isfile(path_from_cli):
        Config.set_library_file(path_from_cli); return
    env = os.environ.get("LIBCLANG_PATH")
    if env and os.path.isfile(env):
        Config.set_library_file(env); return
    for c in [
        rf"C:\Users\{os.environ.get('USERNAME','')}\scoop\apps\llvm\current\bin\libclang.dll",
        r"C:\Program Files\LLVM\bin\libclang.dll",
        r"C:\Program Files (x86)\LLVM\bin\libclang.dll",
    ]:
        if os.path.isfile(c):
            Config.set_library_file(c); return

def parse_args():
    ap = argparse.ArgumentParser()
    ap.add_argument("--libclang", default=None)
    ap.add_argument("--all", action="store_true",
                    help="Scan the entire header and describe all struct/class definitions in that file")
    ap.add_argument("header")
    ap.add_argument("struct", nargs="?", help="Struct name when --all is not used")
    return ap.parse_args()

def norm(p: str) -> str:
    return os.path.normcase(os.path.normpath(os.path.abspath(p)))

def samefile(a: str, b: str) -> bool:
    try:
        return norm(a) == norm(b)
    except Exception:
        return False

def iter_structs(tu):
    def walk(n):
        for c in n.get_children():
            yield c
            yield from walk(c)
    for n in walk(tu.cursor):
        if n.kind in (CursorKind.STRUCT_DECL, CursorKind.CLASS_DECL) and n.is_definition() and n.spelling:
            yield n

def in_this_header(node, header_abs: str) -> bool:
    loc = node.location
    if not loc or not loc.file:
        return False
    return samefile(loc.file.name, header_abs)

def collect_fields(node):
    names=[]
    for f in node.get_children():
        if f.kind == CursorKind.FIELD_DECL and f.spelling:
            names.append(f.spelling)
    return names

def should_skip_name(name: str) -> bool:
    # Exclude system/runtime internal symbols
    return name.startswith("_")

def main():
    args = parse_args()
    setup_libclang(args.libclang)

    header_abs = norm(args.header)
    index = Index.create()
    tu = index.parse(args.header, args=['-x','c++','-std=c++20','-I.'])

    print("#pragma once")
    print("#include <boost/describe.hpp>")

    if args.all:
        seen = set()
        for s in iter_structs(tu):
            if not in_this_header(s, header_abs):
                continue
            if not s.spelling or should_skip_name(s.spelling):
                continue
            if s.spelling in seen:
                continue
            seen.add(s.spelling)
            fields = collect_fields(s)
            if not fields:
                continue
            joined = ", ".join(fields)
            print(f"BOOST_DESCRIBE_STRUCT({s.spelling}, (), ({joined}))")
    else:
        if not args.struct:
            print("// error: struct name missing", file=sys.stderr)
            sys.exit(2)
        target = None
        for s in iter_structs(tu):
            if s.spelling == args.struct and in_this_header(s, header_abs):
                target = s; break
        if not target:
            print(f"// error: struct {args.struct} not found in {args.header}", file=sys.stderr)
            sys.exit(2)
        fields = collect_fields(target)
        joined = ", ".join(fields)
        print(f"BOOST_DESCRIBE_STRUCT({args.struct}, (), ({joined}))")

if __name__ == "__main__":
    main()
