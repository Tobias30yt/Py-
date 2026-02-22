#!/usr/bin/env python3
"""
Windows setup helper for C++ toolchain used by this project.

Usage:
  python tools/setup_cpp_env.py --check
  python tools/setup_cpp_env.py --install
"""

from __future__ import annotations

import argparse
import os
import pathlib
import shutil
import subprocess
import sys
from dataclasses import dataclass


@dataclass
class ToolStatus:
    name: str
    found: bool
    path: str | None = None


def find_tool(name: str) -> ToolStatus:
    path = shutil.which(name)
    if path is None and name == "cmake":
        fallback = pathlib.Path(r"C:\Program Files\CMake\bin\cmake.exe")
        if fallback.exists():
            path = str(fallback)
    return ToolStatus(name=name, found=path is not None, path=path)


def run_command(cmd: list[str], check: bool = True) -> int:
    print("> " + " ".join(cmd))
    proc = subprocess.run(cmd, check=False)
    if check and proc.returncode != 0:
        raise RuntimeError(f"Command failed ({proc.returncode}): {' '.join(cmd)}")
    return proc.returncode


def print_status(tools: list[ToolStatus]) -> None:
    for t in tools:
        if t.found:
            print(f"[OK] {t.name}: {t.path}")
        else:
            print(f"[MISS] {t.name}")


def check_environment() -> int:
    tools = [
        find_tool("winget"),
        find_tool("cmake"),
        find_tool("cl"),
        find_tool("g++"),
        find_tool("clang++"),
    ]
    print_status(tools)

    has_compiler = any(t.found for t in tools if t.name in {"cl", "g++", "clang++"})
    has_cmake = any(t.found for t in tools if t.name == "cmake")

    if has_cmake and has_compiler:
        print("\nEnvironment looks good. You can build now.")
        return 0

    print("\nEnvironment is incomplete.")
    if not has_cmake:
        print("- Missing: cmake")
    if not has_compiler:
        print("- Missing: C++ compiler (cl, g++, or clang++)")
    print("\nRun with --install to install required tools automatically (Windows + winget).")
    return 1


def install_with_winget() -> None:
    if os.name != "nt":
        raise RuntimeError("Automatic install is currently supported only on Windows.")

    if shutil.which("winget") is None:
        raise RuntimeError("winget not found. Install App Installer from Microsoft Store first.")

    print("Installing CMake...")
    run_command(
        [
            "winget",
            "install",
            "--id",
            "Kitware.CMake",
            "-e",
            "--accept-package-agreements",
            "--accept-source-agreements",
        ]
    )

    print("Installing Visual Studio Build Tools (C++ workload)...")
    rc = run_command(
        [
            "winget",
            "install",
            "--id",
            "Microsoft.VisualStudio.2022.BuildTools",
            "-e",
            "--override",
            "--quiet --wait --norestart --nocache "
            "--add Microsoft.VisualStudio.Workload.VCTools "
            "--includeRecommended",
            "--accept-package-agreements",
            "--accept-source-agreements",
        ],
        check=False,
    )
    if rc != 0:
        print(
            "BuildTools install/update failed via winget. "
            "Trying to modify existing Visual Studio installation..."
        )
        repair_existing_visual_studio()
        return

    print(
        "\nInstall commands finished.\n"
        "Open a NEW terminal and run:\n"
        "  python tools/setup_cpp_env.py --check"
    )


def repair_existing_visual_studio() -> None:
    installer = pathlib.Path(
        r"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe"
    )
    if not installer.exists():
        raise RuntimeError(
            "vs_installer.exe not found. Open Visual Studio Installer and add "
            "'Desktop development with C++' manually."
        )

    candidates = [
        pathlib.Path(r"C:\Program Files\Microsoft Visual Studio\2022\Community"),
        pathlib.Path(r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"),
    ]

    install_path = next((p for p in candidates if p.exists()), None)
    if install_path is None:
        raise RuntimeError(
            "No existing VS 2022 installation found for automatic repair. "
            "Install Build Tools manually from Visual Studio Installer."
        )

    run_command(
        [
            str(installer),
            "modify",
            "--installPath",
            str(install_path),
            "--add",
            "Microsoft.VisualStudio.Workload.VCTools",
            "--includeRecommended",
            "--passive",
            "--norestart",
        ]
    )
    print(
        "\nVisual Studio modification finished.\n"
        "Open a NEW terminal and run:\n"
        "  python tools/setup_cpp_env.py --check"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description="Setup C++ build tools for py++.")
    parser.add_argument("--check", action="store_true", help="Only check installed tools.")
    parser.add_argument(
        "--install",
        action="store_true",
        help="Install CMake + Visual Studio C++ Build Tools via winget.",
    )
    args = parser.parse_args()

    if not args.check and not args.install:
        parser.print_help()
        return 1

    try:
        if args.check:
            return check_environment()
        if args.install:
            install_with_winget()
            return 0
    except Exception as exc:  # pragma: no cover
        print(f"ERROR: {exc}")
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
