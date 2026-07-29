#!/usr/bin/env python3
"""
Luminus Engine Auto-Fix Script (v2 — for Cocos2d-x-based project)
"""
import argparse
import os
import re
import sys
from pathlib import Path
from typing import List, Dict, Optional, Tuple

try:
    import requests
except ImportError:
    print("ERROR: pip install requests", file=sys.stderr)
    sys.exit(1)


def fix_missing_include(match, log_text):
    header = match.group('header')
    file = match.group('file')
    p = Path(file)
    if not p.exists(): p = Path('.') / file
    if not p.exists(): return None
    content = p.read_text(encoding='utf-8', errors='replace')
    if f'#include "{header}"' in content or f'#include <{header}>' in content:
        return None
    lines = content.split('\n')
    insert_at = 0
    for i, line in enumerate(lines[:30]):
        if line.startswith('#include'): insert_at = i + 1
    is_system = header.startswith('<') or header.endswith('.h') and '/' not in header and '.' not in header.replace('.h','')
    fmt = f'#include <{header}>' if is_system else f'#include "{header}"'
    lines.insert(insert_at, fmt)
    return p, '\n'.join(lines)


def fix_missing_semicolon(match, log_text):
    file = match.group('file')
    line_no = int(match.group('line'))
    p = Path(file)
    if not p.exists(): p = Path('.') / file
    if not p.exists(): return None
    content = p.read_text(encoding='utf-8', errors='replace')
    lines = content.split('\n')
    if line_no - 1 >= len(lines): return None
    target = lines[line_no - 1].rstrip()
    if target.endswith(';') or target.endswith('{') or target.endswith('}') or target.endswith(':'): return None
    lines[line_no - 1] = target + ';'
    return p, '\n'.join(lines)


def fix_wrong_include_path(match, log_text):
    missing = match.group('header')
    bare = Path(missing).name
    candidates = []
    for p in Path('.').rglob(bare):
        if 'build' in p.parts or '_deps' in p.parts or '.git' in p.parts:
            continue
        candidates.append(p)
    if not candidates: return None
    candidates.sort(key=lambda p: len(p.parts))
    found = candidates[0]
    target_files = []
    for ext in ('*.cpp', '*.h', '*.hpp'):
        for p in Path('src').rglob(ext):
            target_files.append(p)
    for tf in target_files:
        if not tf.exists(): continue
        content = tf.read_text(encoding='utf-8', errors='replace')
        try:
            rel = os.path.relpath(str(found), start=str(tf.parent))
        except Exception: continue
        rel = rel.replace('\\', '/')
        old_patterns = [f'#include "{missing}"', f'#include <{missing}>', f'#include "{bare}"']
        new_line = f'#include "{rel}"'
        new_content = content
        changed = False
        for op in old_patterns:
            if op in new_content:
                new_content = new_content.replace(op, new_line)
                changed = True
        if changed: return tf, new_content
    return None


def fix_missing_paren(match, log_text):
    """Add missing closing parenthesis."""
    file = match.group('file')
    line_no = int(match.group('line'))
    p = Path(file)
    if not p.exists(): p = Path('.') / file
    if not p.exists(): return None
    content = p.read_text(encoding='utf-8', errors='replace')
    lines = content.split('\n')
    if line_no - 1 >= len(lines): return None
    target = lines[line_no - 1].rstrip()
    opens = target.count('(') - target.count(')')
    if opens <= 0: return None
    lines[line_no - 1] = target + ')' * opens + ';'
    return p, '\n'.join(lines)


