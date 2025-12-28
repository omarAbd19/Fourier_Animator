#!/usr/bin/env python3
"""
Build system for Fourier Animator project.

Usage:
    python build.py              Build and run (default)
    python build.py --run        Build and run
    python build.py --build      Build only
    python build.py --clean      Clean build artifacts
    python build.py --rebuild    Clean, build, and run
    python build.py --release    Build with optimizations
"""

import argparse
import shutil
import subprocess
import sys
import time
from pathlib import Path
from dataclasses import dataclass
from typing import List


# =============================================================================
# Configuration
# =============================================================================

@dataclass
class BuildConfig:
    """Build configuration settings."""
    
    # Directories
    src_dir: Path = Path("src")
    include_dir: Path = Path("includes")
    lib_dir: Path = Path("libs")
    build_dir: Path = Path("build")
    
    # Output
    exe_name: str = "Fourier.exe"
    
    # Compiler settings
    compiler: str = "cl"
    
    # Source files
    sources: tuple = (
        "main.c",
        "fourier.c",
        "shapes.c",
        "ui.c",
    )
    
    # Libraries
    raylib_lib: str = "raylibdll.lib"
    raylib_dll: str = "raylib.dll"


# =============================================================================
# Terminal Colors (Windows compatible)
# =============================================================================

class Colors:
    """ANSI color codes for terminal output."""
    
    RESET = "\033[0m"
    BOLD = "\033[1m"
    RED = "\033[91m"
    GREEN = "\033[92m"
    YELLOW = "\033[93m"
    BLUE = "\033[94m"
    CYAN = "\033[96m"
    
    @staticmethod
    def enable():
        """Enable ANSI colors on Windows."""
        if sys.platform == "win32":
            import os
            os.system("")  # Enables ANSI escape sequences


def print_header(msg: str) -> None:
    """Print a formatted header message."""
    print(f"\n{Colors.BOLD}{Colors.BLUE}==> {msg}{Colors.RESET}")


def print_success(msg: str) -> None:
    """Print a success message."""
    print(f"{Colors.GREEN}✓ {msg}{Colors.RESET}")


def print_error(msg: str) -> None:
    """Print an error message."""
    print(f"{Colors.RED}✗ {msg}{Colors.RESET}")


def print_info(msg: str) -> None:
    """Print an info message."""
    print(f"{Colors.CYAN}  {msg}{Colors.RESET}")


# =============================================================================
# Build System
# =============================================================================

class Builder:
    """Handles compilation and build management."""
    
    def __init__(self, config: BuildConfig, release: bool = False):
        self.config = config
        self.release = release
    
    def _get_source_files(self) -> List[str]:
        """Get full paths to source files."""
        return [str(self.config.src_dir / src) for src in self.config.sources]
    
    def _get_compiler_flags(self) -> List[str]:
        """Get compiler flags based on build type."""
        flags = [
            self.config.compiler,
            f"/I{self.config.include_dir}",
            f"/Fo{self.config.build_dir}\\",
            f"/Fe{self.config.build_dir}\\{self.config.exe_name}",
            "/nologo",  # Suppress banner
        ]
        
        if self.release:
            flags.extend(["/O2", "/DNDEBUG"])  # Optimize for speed
        else:
            flags.extend(["/W4", "/WX", "/Zi"])  # Warnings + debug info
        
        return flags
    
    def _copy_dll(self) -> None:
        """Copy raylib DLL to build directory."""
        src = self.config.lib_dir / self.config.raylib_dll
        dst = self.config.build_dir / self.config.raylib_dll
        
        if src.exists() and (not dst.exists() or src.stat().st_mtime > dst.stat().st_mtime):
            shutil.copy2(src, dst)
            print_info(f"Copied {self.config.raylib_dll}")
    
    def clean(self) -> bool:
        """Remove build directory."""
        print_header("Cleaning")
        
        if self.config.build_dir.exists():
            shutil.rmtree(self.config.build_dir)
            print_success(f"Removed {self.config.build_dir}/")
        else:
            print_info("Nothing to clean")
        
        return True
    
    def build(self) -> bool:
        """Compile the project."""
        build_type = "Release" if self.release else "Debug"
        print_header(f"Building ({build_type})")
        
        # Create build directory
        self.config.build_dir.mkdir(exist_ok=True)
        
        # Copy DLL
        self._copy_dll()
        
        # Build command
        cmd = (
            self._get_compiler_flags() +
            self._get_source_files() +
            [str(self.config.lib_dir / self.config.raylib_lib)]
        )
        
        print_info(f"Compiling {len(self.config.sources)} source files...")
        
        start_time = time.perf_counter()
        
        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                check=True
            )
            
            elapsed = time.perf_counter() - start_time
            print_success(f"Build completed in {elapsed:.2f}s")
            
            exe_path = self.config.build_dir / self.config.exe_name
            size_kb = exe_path.stat().st_size / 1024
            print_info(f"Output: {exe_path} ({size_kb:.1f} KB)")
            
            return True
            
        except subprocess.CalledProcessError as e:
            print_error("Build failed!")
            if e.stdout:
                print(e.stdout)
            if e.stderr:
                print(e.stderr)
            return False
    
    def run(self) -> bool:
        """Run the compiled executable."""
        print_header("Running")
        
        exe_path = self.config.build_dir / self.config.exe_name
        
        if not exe_path.exists():
            print_error(f"Executable not found: {exe_path}")
            return False
        
        print_info(f"Launching {self.config.exe_name}...")
        print()
        
        try:
            subprocess.run([str(exe_path)], check=True)
            return True
        except subprocess.CalledProcessError:
            print_error("Application exited with error")
            return False
        except KeyboardInterrupt:
            print_info("Interrupted by user")
            return True


# =============================================================================
# CLI
# =============================================================================

def parse_args() -> argparse.Namespace:
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description="Build system for Fourier Animator",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python build.py              Build and run (debug)
  python build.py --release    Build and run (optimized)
  python build.py --build      Build only
  python build.py --clean      Clean build artifacts
  python build.py --rebuild    Clean + build + run
        """
    )
    
    actions = parser.add_mutually_exclusive_group()
    actions.add_argument(
        "--run", "-r",
        action="store_true",
        help="Build and run (default)"
    )
    actions.add_argument(
        "--build", "-b",
        action="store_true",
        help="Build only, don't run"
    )
    actions.add_argument(
        "--clean", "-c",
        action="store_true",
        help="Clean build artifacts"
    )
    actions.add_argument(
        "--rebuild",
        action="store_true",
        help="Clean, build, and run"
    )
    
    parser.add_argument(
        "--release",
        action="store_true",
        help="Build with optimizations"
    )
    
    return parser.parse_args()


def main() -> int:
    """Main entry point."""
    Colors.enable()
    args = parse_args()
    
    config = BuildConfig()
    builder = Builder(config, release=args.release)
    
    # Handle actions
    if args.clean:
        return 0 if builder.clean() else 1
    
    if args.rebuild:
        builder.clean()
        if not builder.build():
            return 1
        return 0 if builder.run() else 1
    
    if args.build:
        return 0 if builder.build() else 1
    
    # Default: build and run
    if not builder.build():
        return 1
    return 0 if builder.run() else 1


if __name__ == "__main__":
    sys.exit(main())
