# tools/gen_fields.py
# 사용: python gen_fields.py [--libclang "C:/.../libclang.dll"] <header.hpp> <StructName> [--ns My::NS]
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
    ap.add_argument("--libclang", default=None, help="full path to libclang.dll")
    ap.add_argument("header")
    ap.add_argument("struct")
    ap.add_argument("--ns", default=None)
    return ap.parse_args()

def guess_init(field):
    tokens = list(field.get_tokens())
    eq_pos = next((i for i,t in enumerate(tokens) if t.spelling == '='), None)
    if eq_pos is not None:
        init_text = ''.join(t.spelling for t in tokens[eq_pos+1:]).strip()
        if init_text.endswith(';'):
            init_text = init_text[:-1].strip()
        return init_text
    t = field.type.spelling
    if t.endswith('std::string') or t == 'std::string': return '""'
    if t.startswith('std::vector') or t.startswith('std::map') or t.startswith('std::unordered_'): return '{}'
    if t in ('int','long','long long','unsigned','unsigned int','size_t','double','float','bool'):
        return 'false' if t == 'bool' else '0'
    return '{}'

def find_struct(tu, name, ns=None):
    def qname(node):
        parts=[]; cur=node
        from clang.cindex import CursorKind as CK
        while cur and cur.kind != CK.TRANSLATION_UNIT:
            if cur.spelling: parts.append(cur.spelling)
            cur = cur.semantic_parent
        return '::'.join(reversed(parts))
    from clang.cindex import CursorKind as CK
    for c in tu.cursor.get_children():
        stack=[c]
        while stack:
            n=stack.pop()
            if n.kind in (CK.STRUCT_DECL, CK.CLASS_DECL) and n.spelling == name:
                fq = qname(n)
                if ns is None or fq.startswith(ns+'::') or fq == ns:
                    return n
            stack.extend(list(n.get_children()))
    return None

def main():
    args = parse_args()
    setup_libclang(args.libclang)
    index = Index.create()
    tu = index.parse(args.header, args=['-x','c++','-std=c++20','-I.'])

    node = find_struct(tu, args.struct, args.ns)
    if not node:
        print(f"error: struct {args.struct} not found")
        sys.exit(2)

    fields = []
    from clang.cindex import CursorKind as CK
    for f in node.get_children():
        if f.kind == CK.FIELD_DECL:
            fields.append((f.spelling, f.type.spelling, guess_init(f)))

    macro = args.struct.upper() + "_FIELDS"

    # 한 줄로 출력: 백슬래시 라인연속 제거
    elems = "".join([f"(({n}, {t}, {iinit}))" for (n,t,iinit) in fields])
    print("#pragma once")
    print(f"#define {macro} {elems}")
    print(f"ADH_DESCRIBE_EXISTING({args.struct}, {macro})")

if __name__ == "__main__":
    main()
