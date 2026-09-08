import importlib.util
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

from source_fixtures import make_project, CONFIGS, UI, VISUAL

PACKAGE=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('mason_updater', PACKAGE/'apply_mason_ui_update.py')
u=importlib.util.module_from_spec(spec)
spec.loader.exec_module(u)


class UpdateTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.root=Path(self.temp.name).resolve()
        make_project(self.root,u)

    def tearDown(self):
        self.temp.cleanup()

    def prepared(self):
        return u.prepare(self.root)

    def test_all_five_sources_and_two_headers_prepared(self):
        self.assertEqual(len(self.prepared()),7)

    def test_sidebar_order_and_subtab_removal(self):
        s=self.prepared()[u.UI][1].decode()
        names=re.findall(r'\.name\s*=\s*"([^"]+)"',s)
        self.assertEqual(names,['COMMON','Entities','World','Inventory','COMBAT','Aimbot','CLOUD','Configs'])
        self.assertNotIn('.subTabs =',s)
        self.assertNotIn('m_selectedTab == 1',s)
        self.assertIn('configs::save_config("autosave")',s)
        self.assertNotIn('Something went wrong...',s)

    def test_all_requested_aim_controls(self):
        s=self.prepared()[u.VISUAL][1].decode()
        for label in ['Silent aim##','Aim through walls##','Automatically fire##','Bones##','Multi points##','Hit chance##','Rapid fire##','Wallshot##','No recoil##','No spread##','Hitbox override##','Texture override##']:
            self.assertIn('"'+label,s)
        self.assertIn('hit_chance_percent, 0, 100',s)
        self.assertIn('{ "Head", "Neck", "Body", "Arms", "Legs", "Stomach", "Torso" }',s)
        self.assertIn('{ "Head", "Stomach", "Body", "Neck", "Arms", "Legs", "Torso" }',s)
        self.assertIn('{ "Blood", "Grenade", "Molotov", "Glass" }',s)
        self.assertNotIn('"Triggerbot##',s)
        self.assertIn('Add hotkey',s)

    def test_entities_chams_moved_without_duplicate(self):
        s=self.prepared()[u.VISUAL][1].decode()
        self.assertEqual(s.count('"Chams##'),1)
        self.assertIn('Chams##ent2_chams',s)
        self.assertEqual(s.count('static CPopup pop_chams'),1)
        for label in ['Corner Box##','Filled Box##','Step Circle##','Hit Sparks##']:
            self.assertNotIn(label,s)

    def test_world_removals_and_misc_preserved(self):
        s=self.prepared()[u.VISUAL][1].decode()
        for key in ['smoke','flash','blood','team','shake']:
            self.assertIn('&mason_ui::options.remove_'+key,s)
        self.assertIn('alice::tabs::callWorldMisc(child)',s)
        self.assertIn('Teleport##misc_tp',s)
        for label in ['Smoke Color##','Molotov Color##','Night Mode##']:
            self.assertNotIn(label,s)

    def test_dpi_frame_order_and_draw_hook(self):
        s=self.prepared()[u.OVERLAY][1].decode()
        names=['before_platform_frame()','ImGui_ImplWin32_NewFrame()','before_new_frame()','ImGui::NewFrame()','after_new_frame()','RenderMenu()','ImGui::Render()','scale_draw_data','ImGui_ImplDX11_RenderDrawData']
        offsets=[s.index(n) for n in names]
        self.assertEqual(offsets,sorted(offsets))
        ui=self.prepared()[u.UI][1].decode()
        self.assertIn('"DPI SCALE"',ui)
        self.assertIn('"75%", "100%", "125%", "150%", "175%", "200%"',ui)
        self.assertIn('menu_position.x = std::clamp',ui)

    def test_named_configs_and_quick_save_share_new_values(self):
        changes=self.prepared()
        s=changes[u.CONFIGS][1].decode()
        self.assertLess(s.index('mason_ui::write_options(o)'),s.index('f4(o, "accent"'))
        self.assertIn('mason_ui::read_option(key, val)',s)
        self.assertIn('mason_ui::reset_options()',s)
        self.assertIn('if (!f) { set_err("write failed"); return false; }',s)
        self.assertIn('configs::load_config("autosave")',changes[u.MENU][1].decode())

    def test_calls_are_not_mistaken_for_definitions(self):
        a,b=u.function_bounds(CONFIGS,'build_plaintext')
        self.assertIn('std::ostringstream',CONFIGS[a:b])
        a,b=u.function_bounds(CONFIGS,'apply_line')
        self.assertIn('g_aimbot_target',CONFIGS[a:b])

    def test_braces_in_comments_and_strings_are_ignored(self):
        source='void demo() { const char* s="}\\\"{"; /* } */ // }\n if(true) { work(); } }\n demo();'
        a,b=u.function_bounds(source,'demo')
        self.assertEqual(source[b],'}')
        self.assertIn('work()',source[a:b])

    def test_crlf_source_is_supported(self):
        make_project(self.root,u,crlf=True)
        changes=self.prepared()
        self.assertIn(b'\r\n',changes[u.UI][0])
        self.assertNotIn(b'\r\n',changes[u.UI][1])

    def test_dry_run_does_not_write_anything(self):
        before={str(p.relative_to(self.root)):p.read_bytes() for p in self.root.rglob('*') if p.is_file()}
        result=subprocess.run([sys.executable,str(PACKAGE/'apply_mason_ui_update.py'),'--root',str(self.root),'--dry-run'],capture_output=True,text=True)
        self.assertEqual(result.returncode,0,result.stdout+result.stderr)
        after={str(p.relative_to(self.root)):p.read_bytes() for p in self.root.rglob('*') if p.is_file()}
        self.assertEqual(before,after)

    def test_changed_source_rejected_before_writes(self):
        path=self.root/u.VISUAL
        path.write_text(VISUAL.replace('Show Fov##aim_fov_show','Different##aim_fov_show'))
        before=(self.root/u.UI).read_bytes()
        with self.assertRaises(u.UpdateError): self.prepared()
        self.assertEqual((self.root/u.UI).read_bytes(),before)

    def test_preservation_manifest_never_rewritten(self):
        path=self.root/'tests/ui_preservation_manifest.json'
        original=path.read_bytes()
        self.prepared()
        self.assertEqual(path.read_bytes(),original)
        data=json.loads(original); data[0]['modified']=False; path.write_text(json.dumps(data))
        with self.assertRaises(u.UpdateError): self.prepared()

    def test_successful_install_backup_and_idempotence(self):
        changes=self.prepared()
        backup=u.install(self.root,changes)
        for path,(before,after) in changes.items():
            self.assertEqual((self.root/path).read_bytes(),after)
            if before is not None: self.assertEqual((backup/path).read_bytes(),before)
        result=subprocess.run([sys.executable,str(PACKAGE/'apply_mason_ui_update.py'),'--root',str(self.root)],capture_output=True,text=True)
        self.assertEqual(result.returncode,0,result.stdout+result.stderr)
        self.assertIn('Already applied',result.stdout)

    def test_failed_write_rolls_back_all_sources(self):
        changes=self.prepared()
        replace=u.os.replace
        counter=0
        def fail_second(*args):
            nonlocal counter
            counter+=1
            if counter==2: raise OSError('simulated write failure')
            return replace(*args)
        with mock.patch.object(u.os,'replace',side_effect=fail_second):
            with self.assertRaises(OSError): u.install(self.root,changes)
        for path,(before,_) in changes.items():
            if before is None: self.assertFalse((self.root/path).exists())
            else: self.assertEqual((self.root/path).read_bytes(),before)

    def test_custom_sidebar_is_not_silently_overwritten(self):
        path=self.root/u.UI
        custom=UI.replace('{.name="Configs", .callback=tabs::callConfig}', '{.name="Extra", .callback=tabs::callExtra}, {.name="Configs", .callback=tabs::callConfig}')
        path.write_text(custom)
        with self.assertRaises(u.UpdateError): self.prepared()
        self.assertEqual(path.read_text(),custom)

    def test_receipt_write_failure_rolls_back_sources(self):
        changes=self.prepared()
        write_text=Path.write_text
        def fail_receipt(path,*args,**kwargs):
            if path.name=='receipt_pending.json': raise OSError('simulated receipt failure')
            return write_text(path,*args,**kwargs)
        with mock.patch.object(Path,'write_text',new=fail_receipt):
            with self.assertRaises(OSError): u.install(self.root,changes)
        for path,(before,_) in changes.items():
            if before is None: self.assertFalse((self.root/path).exists())
            else: self.assertEqual((self.root/path).read_bytes(),before)
        self.assertFalse((self.root/u.RECEIPT).exists())

    def test_file_changed_during_prepare_is_not_overwritten(self):
        changes=self.prepared()
        path=self.root/u.UI
        path.write_text('later user edit')
        with self.assertRaises(u.UpdateError): u.install(self.root,changes)
        self.assertEqual(path.read_text(),'later user edit')


if __name__=='__main__': unittest.main()