KNOWN_FIXES = [
    (r"fatal error:\s*'(?P<header>[\w./]+\.h[ppx]?)'\s*file not found",
     "fix relative include path", fix_wrong_include_path),

    (r"fatal error:\s*(?P<header>[\w/.]+\.h[ppx]?)[:\s].*?(?P<file>[^\s]+\.(?:cpp|c|cc|h|hpp)):(?P<line>\d+)",
     "missing #include", fix_missing_include),

    (r"(?P<file>[^\s:]+\.(?:cpp|c|cc|h|hpp)):(?P<line>\d+):\d*:\s*error:\s*expected\s*[';\x27]",
     "missing semicolon", fix_missing_semicolon),

    (r"error:\s*expected\s*'\)'.*?(?P<file>[^\s:]+\.(?:cpp|c|cc|h|hpp)):(?P<line>\d+)",
     "missing closing parenthesis", fix_missing_paren),
]


def fetch_failed_logs(repo, run_id, token):
    headers = {'Authorization': f'token {token}', 'Accept': 'application/vnd.github+json'}
    api = f"https://api.github.com/repos/{repo}/actions/runs/{run_id}/jobs"
    r = requests.get(api, headers=headers, timeout=30)
    if r.status_code != 200:
        print(f"Failed to list jobs: {r.status_code} {r.text[:200]}", file=sys.stderr)
        return []
    jobs = r.json().get('jobs', [])
    print(f"Found {len(jobs)} jobs for run {run_id}")
    logs = []
    for job in jobs:
        if job.get('conclusion') != 'failure': continue
        job_id = job['id']
        log_url = f"https://api.github.com/repos/{repo}/actions/jobs/{job_id}/logs"
        lr = requests.get(log_url, headers=headers, timeout=60, allow_redirects=False)
        if lr.status_code == 302:
            actual_url = lr.headers.get('Location')
            if actual_url:
                lr = requests.get(actual_url, timeout=120)
        if lr.status_code != 200:
            print(f"  Could not fetch logs for {job['name']}: {lr.status_code}")
            continue
        logs.append({'job_name': job['name'], 'log_text': lr.text})
        print(f"  Got logs for {job['name']} ({len(lr.text)} bytes)")
    return logs


def analyze_and_fix(logs, repo_root):
    fixes = 0
    for entry in logs:
        text = entry['log_text']
        tail = text[-8000:] if len(text) > 8000 else text
        print(f"\nScanning logs from {entry['job_name']} ({len(text)} bytes)")
        error_lines = [ln for ln in tail.split('\n')
                       if 'error:' in ln.lower() or 'fatal error' in ln.lower()]
        if not error_lines:
            print("  No error lines detected.")
            continue
        print(f"  Found {len(error_lines)} error lines. Sample:")
        for ln in error_lines[:5]:
            print(f"     {ln.strip()[:200]}")
        os.chdir(repo_root)
        for pattern, desc, fixer in KNOWN_FIXES:
            for m in re.finditer(pattern, tail, re.MULTILINE):
                try:
                    result = fixer(m, tail)
                    if result is None: continue
                    filepath, new_content = result
                    filepath.write_text(new_content, encoding='utf-8')
                    print(f"  Applied fix: {desc} -> {filepath}")
                    fixes += 1
                except Exception as e:
                    print(f"  Fix failed ({desc}): {e}")
    return fixes


def main():
    ap = argparse.ArgumentParser(description="Luminus AutoFix v2")
    ap.add_argument('--repo', required=True)
    ap.add_argument('--run-id', type=int, required=True)
    ap.add_argument('--token', required=True)
    ap.add_argument('--max-iterations', type=int, default=1)
    args = ap.parse_args()

    repo_root = Path.cwd()
    print(f"Luminus AutoFix v2 starting")
    print(f"  Repo:   {args.repo}")
    print(f"  Run ID: {args.run_id}")
    print(f"  Root:   {repo_root}")

    logs = fetch_failed_logs(args.repo, args.run_id, args.token)
    if not logs:
        print("No failed logs to analyze.")
        return 0

    fixes = analyze_and_fix(logs, repo_root)
    print(f"\nTotal fixes applied: {fixes}")
    return 0 if fixes > 0 else 2


if __name__ == '__main__':
    sys.exit(main())
