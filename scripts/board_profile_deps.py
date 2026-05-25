from pathlib import Path

Import("env")


project_dir = Path(env.subst("$PROJECT_DIR"))
project_headers = [
    str(path)
    for pattern in ("*.h", "*.hpp")
    for path in (project_dir / "include").rglob(pattern)
]

if project_headers:
    build_dir = env.subst("$BUILD_DIR")
    for source_name in ("main.cpp", "lvgl_port.cpp", "board_profile.cpp"):
        env.Depends(str(Path(build_dir) / "src" / f"{source_name}.o"),
                    project_headers)
