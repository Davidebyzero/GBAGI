#!/usr/bin/env python3
"""Scan AGI logic for ambient animated rooms and emit a generated room table.

The hand-edited source of truth lives in `extra/ambient_room_overrides.ini`.
When a game section exists there, those room numbers override auto-detected
results. If the file or a game section is missing, this script seeds it with
the current scan result and then generates `_generated/ambient_room_table.h`.
"""

from __future__ import annotations

import configparser
import importlib.util
import re
import sys
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
PROJECT_ROOT = REPO_ROOT.parent
GAMES_ROOT = PROJECT_ROOT / "games"
OVERRIDE_PATH = REPO_ROOT / "extra" / "ambient_room_overrides.ini"
REPORT_PATH = REPO_ROOT / "extra" / "ambient_room_scan_report.txt"
HEADER_PATH = REPO_ROOT / "_generated" / "ambient_room_table.h"
ROOM_EXTRACTOR_PATH = PROJECT_ROOT / "Room_extractor" / "room_extractor.py"
COMMANDS_CPP_PATH = REPO_ROOT / "romgui" / "commands.cpp"

GAME_SPECS = {
    "larry1": {
        "canonical_id": "LSL1",
        "aliases": ["LSL1", "LLLLL"],
    },
}


@dataclass
class ObjectState:
    animate: bool = False
    draw: bool = False
    position: bool = False
    view: bool = False
    loop_or_cel: bool = False
    cycle_time: bool = False
    moving: bool = False


@dataclass
class RoomDetection:
    room_number: int
    ambient_objects: list[int]
    add_to_pic_hits: int


def load_agi_command_indices() -> dict[str, int]:
    text = COMMANDS_CPP_PATH.read_text(encoding="utf-8", errors="ignore")
    marker = "const AGICMD agiCommands"
    start = text.find(marker)
    if start >= 0:
        text = text[start:]
    names = re.findall(r'\{"([^"]+)",\s*"[^"]+",\s*\d+,\s*0x[0-9A-Fa-f]+\}', text)
    return {name: index for index, name in enumerate(names)}


AGI_COMMAND_INDICES = load_agi_command_indices()

CMD_ANIMATE_OBJ = AGI_COMMAND_INDICES["animate.obj"]
CMD_DRAW = AGI_COMMAND_INDICES["draw"]
CMD_POSITION = AGI_COMMAND_INDICES["position"]
CMD_POSITION_V = AGI_COMMAND_INDICES["position.v"]
CMD_REPOSITION = AGI_COMMAND_INDICES["reposition"]
CMD_SET_VIEW = AGI_COMMAND_INDICES["set.view"]
CMD_SET_VIEW_V = AGI_COMMAND_INDICES["set.view.v"]
CMD_SET_LOOP = AGI_COMMAND_INDICES["set.loop"]
CMD_SET_LOOP_V = AGI_COMMAND_INDICES["set.loop.v"]
CMD_SET_CEL = AGI_COMMAND_INDICES["set.cel"]
CMD_SET_CEL_V = AGI_COMMAND_INDICES["set.cel.v"]
CMD_CYCLE_TIME = AGI_COMMAND_INDICES["cycle.time"]
CMD_STOP_MOTION = AGI_COMMAND_INDICES["stop.motion"]
CMD_START_MOTION = AGI_COMMAND_INDICES["start.motion"]
CMD_STEP_SIZE = AGI_COMMAND_INDICES["step.size"]
CMD_STEP_TIME = AGI_COMMAND_INDICES["step.time"]
CMD_MOVE_OBJ = AGI_COMMAND_INDICES["move.obj"]
CMD_MOVE_OBJ_V = AGI_COMMAND_INDICES["move.obj.v"]
CMD_FOLLOW_EGO = AGI_COMMAND_INDICES["follow.ego"]
CMD_WANDER = AGI_COMMAND_INDICES["wander"]
CMD_NORMAL_MOTION = AGI_COMMAND_INDICES["normal.motion"]
CMD_SET_DIR = AGI_COMMAND_INDICES["set.dir"]
CMD_GET_DIR = AGI_COMMAND_INDICES["get.dir"]
CMD_ADD_TO_PIC = AGI_COMMAND_INDICES["add.to.pic"]
CMD_ADD_TO_PIC_V = AGI_COMMAND_INDICES["add.to.pic.v"]
CMD_REPOSITION_TO = AGI_COMMAND_INDICES["reposition.to"]
CMD_REPOSITION_TO_V = AGI_COMMAND_INDICES["reposition.to.v"]

