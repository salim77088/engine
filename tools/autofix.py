#!/usr/bin/env python3
"""
Luminus Engine - Auto-Fix Build Errors
======================================
Analyzes failed build logs and attempts to automatically fix common C++ errors.

Usage: python tools/autofix.py <logfile>
"""

import sys
import os
import re
import json
import subprocess
from pathlib import Path

# Patterns to detect and fix common build errors
FIX_PATTERNS = [
    # Missing include - add to file
    {
        "name": "missing_include",
        "regex": r"fatal error: ([\w/.]+): No such file or directory",
        "fix": "add_include"
    },
    # Wrong include path
    {
        "name": "wrong_include_path",
        "regex": r'fatal error: "(\w+\.h(?:pp)?)"',
        "fix": "fix_include_path"
    },
    # Missing semicolon
    {
        "name": "missing_semicolon",
        "regex": r"error: expected ';'.*?before '(\w+)'",
        "fix": "add_semicolon"
    },
    # Undefined reference - usually missing source file in CMakeLists
    {
        "name": "undefined_reference",
        "regex": r"undefined reference to `(\w+)'",
        "fix": "check_cmake_source"
    },
    # Raylib API renames
    {
        "name": "raylib_api_change",
        "regex": r"error: use of undeclared identifier '(\w+)'",
        "fix": "check_raylib_api"
    },
    # MSVC C2065 undeclared identifier
    {
        "name": "msvc_undeclared",
        "regex": r"error C2065: '(\w+)': undeclared identifier",
        "fix": "check_raylib_api"
    },
    # MSVC C4576 - non-standard cast
    {
        "name": "msvc_nonstandard_cast",
        "regex": r"warning C4576:.*followed by.*is non-standard",
        "fix": "fix_cast_syntax"
    },
    # Missing find_package
    {
        "name": "cmake_missing_package",
        "regex": r"Could NOT find (\w+)",
        "fix": "add_cmake_package"
    },
    # Member access of nullptr
    {
        "name": "nullptr_member_access",
        "regex": r"error: member access.*?within null pointer",
        "fix": "add_null_check"
    },
    # Type mismatch - basic
    {
        "name": "type_mismatch",
        "regex": r"error: cannot convert from '(\w+)' to '(\w+)'",
        "fix": "add_cast"
    },
]


def read_log(path):
    """Read failed build logs from file or stdin."""
    if path == "-":
        return sys.stdin.read()
    try:
        with open(path, 'r', errors='ignore') as f:
            return f.read()
    except FileNotFoundError:
        print(f"[autofix] Log file not found: {path}", file=sys.stderr)
        return ""


def find_source_files(src_dir="src"):
    """Recursively find all .cpp and .h files in src/."""
    files = []
    if not os.path.isdir(src_dir):
        return files
    for root, _, fnames in os.walk(src_dir):
        for fname in fnames:
            if fname.endswith(('.cpp', '.h', '.hpp', '.cc')):
                files.append(os.path.join(root, fname))
    return files


def add_include(match, content, file_path):
    """Add a missing #include directive."""
    header = match.group(1)
    # Common header -> file mapping
    header_map = {
        "string": "<string>",
        "vector": "<vector>",
        "memory": "<memory>",
        "iostream": "<iostream>",
        "fstream": "<fstream>",
        "sstream": "<sstream>",
        "cmath": "<cmath>",
        "cstdio": "<cstdio>",
        "cstdint": "<cstdint>",
        "functional": "<functional>",
        "unordered_map": "<unordered_map>",
        "algorithm": "<algorithm>",
    }
    if header in header_map:
        include_line = f"#include {header_map[header]}"
        if include_line not in content:
            # Add after first #include block
            lines = content.split('\n')
            for i, line in enumerate(lines):
                if line.startswith('#include'):
                    lines.insert(i + 1, include_line)
                    return True, '\n'.join(lines)
            lines.insert(0, include_line)
            return True, '\n'.join(lines)
    return False, content


