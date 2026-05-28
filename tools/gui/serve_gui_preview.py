#!/usr/bin/env python3
"""Serve the Dinotofu GUI preview with only Python standard library.

The server also exposes a tiny local input endpoint used by the experimental
browser GUI. It writes each submitted command to gui_debug/input_queue/*.cmd;
the C++ terminal game can consume this queue when DINOTOFU_GUI_INPUT_MODE=1.
It also exposes /gui/snapshot so the experimental UI can fetch the current
menu/combat/status bundle in one request.
"""

from __future__ import annotations

import argparse
import http.server
import json
import os
import shutil
import socketserver
import time
from pathlib import Path
from urllib.parse import urlparse


MAX_COMMAND_LENGTH = 500


def main() -> int:
    parser = argparse.ArgumentParser(description="Serve Dinotofu GUI preview files.")
    parser.add_argument("--root", default=str(Path(__file__).resolve().parents[2]), help="Project root to serve")
    parser.add_argument("--host", default="127.0.0.1", help="Host to bind")
    parser.add_argument("--port", default=8787, type=int, help="Port to bind")
    parser.add_argument("--gui-debug-dir", default="", help="Directory containing latest_menu/latest_combat and pending_input")
    args = parser.parse_args()

    root = Path(args.root).resolve()
    gui_debug_dir = Path(args.gui_debug_dir).resolve() if args.gui_debug_dir else root / "gui_debug"
    pending_input_file = gui_debug_dir / "pending_input.txt"
    input_queue_dir = gui_debug_dir / "input_queue"
    input_history_file = gui_debug_dir / "input_history.json"
    consumed_input_file = gui_debug_dir / "last_consumed_input.json"
    force_stop_file = gui_debug_dir / "force_stop.txt"
    os.chdir(root)

    def sorted_queue_command_paths() -> list[Path]:
        if not input_queue_dir.exists():
            return []
        return sorted(
            path
            for path in input_queue_dir.iterdir()
            if path.is_file() and path.suffix == ".cmd"
        )

    def count_queue_files() -> int:
        return len(sorted_queue_command_paths())

    def read_json_file(path: Path):
        if not path.exists():
            return None
        try:
            return json.loads(path.read_text(encoding="utf-8") or "null")
        except (OSError, json.JSONDecodeError):
            return None

    def safe_child_json_path(file_name: str, fallback: str) -> Path:
        safe_name = Path(str(file_name or fallback)).name
        if not safe_name.endswith(".json"):
            safe_name = fallback
        return gui_debug_dir / safe_name

    def build_pending_queue_preview(limit: int = 12) -> list[dict]:
        preview: list[dict] = []
        for command_path in sorted_queue_command_paths()[:limit]:
            command_id = command_path.stem
            command = ""
            try:
                command = command_path.read_text(encoding="utf-8").splitlines()[0] if command_path.exists() else ""
            except OSError:
                command = ""

            metadata = read_json_file(input_queue_dir / f"{command_id}.meta.json")
            if not isinstance(metadata, dict):
                metadata = {}

            preview.append({
                "id": command_id,
                "file": command_path.name,
                "command": command,
                "display": metadata.get("display") or ("[Entree]" if command == "" else command),
                "screenId": metadata.get("screenId", ""),
                "screenSequence": metadata.get("screenSequence", 0),
                "inputMode": metadata.get("inputMode", ""),
                "expectedInputPattern": metadata.get("expectedInputPattern", ""),
                "primarySubmitLabel": metadata.get("primarySubmitLabel", ""),
                "acceptsEmptyInput": metadata.get("acceptsEmptyInput", False),
                "actionId": metadata.get("actionId", ""),
                "targetActionId": metadata.get("targetActionId", ""),
                "actionLabel": metadata.get("actionLabel", ""),
                "actionDisplayLabel": metadata.get("actionDisplayLabel", ""),
                "focusCardId": metadata.get("focusCardId", ""),
                "focusCardLabel": metadata.get("focusCardLabel", ""),
                "itemCardId": metadata.get("itemCardId", ""),
                "itemCardName": metadata.get("itemCardName", ""),
                "itemCardKind": metadata.get("itemCardKind", ""),
                "itemCardSection": metadata.get("itemCardSection", ""),
                "itemCardActionType": metadata.get("itemCardActionType", ""),
                "itemCardActionTypeLabel": metadata.get("itemCardActionTypeLabel", ""),
                "actionRole": metadata.get("actionRole", ""),
                "actionStyle": metadata.get("actionStyle", ""),
                "actionGroup": metadata.get("actionGroup", ""),
                "hasTextLengthRange": metadata.get("hasTextLengthRange", False),
                "textMinLength": metadata.get("textMinLength", 0),
                "textMaxLength": metadata.get("textMaxLength", 0),
                "knowledgeSensitive": metadata.get("knowledgeSensitive", False),
                "recommendationReason": metadata.get("recommendationReason", ""),
                "queuedAtEpoch": metadata.get("queuedAtEpoch"),
            })
        return preview

    def build_latest_menu_payload():
        gui_state = read_json_file(gui_debug_dir / "gui_state.json")
        menu_file = "latest_menu.json"
        if isinstance(gui_state, dict):
            menu_file = str(gui_state.get("latestMenuFile") or menu_file)
        menu = read_json_file(safe_child_json_path(menu_file, "latest_menu.json"))
        return menu if isinstance(menu, dict) else None

    def as_int(value, default: int = 0) -> int:
        try:
            return int(value)
        except (TypeError, ValueError):
            return default

    def validate_input_against_latest_menu(command: str, metadata: dict, payload: dict) -> tuple[bool, str]:
        # FR: seconde barrière côté serveur. Le navigateur valide déjà la saisie,
        # mais cette vérification évite surtout les clics envoyés depuis un ancien écran.
        menu = build_latest_menu_payload()
        if not isinstance(menu, dict):
            return True, ""

        current_screen_id = str(menu.get("screenId", ""))
        request_screen_id = str(metadata.get("screenId", ""))
        if current_screen_id and request_screen_id and current_screen_id != request_screen_id and not bool(payload.get("allowStaleInput", False)):
            return False, "screen changed before input was queued"

        gui_state = read_json_file(gui_debug_dir / "gui_state.json")
        if isinstance(gui_state, dict) and not bool(payload.get("allowStaleInput", False)):
            current_sequence = as_int(gui_state.get("sequence", 0), 0)
            request_sequence = as_int(metadata.get("screenSequence", 0), 0)
            last_export_kind = str(gui_state.get("lastExportKind", ""))
            active_screen_id = str(gui_state.get("activeScreenId", ""))
            if (
                request_sequence > 0
                and current_sequence > request_sequence
                and last_export_kind == "menu"
                and active_screen_id
                and active_screen_id != request_screen_id
            ):
                return False, "screen sequence is older than the current menu"

        input_mode = str(menu.get("inputMode", metadata.get("inputMode", "choice")) or "choice")
        accepts_empty = bool(menu.get("acceptsEmptyInput", False))
        value = str(command or "")
        trimmed = value.strip()

        if input_mode == "display":
            return False, "current screen is display-only"

        if not trimmed:
            if accepts_empty or input_mode == "continue":
                return True, ""
            return False, "current screen expects a non-empty input"

        if input_mode == "quantity":
            if not trimmed.lstrip("-").isdigit():
                return False, "current screen expects a numeric quantity"
            if bool(menu.get("hasNumericRange", False)):
                number_value = as_int(trimmed)
                minimum = as_int(menu.get("numericMin", 0))
                maximum = as_int(menu.get("numericMax", 0))
                if number_value < minimum or number_value > maximum:
                    return False, f"quantity out of range: {minimum}-{maximum}"
            return True, ""

        if input_mode == "confirmation":
            keyword = str(menu.get("confirmationKeyword", ""))
            if keyword and trimmed != keyword:
                return False, "current screen expects the exact confirmation keyword"
            return True, ""

        if input_mode == "text":
            if bool(menu.get("hasTextLengthRange", False)):
                minimum = as_int(menu.get("textMinLength", 0))
                maximum = as_int(menu.get("textMaxLength", 0))
                length = len(trimmed)
                if minimum > 0 and length < minimum:
                    return False, f"text too short: minimum {minimum}"
                if maximum > 0 and length > maximum:
                    return False, f"text too long: maximum {maximum}"
            return True, ""

        if input_mode == "choice" and trimmed.lstrip("-").isdigit():
            wanted = trimmed
            actions = menu.get("actions", [])
            if isinstance(actions, list):
                for action in actions:
                    if isinstance(action, dict) and str(action.get("number", "")) == wanted:
                        if action.get("enabled") is False:
                            return False, "selected action is locked on the current screen"
                        return True, ""

            valid_choices = menu.get("validChoices", [])
            if isinstance(valid_choices, list) and valid_choices and wanted not in {str(choice) for choice in valid_choices}:
                # FR: on refuse les nombres inconnus côté serveur, mais les textes restent possibles
                # pour les codes cachés/cheats tant que le mode attend un choix.
                return False, "numeric choice is not valid on the current screen"

        return True, ""

    def build_status_payload() -> dict:
        legacy_pending_count = 0
        if pending_input_file.exists():
            legacy_pending_count = len(pending_input_file.read_text(encoding="utf-8").splitlines())

        pending_queue = build_pending_queue_preview()
        queue_count = count_queue_files()
        history = read_json_file(input_history_file)
        consumed = read_json_file(consumed_input_file)
        history_list = history if isinstance(history, list) else []
        last_queued = history_list[-1] if history_list and isinstance(history_list[-1], dict) else None

        return {
            "ok": True,
            "guiDebugDir": str(gui_debug_dir),
            "pendingInputFile": str(pending_input_file),
            "inputQueueDir": str(input_queue_dir),
            "pendingInputCount": queue_count + legacy_pending_count,
            "pendingQueueCount": queue_count,
            "pendingLegacyCount": legacy_pending_count,
            "pendingQueue": pending_queue,
            "latestMenuExists": (gui_debug_dir / "latest_menu.json").exists(),
            "latestCombatExists": (gui_debug_dir / "latest_combat.json").exists(),
            "guiStateExists": (gui_debug_dir / "gui_state.json").exists(),
            "lastConsumedInput": consumed if isinstance(consumed, dict) else None,
            "lastQueuedInput": last_queued,
            "inputHistory": history_list[-8:],
        }

    class Handler(http.server.SimpleHTTPRequestHandler):
        def end_headers(self) -> None:
            self.send_header("Cache-Control", "no-store")
            super().end_headers()

        def _send_json(self, status: int, payload: dict) -> None:
            body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
            self.send_response(status)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def do_GET(self) -> None:  # noqa: N802 - stdlib handler method name
            parsed = urlparse(self.path)
            if parsed.path == "/gui/status":
                try:
                    self._send_json(200, build_status_payload())
                except OSError as error:
                    self._send_json(500, {"ok": False, "error": str(error)})
                return

            if parsed.path == "/gui/snapshot":
                try:
                    gui_state = read_json_file(gui_debug_dir / "gui_state.json")
                    menu_file = "latest_menu.json"
                    combat_file = "latest_combat.json"
                    if isinstance(gui_state, dict):
                        menu_file = str(gui_state.get("latestMenuFile") or menu_file)
                        combat_file = str(gui_state.get("latestCombatFile") or combat_file)

                    menu = read_json_file(safe_child_json_path(menu_file, "latest_menu.json"))
                    combat = read_json_file(safe_child_json_path(combat_file, "latest_combat.json"))

                    self._send_json(200, {
                        "ok": True,
                        "type": "gui_snapshot_bundle",
                        "fetchedAtEpoch": int(time.time()),
                        "status": build_status_payload(),
                        "guiState": gui_state if isinstance(gui_state, dict) else None,
                        "menu": menu if isinstance(menu, dict) else None,
                        "combat": combat if isinstance(combat, dict) else None,
                    })
                except OSError as error:
                    self._send_json(500, {"ok": False, "error": str(error)})
                return

            super().do_GET()

        def do_POST(self) -> None:  # noqa: N802 - stdlib handler method name
            parsed = urlparse(self.path)
            if parsed.path not in {"/gui/input", "/gui/clear-input", "/gui/force-stop"}:
                self._send_json(404, {"ok": False, "error": "unknown endpoint"})
                return

            length_text = self.headers.get("Content-Length", "0")
            try:
                length = max(0, min(4096, int(length_text)))
            except ValueError:
                length = 0

            raw_body = self.rfile.read(length)
            try:
                payload = json.loads(raw_body.decode("utf-8") or "{}")
            except json.JSONDecodeError:
                self._send_json(400, {"ok": False, "error": "invalid json"})
                return

            if parsed.path == "/gui/clear-input":
                try:
                    gui_debug_dir.mkdir(parents=True, exist_ok=True)
                    pending_input_file.write_text("", encoding="utf-8")
                    if input_queue_dir.exists():
                        shutil.rmtree(input_queue_dir)
                    input_queue_dir.mkdir(parents=True, exist_ok=True)
                except OSError as error:
                    self._send_json(500, {"ok": False, "error": str(error)})
                    return

                self._send_json(200, {"ok": True, "cleared": True, "file": str(pending_input_file), "queueDir": str(input_queue_dir)})
                return

            if parsed.path == "/gui/force-stop":
                try:
                    gui_debug_dir.mkdir(parents=True, exist_ok=True)
                    force_stop_file.write_text(str(int(time.time())) + "\n", encoding="utf-8")
                    pending_input_file.write_text("", encoding="utf-8")
                    if input_queue_dir.exists():
                        shutil.rmtree(input_queue_dir)
                    input_queue_dir.mkdir(parents=True, exist_ok=True)
                except OSError as error:
                    self._send_json(500, {"ok": False, "error": str(error)})
                    return

                self._send_json(200, {"ok": True, "forceStopRequested": True, "file": str(force_stop_file)})
                return

            command = str(payload.get("command", ""))
            command = command.replace("\r", "").replace("\n", "")[:MAX_COMMAND_LENGTH]

            metadata = {
                "screenId": str(payload.get("screenId", ""))[:120],
                "screenSequence": as_int(payload.get("screenSequence", 0), 0),
                "screenCategory": str(payload.get("screenCategory", ""))[:80],
                "inputMode": str(payload.get("inputMode", ""))[:80],
                "inputIntent": str(payload.get("inputIntent", ""))[:120],
                "expectedInputPattern": str(payload.get("expectedInputPattern", ""))[:120],
                "primarySubmitLabel": str(payload.get("primarySubmitLabel", ""))[:120],
                "acceptsEmptyInput": bool(payload.get("acceptsEmptyInput", False)),
                "hasNumericRange": bool(payload.get("hasNumericRange", False)),
                "numericMin": payload.get("numericMin", 0),
                "numericMax": payload.get("numericMax", 0),
                "hasTextLengthRange": bool(payload.get("hasTextLengthRange", False)),
                "textMinLength": payload.get("textMinLength", 0),
                "textMaxLength": payload.get("textMaxLength", 0),
                "commandKind": str(payload.get("commandKind", ""))[:80],
                "actionId": str(payload.get("actionId", ""))[:160],
                "targetActionId": str(payload.get("targetActionId", ""))[:160],
                "actionLabel": str(payload.get("actionLabel", ""))[:220],
                "actionDisplayLabel": str(payload.get("actionDisplayLabel", ""))[:240],
                "focusCardId": str(payload.get("focusCardId", ""))[:160],
                "focusCardLabel": str(payload.get("focusCardLabel", ""))[:180],
                "itemCardId": str(payload.get("itemCardId", ""))[:160],
                "itemCardName": str(payload.get("itemCardName", ""))[:180],
                "itemCardKind": str(payload.get("itemCardKind", ""))[:80],
                "itemCardSection": str(payload.get("itemCardSection", ""))[:80],
                "itemCardActionType": str(payload.get("itemCardActionType", ""))[:80],
                "itemCardActionTypeLabel": str(payload.get("itemCardActionTypeLabel", ""))[:120],
                "actionRole": str(payload.get("actionRole", ""))[:80],
                "actionStyle": str(payload.get("actionStyle", ""))[:80],
                "actionGroup": str(payload.get("actionGroup", ""))[:80],
                "knowledgeSensitive": bool(payload.get("knowledgeSensitive", False)),
                "recommendationReason": str(payload.get("recommendationReason", ""))[:220],
                "source": str(payload.get("source", "browser_gui"))[:80],
            }

            allow_queue_stacking = bool(payload.get("allowQueueStacking", False))
            legacy_has_pending = pending_input_file.exists() and bool(pending_input_file.read_text(encoding="utf-8").strip())
            if not allow_queue_stacking and (count_queue_files() + (1 if legacy_has_pending else 0)) > 0:
                self._send_json(409, {
                    "ok": False,
                    "error": "input queue already has a pending command",
                    "status": build_status_payload(),
                })
                return

            valid_input, validation_error = validate_input_against_latest_menu(command, metadata, payload)
            if not valid_input:
                self._send_json(409, {
                    "ok": False,
                    "error": validation_error,
                    "status": build_status_payload(),
                })
                return

            # Empty command is allowed: it means Enter / continue.
            try:
                gui_debug_dir.mkdir(parents=True, exist_ok=True)
                input_queue_dir.mkdir(parents=True, exist_ok=True)

                command_id = f"{time.time_ns()}_{os.getpid()}"
                temporary_path = input_queue_dir / f"{command_id}.tmp"
                command_path = input_queue_dir / f"{command_id}.cmd"
                temporary_path.write_text(command + "\n", encoding="utf-8")
                temporary_path.replace(command_path)

                meta_path = input_queue_dir / f"{command_id}.meta.json"
                meta_path.write_text(json.dumps({
                    "id": command_id,
                    "queuedAtEpoch": int(time.time()),
                    "command": command,
                    "display": "[Entree]" if command == "" else command,
                    **metadata,
                }, ensure_ascii=False, indent=2), encoding="utf-8")

                history = []
                if input_history_file.exists():
                    try:
                        parsed_history = json.loads(input_history_file.read_text(encoding="utf-8") or "[]")
                        if isinstance(parsed_history, list):
                            history = parsed_history[-24:]
                    except json.JSONDecodeError:
                        history = []
                history.append({
                    "id": command_id,
                    "queuedAtEpoch": int(time.time()),
                    "command": command,
                    "display": "[Entree]" if command == "" else command,
                    **metadata,
                })
                input_history_file.write_text(json.dumps(history[-25:], ensure_ascii=False, indent=2), encoding="utf-8")
            except OSError as error:
                self._send_json(500, {"ok": False, "error": str(error)})
                return

            self._send_json(200, {"ok": True, "queued": command, "id": command_id, "queueDir": str(input_queue_dir), "metadata": metadata})

    class ReusableTCPServer(socketserver.TCPServer):
        allow_reuse_address = True

    with ReusableTCPServer((args.host, args.port), Handler) as server:
        print(f"Dinotofu GUI experimental: http://{args.host}:{args.port}/tools/gui/dinotofu_gui_experimental.html")
        print(f"Project root served: {root}")
        print(f"GUI debug/input directory: {gui_debug_dir}")
        server.serve_forever()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