MOVEMENT_OPS = {
    CMD_REPOSITION,
    CMD_STOP_MOTION,
    CMD_START_MOTION,
    CMD_STEP_SIZE,
    CMD_STEP_TIME,
    CMD_MOVE_OBJ,
    CMD_MOVE_OBJ_V,
    CMD_FOLLOW_EGO,
    CMD_WANDER,
    CMD_NORMAL_MOTION,
    CMD_SET_DIR,
    CMD_GET_DIR,
    CMD_REPOSITION_TO,
    CMD_REPOSITION_TO_V,
}

OBJECT_FIRST_PARAM_OPS = {
    CMD_ANIMATE_OBJ,
    CMD_DRAW,
    CMD_POSITION,
    CMD_POSITION_V,
    CMD_REPOSITION,
    CMD_SET_VIEW,
    CMD_SET_VIEW_V,
    CMD_SET_LOOP,
    CMD_SET_LOOP_V,
    CMD_SET_CEL,
    CMD_SET_CEL_V,
    CMD_CYCLE_TIME,
    CMD_STOP_MOTION,
    CMD_START_MOTION,
    CMD_STEP_SIZE,
    CMD_STEP_TIME,
    CMD_MOVE_OBJ,
    CMD_MOVE_OBJ_V,
    CMD_FOLLOW_EGO,
    CMD_WANDER,
    CMD_NORMAL_MOTION,
    CMD_SET_DIR,
    CMD_GET_DIR,
    CMD_REPOSITION_TO,
    CMD_REPOSITION_TO_V,
}


def load_room_extractor_module():
    spec = importlib.util.spec_from_file_location("room_extractor", ROOM_EXTRACTOR_PATH)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load room extractor from {ROOM_EXTRACTOR_PATH}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def flatten_logic_commands(code: bytes, command_param_counts: list[int], test_param_counts: list[int]) -> list[tuple[int, bytes]]:
    out: list[tuple[int, bytes]] = []

    def walk(start: int, end: int) -> None:
        pos = start
        limit = min(end, len(code))
        while pos < limit:
            op = code[pos]
            if op == 0:
                return
            if op == 0xFF:
                cond_pos = pos + 1
                while cond_pos < limit:
                    cond_op = code[cond_pos]
                    cond_pos += 1
                    if cond_op == 0xFF:
                        break
                    if cond_op >= 0xFC:
                        continue
                    if cond_op == 0x0E:
                        if cond_pos >= limit:
                            return
                        said_count = code[cond_pos]
                        cond_pos += 1 + (said_count * 2)
                    else:
                        cond_pos += test_param_counts[cond_op] if cond_op < len(test_param_counts) else 0
                if cond_pos + 1 >= limit:
                    return
                jump_offset = code[cond_pos] | (code[cond_pos + 1] << 8)
                then_start = cond_pos + 2
                then_end = min(limit, then_start + jump_offset)
                walk(then_start, then_end)
                pos = then_end
                continue
            if op == 0xFE:
                if pos + 2 >= limit:
                    return
                offset = code[pos + 1] | (code[pos + 2] << 8)
                pos = pos + 3 + offset
                continue
            param_count = command_param_counts[op] if op < len(command_param_counts) else 0
            params = code[pos + 1:pos + 1 + param_count]
            out.append((op, params))
            pos += 1 + param_count

    walk(0, len(code))
    return out


