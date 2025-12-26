import subprocess
from pathlib import Path


FILES_TO_COMPILE = ["src/main.c", "src/fourier.c", "src/shapes.c", "src/ui.c"]
PATH_TO_RAYLIB_DLL = "libs/raylibdll.lib"
INCLUDE_PATH = "includes"
MAIN_EXE = "main.exe"
PATH_TO_MAIN = Path.cwd() / MAIN_EXE

def compile_c_files(c_files=FILES_TO_COMPILE, raylib_path=PATH_TO_RAYLIB_DLL):
    cmd = ["cl", "/W4", "/WX", f"/I{INCLUDE_PATH}"] + c_files + [raylib_path]
    subprocess.run(cmd, check=True)

def run_exe(main_path=PATH_TO_MAIN):
    subprocess.run([str(main_path)], check=True)
    
if __name__ == "__main__":
    compile_c_files()
    run_exe()