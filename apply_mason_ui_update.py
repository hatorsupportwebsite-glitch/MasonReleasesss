#!/usr/bin/env python3
"""Apply the requested UI-only update to a local MasonReleasesss checkout.

Python standard library only. No network calls, Git pushes, game hooks or builds.
All edits are prepared and checked first. Original files are backed up; a failed
write is rolled back. Run --dry-run first to check your exact source revision.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import difflib
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import sys
import tempfile

HERE = Path(__file__).resolve().parent
PAYLOAD = HERE / 'mason_ui_payload'
spec = importlib.util.spec_from_file_location('mason_layout_snippets', PAYLOAD / 'layout_snippets.py')
S = importlib.util.module_from_spec(spec)
spec.loader.exec_module(S)
MARKER = '// Mason UI customization v1'
UI = 'product/everness/src/alice/ui/ui.cpp'
VISUAL = 'product/everness/src/alice/ui/tabs/visual/visual.cpp'
CONFIGS = 'product/render/configs.cpp'
MENU = 'product/render/menu.cpp'
OVERLAY = 'product/render/overlay.cpp'
RECEIPT = '.mason-ui-update.json'


class UpdateError(RuntimeError):
    pass


def require(condition, message):
    if not condition:
        raise UpdateError(message)


def masked_cpp(text):
    """Keep offsets, but hide comments/quoted strings when matching delimiters."""
    pattern = r'//[^\n]*|/\*[\s\S]*?\*/|R"([^\s()\\]{0,16})\([\s\S]*?\)\1"|"(?:\\[\s\S]|[^"\\])*"|\'(?:\\[\s\S]|[^\'\\])*\''
    return re.sub(pattern, lambda m: ''.join('\n' if c == '\n' else ' ' for c in m.group()), text)


def closing_delimiter(mask, opening, left, right):
    require(mask[opening] == left, 'Internal delimiter mismatch')
    depth = 0
    for i in range(opening, len(mask)):
        if mask[i] == left:
            depth += 1
        elif mask[i] == right:
            depth -= 1
            if depth == 0:
                return i
    raise UpdateError('Unbalanced source delimiters; no files were changed')


def one_match(pattern, text, description, flags=0):
    matches = list(re.finditer(pattern, text, flags))
    require(len(matches) == 1, f'{description}: expected one match, found {len(matches)}. Source revision differs.')
    return matches[0]


def substitute(pattern, replacement, text, description, flags=0):
    match = one_match(pattern, text, description, flags)
    return text[:match.start()] + replacement + text[match.end():]


def function_bounds(text, name):
    mask = masked_cpp(text)
    definitions = []
    for match in re.finditer(r'\b' + re.escape(name) + r'\s*\(', mask):
        param_open = mask.index('(', match.start())
        param_close = closing_delimiter(mask, param_open, '(', ')')
        tail = re.match(r'\s*(?:const\s*)?(?:noexcept\s*)?\{', mask[param_close + 1:])
        if tail:
            opening = param_close + 1 + tail.end() - 1
            definitions.append((opening, closing_delimiter(mask, opening, '{', '}')))
    require(len(definitions) == 1, f'{name}: expected one function definition, found {len(definitions)}')
    return definitions[0]


def change_body(text, name, transform):
    opening, closing = function_bounds(text, name)
    return text[:opening + 1] + transform(text[opening + 1:closing]) + text[closing:]


def calls(text, name):
    mask = masked_cpp(text)
    for match in re.finditer(r'\b' + re.escape(name) + r'\s*\(', mask):
        opening = mask.index('(', match.start())
        closing = closing_delimiter(mask, opening, '(', ')')
        end = closing + 1
        while end < len(mask) and mask[end].isspace():
            end += 1
        require(end < len(mask) and mask[end] == ';', f'{name}: expected a complete statement')
        yield match.start(), end + 1


def call_by_label(text, name, label):
    matches = [(a, b) for a, b in calls(text, name) if '"' + label + '"' in text[a:b]]
    require(len(matches) == 1, f'{name} / {label}: expected one control, found {len(matches)}')
    return matches[0]


def change_call(text, name, label, replacement):
    a, b = call_by_label(text, name, label)
    old = text[a:b]
    new = replacement(old) if callable(replacement) else replacement
    return text[:a] + new + text[b:]


def add_header(text, includes):
    require(MARKER not in text, 'A file is already customized. Do not apply the update twice.')
    return MARKER + '\n' + includes + '\n' + text


def child_bounds(body, child):
    mask = masked_cpp(body)
    match = one_match(r'\bif\s*\(\s*child\s*==\s*' + str(child) + r'\s*\)\s*\{', mask, f'child == {child}')
    opening = mask.index('{', match.start())
    return opening, closing_delimiter(mask, opening, '{', '}')


def patch_ui(text):
    text = add_header(text, '#include "mason_ui_options.hpp"\n#include "render/configs.h"\n#include <cstdlib>')

    def init(body):
        mask = masked_cpp(body)
        start = one_match(r'\bm_tabs\s*=\s*\{', mask, 'sidebar registration')
        opening = mask.index('{', start.start())
        closing = closing_delimiter(mask, opening, '{', '}')
        old_names = re.findall(r'\.name\s*=\s*"([^"]+)"', body[opening:closing])
        require(old_names == ['VISUAL', 'Entities', 'World', 'COMBAT', 'Legitbot', 'CLOUD', 'Skins', 'Configs'],
                'Sidebar has additional or renamed tabs; review required before replacement')
        end = closing + 1
        while body[end].isspace():
            end += 1
        require(body[end] == ';', 'Missing sidebar terminator')
        return body[:start.start()] + S.SIDEBAR + body[end + 1:]

    text = change_body(text, 'CUserInterface::init', init)

    def settings(body):
        body = substitute(r'CThemes::current\s*=\s*\*themePtrs\s*\[\s*currentThemeIndex\s*\]\s*;',
                          'CThemes::current = *themePtrs[currentThemeIndex];\n                mason_ui::options.theme_name = themeNames[currentThemeIndex];',
                          body, 'remember selected theme')
        body = substitute(r'themePtrs\s*\[\s*i\s*\]\s*==\s*&CThemes::current',
                          '!mason_ui::options.theme_name.empty() && themeNames[i] == mason_ui::options.theme_name',
                          body, 'restore theme selection')
        a, _ = call_by_label(body, 'CUIElements::toggle', 'Main glow')
        return body[:a] + S.DPI_SETTING + body[a:]

    text = change_body(text, 'CUserInterface::MenuSettings::render', settings)

    def render(body):
        # Replace only the top-bar contents before the existing settings gear.
        _, after_top_call = call_by_label(body, 'BeginChild', '##top_area')
        mask = masked_cpp(body)
        opening = mask.find('{', after_top_call)
        require(opening >= 0 and not mask[after_top_call:opening].strip(), 'Unexpected top-bar body')
        gear = one_match(r'SetCursorPos\s*\(\s*\{\s*GetWindowSize\s*\(\s*\)\s*\.x\s*-\s*28\s*-\s*10',
                         body[opening + 1:], 'top-bar settings gear')
        gear_start = opening + 1 + gear.start()
        body = body[:opening + 1] + S.SAVE_AREA + body[gear_start:]

        # The logical menu must still fit smaller desktops at 150-200% scale.
        body = re.sub(r'\bm_windowSize\b', 'menu_size', body)
        size_definition = '''auto& style = GetStyle( );
    const ImVec2 menu_size(
        (std::min)(m_windowSize.x, (std::max)(320.0f, io.DisplaySize.x - 16.0f)),
        (std::min)(m_windowSize.y, (std::max)(220.0f, io.DisplaySize.y - 16.0f)));
'''
        body = substitute(r'auto\s*&\s*style\s*=\s*GetStyle\s*\(\s*\)\s*;', size_definition,
                          body, 'logical menu bounds')
        positions = list(calls(body, 'SetNextWindowPos'))
        require(len(positions) == 1, 'Expected one menu position statement')
        a, b = positions[0]
        position = '''ImVec2 menu_position((io.DisplaySize.x - menu_size.x) * 0.5f, (io.DisplaySize.y - menu_size.y) * 0.5f);
    if (const auto* previous = ImGui::FindWindowByName("@alicegang")) menu_position = previous->Pos;
    menu_position.x = std::clamp(menu_position.x, 8.0f, (std::max)(8.0f, io.DisplaySize.x - menu_size.x - 8.0f));
    menu_position.y = std::clamp(menu_position.y, 8.0f, (std::max)(8.0f, io.DisplaySize.y - menu_size.y - 8.0f));
    SetNextWindowPos(menu_position, ImGuiCond_Always);'''
        return body[:a] + position + body[b:]

    return change_body(text, 'CUserInterface::render', render)


def patch_visual(text):
    text = add_header(text, '#include "mason_ui_options.hpp"\n#include <algorithm>')
    helper = one_match(r'\bstatic\s+void\s+esp_popup_gradient_anim_lr\s*\(', masked_cpp(text), 'popup helper insertion')
    text = text[:helper.start()] + S.CHOICE_HELPER + text[helper.start():]

    for label in ['Corner Box##ent2_corner', 'Filled Box##ent2_fill', 'Step Circle##ent2_circle']:
        text = change_call(text, 'CUIElements::toggle', label, '')

    # Relocate the existing Chams control instead of creating a duplicate.
    a, b = call_by_label(text, 'CUIElements::toggle', 'Chams##ent_chams')
    chams = text[a:b].replace('Chams##ent_chams', 'Chams##ent2_chams')
    text = text[:a] + text[b:]
    text = substitute(r'\bstatic\s+CPopup\s+pop_chams\s*\([^;]*\)\s*;', '', text, 'old Chams popup')
    text = substitute(r'\bstatic\s+CPopup\s+pop_hitsparks\s*\([^;]*\)\s*;',
                      'static CPopup pop_chams( 180, "Chams", std::nullopt );', text, 'relocated Chams popup')
    text = change_call(text, 'CUIElements::toggle', 'Hit Sparks##ent2_hitsparks', chams)

    for label in ['Smoke Color##w_smoke', 'Molotov Color##w_molo']:
        text = change_call(text, 'CUIElements::color', label, '')
    text = change_call(text, 'CUIElements::toggle', 'Grenade Trails##w_gren_trails',
                       lambda old: old.replace('ElemType::Middle', 'ElemType::End', 1))
    text = change_call(text, 'CUIElements::toggle', 'Night Mode##w_night', S.REMOVALS)
    text = change_body(text, 'alice::tabs::callWorld', lambda body: body + '''
    // Misc remains accessible on the same page after removing the subtab switch.
    if (child == 1 || child == 2) {
        ImGui::Spacing();
        CUIElements::text("Misc");
        alice::tabs::callWorldMisc(child);
    }
''')

    def aim(body):
        opening, closing = child_bounds(body, 1)
        left = body[opening + 1:closing]
        a, _ = call_by_label(left, 'CUIElements::toggle', 'Show Fov##aim_fov_show')
        body = body[:opening + 1] + left[:a] + S.AIM_LEFT + body[closing:]
        opening, closing = child_bounds(body, 2)
        require(not masked_cpp(body[opening + 1:closing]).strip(), 'Aimbot right column is no longer empty; review required')
        return body[:opening + 1] + S.AIM_RIGHT + body[closing:]

    text = change_body(text, 'alice::tabs::callLegitbot', aim)
    # Keep the legacy declaration link-compatible, but no Triggerbot UI remains.
    return change_body(text, 'alice::tabs::callLegitbotTrigger',
                       lambda body: '\n    (void)child; // Removed from the standalone menu.\n')


def patch_configs(text):
    text = add_header(text, '#include "mason_ui_options.hpp"')

    def build(body):
        match = one_match(r'o\s*<<\s*"v=2\\n"\s*;', body, 'configuration version line')
        return body[:match.end()] + S.CONFIG_WRITE + body[match.end():]

    text = change_body(text, 'build_plaintext', build)
    text = change_body(text, 'apply_line', lambda body: S.CONFIG_APPLY + body)

    def load(body):
        match = one_match(r'if\s*\(\s*!\s*read_decrypt\s*\(\s*p\s*,\s*plain\s*\)\s*\)\s*return\s+false\s*;',
                          body, 'successful configuration read')
        return body[:match.end()] + '\n        mason_ui::reset_options(); // Defaults for older profiles without ui2_ fields.\n' + body[match.end():]

    text = change_body(text, 'load_config', load)

    def write(body):
        return substitute(r'\breturn\s+true\s*;',
                          'f.flush();\n        if (!f) { set_err("write failed"); return false; }\n        return true;',
                          body, 'configuration write status')

    return change_body(text, 'write_encrypted', write)


def patch_menu(text):
    text = add_header(text, '#include "configs.h"\n#include <algorithm>')

    def initialize(body):
        return substitute(r'\bSetMenuOpen\s*\(\s*true\s*\)\s*;', '''// Restore the same profile written by the Save button on every tab.
    const auto mason_profiles = configs::list_configs();
    if (std::find(mason_profiles.begin(), mason_profiles.end(), "autosave") != mason_profiles.end())
        configs::load_config("autosave");
    SetMenuOpen(true);''', body, 'startup profile restore')

    return change_body(text, 'InitializeMenu', initialize)


def patch_overlay(text):
    text = add_header(text, '#include "mason_ui_dpi.hpp"')
    text = substitute(r'ImGui_ImplDX11_NewFrame\s*\(\s*\)\s*;\s*ImGui_ImplWin32_NewFrame\s*\(\s*\)\s*;\s*ImGui::NewFrame\s*\(\s*\)\s*;',
                      '''ImGui_ImplDX11_NewFrame();
            mason_ui::dpi::before_platform_frame();
            ImGui_ImplWin32_NewFrame();
            mason_ui::dpi::before_new_frame();
            ImGui::NewFrame();
            mason_ui::dpi::after_new_frame();''', text, 'DPI input/frame integration')
    return substitute(r'RenderMenu\s*\(\s*\)\s*;\s*ImGui::Render\s*\(\s*\)\s*;',
                      '''RenderMenu();
            ImGui::Render();
            mason_ui::dpi::scale_draw_data(ImGui::GetDrawData());''', text, 'DPI draw integration')


TRANSFORMS = {UI: patch_ui, VISUAL: patch_visual, CONFIGS: patch_configs, MENU: patch_menu, OVERLAY: patch_overlay}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def confined(root, relative):
    path = root / relative
    require(not path.is_symlink(), f'Refusing to replace a symlink: {relative}')
    require(path.resolve().is_relative_to(root), f'Path leaves the project: {relative}')
    return path


def prepare(root):
    require((root / 'CMakeLists.txt').is_file(), 'Run in the project root (the directory containing CMakeLists.txt).')
    changes = {}
    for relative, transform in TRANSFORMS.items():
        path = confined(root, relative)
        require(path.is_file(), f'Missing source: {relative}')
        before = path.read_bytes()
        text = before.decode('utf-8-sig').replace('\r\n', '\n')
        after = transform(text).encode('utf-8')
        changes[relative] = (before, after)
    for relative in ['product/mason_ui_options.hpp', 'product/mason_ui_dpi.hpp']:
        path = confined(root, relative)
        before = path.read_bytes() if path.exists() else None
        after = (PAYLOAD / relative).read_bytes()
        require(before is None or before == after, f'Existing custom header differs: {relative}')
        changes[relative] = (before, after)

    # UI files were already intentional edits in the project's preservation manifest.
    # Never disable the verifier or rewrite its original hashes to hide changes.
    manifest_path = root / 'tests/ui_preservation_manifest.json'
    require(manifest_path.is_file(), 'Missing UI preservation manifest')
    manifest = json.loads(manifest_path.read_text(encoding='utf-8-sig'))
    for item in manifest:
        if 'product/' + item['path'] in changes:
            require(item.get('modified') is True, f"Preserved file would change: {item['path']}. Review the manifest intentionally first.")

    ui = changes[UI][1].decode()
    visual = changes[VISUAL][1].decode()
    require('.name = "COMMON"' in ui and '.name = "Inventory"' in ui, 'Sidebar validation failed')
    require('"Triggerbot"' not in ui and '.subTabs =' not in ui, 'Obsolete subtab remains')
    for old_label in ['Corner Box##', 'Filled Box##', 'Step Circle##', 'Hit Sparks##', 'Smoke Color##', 'Molotov Color##', 'Night Mode##']:
        require(old_label not in visual, f'Removed control remains: {old_label}')
    require(visual.count('"Chams##') == 1, 'Expected one Chams control')
    return changes


def install(root, changes):
    timestamp = datetime.now(timezone.utc).strftime('%Y%m%d-%H%M%S-%f')
    backup = root / ('.mason-ui-backup-' + timestamp)
    backup.mkdir()
    # Verify every file still matches the exact bytes used to prepare the edits.
    for relative, (before, _) in changes.items():
        path = confined(root, relative)
        current = path.read_bytes() if path.exists() else None
        require(current == before, f'File changed while preparing update: {relative}')
        if before is not None:
            target = backup / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(before)
    written = []
    try:
        with tempfile.TemporaryDirectory(prefix='.mason-ui-stage-', dir=root) as staging:
            staging = Path(staging)
            for relative, (_, after) in changes.items():
                target = staging / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(after)
            for relative in changes:
                target = confined(root, relative)
                target.parent.mkdir(parents=True, exist_ok=True)
                os.replace(staging / relative, target)
                written.append(relative)
        receipt = {
            'version': 1,
            'backup': backup.name,
            'files': {name: {'before': digest(old) if old is not None else None, 'after': digest(new)}
                      for name, (old, new) in changes.items()},
        }
        receipt_pending = backup / 'receipt_pending.json'
        receipt_pending.write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
        os.replace(receipt_pending, confined(root, RECEIPT))
    except BaseException:
        for relative in reversed(written):
            before = changes[relative][0]
            target = root / relative
            if before is None:
                target.unlink(missing_ok=True)
            else:
                target.write_bytes(before)
        raise
    return backup


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path.cwd(), help='Existing project root')
    parser.add_argument('--dry-run', action='store_true', help='Validate all edits without writing anything')
    parser.add_argument('--diff', type=Path, help='Optionally write a reviewable unified diff')
    args = parser.parse_args()
    root = args.root.resolve()
    try:
        receipt_path = confined(root, RECEIPT)
        if receipt_path.is_file():
            receipt = json.loads(receipt_path.read_text(encoding='utf-8'))
            require(receipt.get('version') == 1, 'Unknown existing update version')
            expected_files = set(TRANSFORMS) | {'product/mason_ui_options.hpp', 'product/mason_ui_dpi.hpp'}
            require(set(receipt.get('files', {})) == expected_files, 'Incomplete existing update receipt')
            for relative, hashes in receipt['files'].items():
                path = confined(root, relative)
                require(path.is_file() and digest(path.read_bytes()) == hashes['after'],
                        f'Already customized, with later changes in {relative}. No files were overwritten.')
            print('Already applied. All customized files are intact; no files changed.')
            return 0
        changes = prepare(root)
        print('Validated all requested changes:')
        for relative in changes:
            print('  ' + relative)
        if args.diff:
            fragments = []
            for relative, (old, new) in changes.items():
                fragments.extend(difflib.unified_diff(
                    (old or b'').decode('utf-8-sig').replace('\r\n', '\n').splitlines(keepends=True),
                    new.decode('utf-8').splitlines(keepends=True),
                    fromfile='a/' + relative if old is not None else '/dev/null', tofile='b/' + relative))
            args.diff.write_text(''.join(fragments), encoding='utf-8', newline='\n')
        if args.dry_run:
            print('DRY RUN OK. No project files changed.')
            return 0
        backup = install(root, changes)
        print('UI update applied. Backup: ' + backup.name)
        print('Next: python tests/verify_source.py')
        print('Then commit the changed product files and run your Windows build on that new commit.')
        return 0
    except (UpdateError, OSError, UnicodeError, json.JSONDecodeError) as error:
        print('UPDATE STOPPED: ' + str(error), file=sys.stderr)
        print('No partial UI update has been left behind.', file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