def detect_ambient_rooms(game_dir: Path, canonical_id: str) -> tuple[list[int], list[RoomDetection]]:
    module = load_room_extractor_module()
    loader = module.AGIResourceLoader(game_dir)
    detections: list[RoomDetection] = []

    for logic_num in loader.list_logics():
        if logic_num == 0:
            continue
        try:
            code = module.parse_logic_code(loader.load_logic(logic_num))
        except Exception:
            continue

        commands = flatten_logic_commands(code, module.COMMAND_PARAM_COUNTS, module.TEST_PARAM_COUNTS)
        object_states: dict[int, ObjectState] = defaultdict(ObjectState)
        add_to_pic_hits = 0

        for op, params in commands:
            if op in {CMD_ADD_TO_PIC, CMD_ADD_TO_PIC_V}:
                add_to_pic_hits += 1

            if op not in OBJECT_FIRST_PARAM_OPS or not params:
                continue

            obj_id = int(params[0])
            if obj_id == 0:
                continue

            state = object_states[obj_id]
            if op == CMD_ANIMATE_OBJ:
                state.animate = True
            elif op in {CMD_DRAW}:
                state.draw = True
            elif op in {CMD_POSITION, CMD_POSITION_V}:
                state.position = True
            elif op in {CMD_SET_VIEW, CMD_SET_VIEW_V}:
                state.view = True
            elif op in {CMD_SET_LOOP, CMD_SET_LOOP_V, CMD_SET_CEL, CMD_SET_CEL_V}:
                state.loop_or_cel = True
            elif op == CMD_CYCLE_TIME:
                state.cycle_time = True

            if op in MOVEMENT_OPS:
                state.moving = True

        ambient_objects = sorted(
            obj_id
            for obj_id, state in object_states.items()
            if state.animate and state.draw and state.position and state.view and state.cycle_time and not state.moving
        )

        if len(ambient_objects) >= 2 or (len(ambient_objects) >= 1 and add_to_pic_hits > 0):
            detections.append(
                RoomDetection(
                    room_number=logic_num,
                    ambient_objects=ambient_objects,
                    add_to_pic_hits=add_to_pic_hits,
                )
            )

    return [entry.room_number for entry in detections], detections


def parse_room_list(text: str) -> list[int]:
    rooms: list[int] = []
    for chunk in text.replace(",", " ").split():
        value = chunk.strip()
        if not value:
            continue
        room_num = int(value, 10)
        if 0 <= room_num <= 255 and room_num not in rooms:
            rooms.append(room_num)
    rooms.sort()
    return rooms


def room_list_text(rooms: list[int]) -> str:
    return ", ".join(str(room) for room in rooms)


def load_or_seed_overrides(auto_detected: dict[str, list[int]]) -> dict[str, list[int]]:
    config = configparser.ConfigParser()
    config.optionxform = str
    if OVERRIDE_PATH.exists():
        config.read(OVERRIDE_PATH, encoding="utf-8")

    changed = False
    for canonical_id, rooms in auto_detected.items():
        if not config.has_section(canonical_id):
            config.add_section(canonical_id)
            changed = True
        if not config.has_option(canonical_id, "rooms"):
            config.set(canonical_id, "rooms", room_list_text(rooms))
            changed = True

    if changed or not OVERRIDE_PATH.exists():
        OVERRIDE_PATH.parent.mkdir(parents=True, exist_ok=True)
        with OVERRIDE_PATH.open("w", encoding="utf-8") as handle:
            handle.write("# Hand-edit ambient room lists here.\n")
            handle.write("# The scanner seeds missing sections, but existing values are kept.\n")
            handle.write("# Use comma or space separated room numbers.\n\n")
            config.write(handle)

    final_rooms: dict[str, list[int]] = {}
    for canonical_id, detected in auto_detected.items():
        if config.has_option(canonical_id, "rooms"):
            final_rooms[canonical_id] = parse_room_list(config.get(canonical_id, "rooms"))
        else:
            final_rooms[canonical_id] = list(detected)
    return final_rooms