def fix_cast_syntax(match, content, file_path):
    """Convert (Type){...} casts to Type{...} aggregate initialization (MSVC C4576)."""
    # Match patterns like (Color){255, 255, 255, 255}
    new_content = re.sub(
        r'\(([A-Za-z_]\w*)\)\s*\{',
        r'\1{',
        content
    )
    if new_content != content:
        return True, new_content
    return False, content


def check_raylib_api(match, content, file_path):
    """Check for raylib API renames and apply known mappings."""
    api_map = {
        'DrawCircle': '::DrawCircle',  # avoid shadowing
        'DrawText': '::DrawText',
        'DrawRectangle': '::DrawRectangle',
        'DrawTexture': '::DrawTexture',
    }
    identifier = match.group(1)
    if identifier in api_map:
        new_id = api_map[identifier]
        if new_id.lstrip(':') not in content.replace(new_id, ''):
            content = content.replace(f'({identifier}', f'({new_id}', 1)
            return True, content
    return False, content


def add_semicolon(match, content, file_path):
    """Add missing semicolon before the specified identifier."""
    identifier = match.group(1)
    # Find lines ending without semicolon before identifier
    lines = content.split('\n')
    fixed = False
    for i in range(len(lines) - 1):
        stripped = lines[i].rstrip()
        if stripped and not stripped.endswith((';', '{', '}', ':', ',')) and not stripped.startswith('#'):
            if identifier in lines[i + 1]:
                lines[i] = stripped + ';'
                fixed = True
                break
    return fixed, '\n'.join(lines)


def add_cmake_package(match, content, file_path):
    """Suggest adding missing CMake find_package."""
    pkg = match.group(1)
    print(f"[autofix] Missing CMake package detected: {pkg}")
    print(f"[autofix] Please install {pkg} or add to CMakeLists.txt")
    return False, content


def check_cmake_source(match, content, file_path):
    """Check if a missing source file should be added to CMakeLists.txt."""
    symbol = match.group(1)
    print(f"[autofix] Undefined reference: {symbol}")
    # Try to find which file defines this symbol
    result = subprocess.run(
        ['grep', '-rl', symbol, 'src/'],
        capture_output=True, text=True
    )
    if result.returncode == 0:
        found_file = result.stdout.strip().split('\n')[0]
        if found_file:
            cmake_path = "CMakeLists.txt"
            if os.path.exists(cmake_path):
                with open(cmake_path, 'r') as f:
                    cmake = f.read()
                rel_path = found_file.replace('\\', '/')
                if rel_path not in cmake:
                    # Add to LUMINUS_SOURCES
                    cmake = cmake.replace(
                        'set(LUMINUS_SOURCES',
                        f'set(LUMINUS_SOURCES'
                    )
                    cmake = re.sub(
                        r'(set\(LUMINUS_SOURCES\s*\n)(\s+src/main\.cpp)',
                        r'\1    src/main.cpp\n    ' + rel_path,
                        cmake
                    )
                    with open(cmake_path, 'w') as f:
                        f.write(cmake)
                    print(f"[autofix] Added {rel_path} to CMakeLists.txt")
                    return True, cmake
    return False, content


def add_null_check(match, content, file_path):
    """Add a basic null check before member access."""
    # Too complex to auto-fix safely; just report
    return False, content


def add_cast(match, content, file_path):
    """Add explicit cast for type mismatch."""
    return False, content


def fix_include_path(match, content, file_path):
    """Try to find correct include path for a header."""
    header = match.group(1)
    result = subprocess.run(
        ['find', 'src', '-name', header],
        capture_output=True, text=True
    )
    if result.returncode == 0:
        found_paths = result.stdout.strip().split('\n')
        for path in found_paths:
            if path:
                # Compute relative path
                dir_of_file = os.path.dirname(file_path)
                rel = os.path.relpath(os.path.dirname(path), dir_of_file)
                new_include = f'#include "{rel}/{header}"'
                old_include_pattern = f'#include "{header}"'
                if old_include_pattern in content:
                    content = content.replace(old_include_pattern, new_include, 1)
                    return True, content
    return False, content


