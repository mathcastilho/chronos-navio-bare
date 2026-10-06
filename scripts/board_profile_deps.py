import re
import shutil
from pathlib import Path


Import("env")


def remove_c_function(source, function_name):
    signature = re.compile(
        rf"(?m)^[^\n]*\b{re.escape(function_name)}\s*\([^;\n]*\)\s*\n\{{"
    )
    match = signature.search(source)
    if not match:
        return source

    opening_brace = source.index("{", match.start())
    depth = 0
    for index in range(opening_brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[:match.start()] + source[index + 1 :]

    raise RuntimeError(f"Unbalanced function body for {function_name} in navio_ui")


def remove_navigation_status_indicators(navio_ui_dir):
    for view_name in ("nav_default", "nav_hor", "nav_atom", "nav_stick"):
        view_dir = navio_ui_dir / "components" / "views" / view_name
        generated_source_path = view_dir / f"{view_name}_gen.c"
        generated_source = generated_source_path.read_text(encoding="utf-8")

        lines = generated_source.splitlines(keepends=True)
        blocks = []
        line_index = 0
        while line_index < len(lines):
            line = lines[line_index]
            if not re.match(
                r"\s*lv_obj_t \* wd_image_[A-Za-z0-9_]+ = wd_image_create\(", line
            ):
                blocks.append(line)
                line_index += 1
                continue

            end_index = line_index + 1
            while end_index < len(lines) and lines[end_index].strip():
                end_index += 1
            block = "".join(lines[line_index:end_index])
            if not any(
                icon in block for icon in ("icon_connection", "icon_navigation")
            ):
                blocks.append(block)
            line_index = end_index

        pruned_source = "".join(blocks)
        if "icon_connection" in pruned_source or "icon_navigation" in pruned_source:
            raise RuntimeError(
                f"Could not remove status indicators from {generated_source_path}"
            )
        generated_source_path.write_text(pruned_source, encoding="utf-8")

        xml_path = view_dir / f"{view_name}.xml"
        xml_source = xml_path.read_text(encoding="utf-8")
        widget_pattern = r"<wd_image\b.*?</wd_image>"
        xml_source = re.sub(
            widget_pattern,
            lambda match: ""
            if any(
                f'src="{icon}"' in match.group(0)
                for icon in ("icon_connection", "icon_navigation")
            )
            else match.group(0),
            xml_source,
            flags=re.DOTALL,
        )
        if "icon_connection" in xml_source or "icon_navigation" in xml_source:
            raise RuntimeError(f"Could not remove status indicators from {xml_path}")
        xml_path.write_text(xml_source, encoding="utf-8")


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

navio_screens = (
    project_dir
    / ".pio"
    / "libdeps"
    / env.subst("$PIOENV")
    / "navio_ui"
    / "custom"
    / "screens.c"
)
if not navio_screens.is_file():
    raise RuntimeError(f"Could not find navio_ui screens source: {navio_screens}")

source = navio_screens.read_text(encoding="utf-8")
for function_name in (
    "screen_settings",
    "screen_about",
    "get_list_from_wd",
    "on_settings_status",
    "scroll_y_end_listener_cb",
    "scroll_x_end_listener_cb",
    "settings_timer_cb",
    "settings_event_cb",
):
    source = remove_c_function(source, function_name)

for declaration in (
    r"static void lv_obj_animate_y\(lv_obj_t \*obj, int32_t start, int32_t end, int32_t duration\);\n",
    r"static void scroll_y_end_listener_cb\(lv_event_t \*e\);\n",
    r"static void scroll_x_end_listener_cb\(lv_event_t \*e\);\n",
    r"static void settings_timer_cb\(lv_timer_t \* timer\);\n",
    r"static void settings_event_cb\(lv_event_t \* e\);\n",
    r"static lv_timer_t \* settings_timer;\n",
):
    source = re.sub(declaration, "", source)

settings_route = ".left  = { screen_settings, LV_SCR_LOAD_ANIM_OVER_LEFT },"
about_route = ".right = { screen_about, LV_SCR_LOAD_ANIM_OVER_RIGHT },"
disabled_settings_route = ".left  = { NULL, 0 },"
disabled_about_route = ".right = { NULL, 0 },"
if settings_route in source:
    source = source.replace(settings_route, disabled_settings_route, 1)
elif disabled_settings_route not in source:
    raise RuntimeError("Could not find navigation route to Settings")
if about_route in source:
    source = source.replace(about_route, disabled_about_route, 1)
elif disabled_about_route not in source:
    raise RuntimeError("Could not find navigation route to About")
navio_screens.write_text(source, encoding="utf-8")

navio_screens_header = navio_screens.with_name("screens.h")
header = navio_screens_header.read_text(encoding="utf-8")
for declaration in (
    "lv_obj_t * screen_settings(void);\n",
    "lv_obj_t * screen_about(void);\n",
    "lv_obj_t * get_list_from_wd(lv_obj_t * parent, const char * name);\n",
    "void on_settings_status(bool state);\n",
):
    header = header.replace(declaration, "", 1)
navio_screens_header.write_text(header, encoding="utf-8")

navio_ui_dir = navio_screens.parent.parent
navio_ui_generated_header = navio_ui_dir / "navio_ui_gen.h"
generated_header = navio_ui_generated_header.read_text(encoding="utf-8")
for screen_name in ("settings", "about"):
    generated_header = generated_header.replace(
        f'#include "screens/{screen_name}/{screen_name}_gen.h"\n', ""
    )
navio_ui_generated_header.write_text(generated_header, encoding="utf-8")

remove_navigation_status_indicators(navio_ui_dir)

for screen_name in ("settings", "about"):
    screen_dir = navio_ui_dir / "screens" / screen_name
    if screen_dir.is_dir():
        shutil.rmtree(screen_dir)
