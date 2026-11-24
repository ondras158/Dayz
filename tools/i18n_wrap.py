#!/usr/bin/env python3
"""Jednorázový převod textů ve skriptech na překladové volání.

  "Vybrano " + amount.ToString() + " " + m_Currency   ->   OS_F("Vybrano %1 %2", amount.ToString(), m_Currency)
  "Nemas dost penez."                                   ->   OS_T("Nemas dost penez.")

Použití: python3 tools/i18n_wrap.py soubor.c [...]   (soubory přepíše)
Po převodu se seznam textů obnoví nástrojem tools/i18n_build.py.
"""
import re
import sys

SINKS = {"Notify", "SetText", "SetStatus", "SetLabel", "Label", "NotifyGroup", "RemoveMember",
         "NotifySteamId", "AddUserMark", "Chat", "ChatAll", "AddItem"}
SKIP_LINE = re.compile(r'Print\(|OS_T\(|OS_F\(|\bFind(Text|List|Button|Edit|AnyWidget|Widget)\(|ConfigGet|ConfigIs|'
                       r'CreateWidgets|GetInputByName|\.json"|\.layout"|OS_TAG|ShowWidget\(|Log\(|LoadFile|SaveFile|'
                       r'MakeDirectory|ExcludeContains|NameContains|BaseClasses|SetHealth|GetBoneIndex|^\s*//')
TOKEN = re.compile(r'"(?:\\.|[^"\\])*"|[A-Za-z_][A-Za-z0-9_]*|\d+\.?\d*|==|!=|&&|\|\||<=|>=|\S')
DELIM_SEG = {",", ";", "{", "}", "=", "return", "?", ":", "&&", "||", "<", ">", "<=", ">=", "-", "*", "/", "!"}
CMP = {"==", "!="}


def is_text_literal(lit, in_sink):
    body = lit[1:-1]
    if not re.search(r"[A-Za-z]{2}", body):
        return False
    if any(c in body for c in "\\$") or body.startswith("color") or body.startswith("#") or ".paa" in body:
        return False
    if "/" in body and " " not in body:
        return False
    if in_sink:
        return True
    if " " in body or re.search(r"[.:!?]\s*$", body):
        return True
    return False


def enclosing_call(tokens, idx):
    depth = 0
    for j in range(idx - 1, -1, -1):
        t = tokens[j]
        if t == ")":
            depth += 1
        elif t == "(":
            if depth == 0:
                return tokens[j - 1] if j > 0 else ""
            depth -= 1
    return ""


def segment(tokens, idx):
    """hranice výrazu (spojování přes +) kolem tokenu idx; vrací (start, end) včetně"""
    depth = 0
    start = idx
    j = idx - 1
    while j >= 0:
        t = tokens[j]
        if t in (")", "]"):
            depth += 1
        elif t in ("(", "["):
            if depth == 0:
                break
            depth -= 1
        elif depth == 0 and (t in DELIM_SEG or t in CMP):
            break
        start = j
        j -= 1
    depth = 0
    end = idx
    j = idx + 1
    while j < len(tokens):
        t = tokens[j]
        if t in ("(", "["):
            depth += 1
        elif t in (")", "]"):
            if depth == 0:
                break
            depth -= 1
        elif depth == 0 and (t in DELIM_SEG or t in CMP):
            break
        end = j
        j += 1
    before = tokens[start - 1] if start > 0 else ""
    after = tokens[end + 1] if end + 1 < len(tokens) else ""
    return start, end, before, after


def split_plus(tokens, start, end):
    """části oddělené + ; každá část = seznam indexů tokenů"""
    parts = [[]]
    depth = 0
    for k in range(start, end + 1):
        t = tokens[k]
        if t in ("(", "["):
            depth += 1
        elif t in (")", "]"):
            depth -= 1
        if t == "+" and depth == 0:
            parts.append([])
        else:
            parts[-1].append(k)
    return parts


def join_tokens(toks):
    out = ""
    prev = ""
    for t in toks:
        if out and re.match(r"[A-Za-z0-9_\"]", t) and re.match(r".*[A-Za-z0-9_\"]$", prev):
            out += " "
        out += t
        prev = t
    return out


def convert_line(line):
    if SKIP_LINE.search(line):
        return line
    # pracujeme na pozicích v řádku
    spans = [(m.start(), m.end(), m.group()) for m in TOKEN.finditer(line)]
    tokens = [s[2] for s in spans]
    done = set()
    edits = []
    for i, t in enumerate(tokens):
        if i in done or not t.startswith('"'):
            continue
        call = enclosing_call(tokens, i)
        start, end, before, after = segment(tokens, i)
        if before in CMP or after in CMP:
            continue
        parts = split_plus(tokens, start, end)
        if any(len(p) == 0 for p in parts):
            continue
        lits = [tokens[p[0]] for p in parts if len(p) == 1 and tokens[p[0]].startswith('"')]
        if not any(is_text_literal(l, call in SINKS) for l in lits):
            continue
        if not any(is_text_literal(l, False) for l in lits) and call not in SINKS:
            continue
        fmt = ""
        params = []
        for p in parts:
            if len(p) == 1 and tokens[p[0]].startswith('"'):
                fmt += tokens[p[0]][1:-1]
            else:
                params.append(line[spans[p[0]][0]:spans[p[-1]][1]].strip())
                fmt += "%" + str(len(params))
        if len(params) > 9:
            print("  !!! víc než 9 parametrů:", line.strip())
            continue
        if params:
            repl = 'OS_F("' + fmt + '", ' + ", ".join(params) + ")"
        else:
            repl = 'OS_T("' + fmt + '")'
        edits.append((spans[start][0], spans[end][1], repl))
        done.update(range(start, end + 1))
    for s, e, r in sorted(edits, reverse=True):
        line = line[:s] + r + line[e:]
    return line


def main():
    for path in sys.argv[1:]:
        src = open(path, encoding="utf-8").read().split("\n")
        out = [convert_line(l) for l in src]
        changed = sum(1 for a, b in zip(src, out) if a != b)
        open(path, "w", encoding="utf-8").write("\n".join(out))
        print(f"{path}: {changed} řádků")


if __name__ == "__main__":
    main()