FIX_FUNCTIONS = {
    'add_include': add_include,
    'fix_include_path': fix_include_path,
    'add_semicolon': add_semicolon,
    'check_cmake_source': check_cmake_source,
    'check_raylib_api': check_raylib_api,
    'fix_cast_syntax': fix_cast_syntax,
    'add_cmake_package': add_cmake_package,
    'add_null_check': add_null_check,
    'add_cast': add_cast,
}


def extract_error_files(log):
    """Extract file paths from error messages in the log."""
    file_pattern = re.compile(r'(/[\w/.]+\.(?:cpp|h|hpp|cc)):(\d+):')
    matches = file_pattern.findall(log)
    files = set()
    for path, _ in matches:
        if os.path.exists(path):
            files.add(path)
        # Also try relative paths
        rel = path.lstrip('/')
        if os.path.exists(rel):
            files.add(rel)
    return files


def main():
    if len(sys.argv) < 2:
        print("Usage: python tools/autofix.py <logfile>", file=sys.stderr)
        sys.exit(1)
    
    log_path = sys.argv[1]
    log = read_log(log_path)
    
    if not log:
        print("[autofix] Empty log, nothing to fix.")
        sys.exit(0)
    
    print(f"[autofix] Analyzing {len(log)} bytes of log...")
    
    # Collect all unique errors
    errors_found = []
    fixes_applied = 0
    
    for pattern in FIX_PATTERNS:
        matches = list(re.finditer(pattern["regex"], log))
        if matches:
            print(f"[autofix] Pattern '{pattern['name']}': {len(matches)} matches")
            errors_found.append((pattern, matches))
    
    if not errors_found:
        print("[autofix] No recognized error patterns found.")
        sys.exit(0)
    
    # Determine which source files to patch
    files_to_patch = extract_error_files(log)
    if not files_to_patch:
        # Fall back: patch all source files
        files_to_patch = set(find_source_files())
    
    print(f"[autofix] Files to check: {len(files_to_patch)}")
    
    # Apply fixes
    for pattern, matches in errors_found:
        fix_fn = FIX_FUNCTIONS.get(pattern["fix"])
        if not fix_fn:
            continue
        
        for file_path in files_to_patch:
            try:
                with open(file_path, 'r', errors='ignore') as f:
                    content = f.read()
                
                for match in matches:
                    fixed, content = fix_fn(match, content, file_path)
                    if fixed:
                        with open(file_path, 'w') as f:
                            f.write(content)
                        print(f"[autofix] Applied fix '{pattern['name']}' to {file_path}")
                        fixes_applied += 1
                        break  # Move to next file
            except Exception as e:
                print(f"[autofix] Error processing {file_path}: {e}", file=sys.stderr)
    
    # Also handle CMakeLists.txt for cmake-specific fixes
    for pattern, matches in errors_found:
        fix_fn = FIX_FUNCTIONS.get(pattern["fix"])
        if not fix_fn:
            continue
        if os.path.exists("CMakeLists.txt"):
            with open("CMakeLists.txt", 'r') as f:
                cmake_content = f.read()
            for match in matches:
                fixed, cmake_content = fix_fn(match, cmake_content, "CMakeLists.txt")
                if fixed:
                    with open("CMakeLists.txt", 'w') as f:
                        f.write(cmake_content)
                    print("[autofix] Applied fix to CMakeLists.txt")
                    fixes_applied += 1
    
    if fixes_applied > 0:
        print(f"[autofix] TOTAL: {fixes_applied} fixes applied.")
    else:
        print("[autofix] No fixes could be applied automatically.")
        print("[autofix] Manual intervention required.")
    
    sys.exit(0 if fixes_applied > 0 else 1)


if __name__ == "__main__":
    main()