def write_report(detection_report: dict[str, list[RoomDetection]], final_rooms: dict[str, list[int]]) -> None:
    lines = [
        "GBAGI Ambient Room Scan Report",
        "==============================",
        "",
        f"Override file: {OVERRIDE_PATH.as_posix()}",
        "",
    ]
    for canonical_id, detections in sorted(detection_report.items()):
        lines.append(f"[{canonical_id}]")
        if detections:
            lines.append("Auto-detected rooms:")
            for entry in detections:
                lines.append(
                    f"  room {entry.room_number}: anchored objects {entry.ambient_objects}, add.to.pic hits {entry.add_to_pic_hits}"
                )
        else:
            lines.append("Auto-detected rooms: none")
        lines.append(f"Final rooms: {room_list_text(final_rooms.get(canonical_id, [])) or '(none)'}")
        lines.append("")
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text("\n".join(lines), encoding="utf-8")


def write_header(final_rooms: dict[str, list[int]]) -> None:
    lines = [
        "/* Auto-generated by extra/generate_ambient_room_table.py. */",
        "#ifndef GBAGI_AMBIENT_ROOM_TABLE_H",
        "#define GBAGI_AMBIENT_ROOM_TABLE_H",
        "",
        "typedef struct ambient_room_game_list {",
        "\tconst char *game_id;",
        "\tconst U8 *rooms;",
        "\tU8 room_count;",
        "} ambient_room_game_list;",
        "",
    ]

    table_entries: list[str] = []
    for folder_name, spec in sorted(GAME_SPECS.items()):
        canonical_id = spec["canonical_id"]
        rooms = sorted(final_rooms.get(canonical_id, []))
        array_name = f"kAmbientRooms_{canonical_id}"
        room_items = ", ".join(f"{room}U" for room in rooms) if rooms else "0U"
        lines.append(f"static const U8 {array_name}[] = {{{room_items}}};")
        for alias in spec["aliases"]:
            table_entries.append(
                f'\t{{"{alias}", {array_name}, {len(rooms)}U}},'
            )
    if not table_entries:
        lines.append("static const U8 kAmbientRooms_Empty[] = {0U};")
        lines.append("static const ambient_room_game_list kAmbientRoomGameLists[] = {{NULL, kAmbientRooms_Empty, 0U}};")
        lines.append("#define K_AMBIENT_ROOM_GAME_LIST_COUNT 0U")
    else:
        lines.append("")
        lines.append("static const ambient_room_game_list kAmbientRoomGameLists[] = {")
        lines.extend(table_entries)
        lines.append("};")
        lines.append(f"#define K_AMBIENT_ROOM_GAME_LIST_COUNT {len(table_entries)}U")
    lines.append("")
    lines.append("#endif")
    lines.append("")

    HEADER_PATH.parent.mkdir(parents=True, exist_ok=True)
    HEADER_PATH.write_text("\n".join(lines), encoding="ascii")


def main() -> int:
    auto_detected: dict[str, list[int]] = {}
    detection_report: dict[str, list[RoomDetection]] = {}

    for folder_name, spec in sorted(GAME_SPECS.items()):
        game_dir = GAMES_ROOT / folder_name
        canonical_id = spec["canonical_id"]
        if game_dir.is_dir():
            rooms, detections = detect_ambient_rooms(game_dir, canonical_id)
            auto_detected[canonical_id] = sorted(rooms)
            detection_report[canonical_id] = detections
        else:
            auto_detected[canonical_id] = []
            detection_report[canonical_id] = []

    final_rooms = load_or_seed_overrides(auto_detected)
    write_report(detection_report, final_rooms)
    write_header(final_rooms)

    for canonical_id in sorted(final_rooms):
        print(f"{canonical_id}={room_list_text(final_rooms[canonical_id])}")
    print(f"OVERRIDES={OVERRIDE_PATH}")
    print(f"REPORT={REPORT_PATH}")
    print(f"HEADER={HEADER_PATH}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
