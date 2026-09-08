"""Offline source and PE checks. Python standard library only."""
from pathlib import Path
import argparse, hashlib, json, re, struct
ROOT=Path(__file__).resolve().parents[1]
P=ROOT/'product'

def check_source():
    for name in ['features','mem','menu','backend/minhook','backend/glow','backend/clipper']:
        assert not (P/name).exists(), 'Old runtime directory: '+name
    forbidden=re.compile(r'\b(?:OpenProcess|ReadProcessMemory|WriteProcessMemory|CreateRemoteThread|VirtualAllocEx|VirtualProtectEx|DeviceIoControl|CreateToolhelp32Snapshot|SendInput|mouse_event|keybd_event|SetWindowDisplayAffinity|LoadColliderMapByIndex|InitWorldColorHook|MH_\w+|WinHttp\w*|InternetOpen\w*)\s*\(')
    for f in P.rglob('*'):
        if not f.is_file(): continue
        assert f.suffix.lower() not in {'.dll','.exe','.sys','.obj','.pdb','.rar','.zip','.7z'},str(f)
        if f.suffix not in {'.cpp','.h','.hpp'} or any(x in f.parts for x in ['backend','thirdparty','include']):continue
        t=f.read_text(encoding='utf-8')
        assert 'flowering' not in t.lower(),str(f)
        assert not forbidden.search(t),str(f)
        assert 'DllMain' not in t,str(f)
        assert not re.search(r'#include\s+"(?:features/|mem/|memory\.h|scanner\.h)',t),str(f)
    assert 'wWinMain' in (P/'entry.cpp').read_text()
    for item in json.loads((ROOT/'tests/ui_preservation_manifest.json').read_text()):
        f=P/item['path']; assert f.is_file(),item['path']
        if not item['modified']: assert hashlib.sha256(f.read_bytes()).hexdigest()==item['original_sha256'],item['path']
    code=(P/'assets/logo_bytes.hpp').read_text()
    logo=bytes(int(x,16) for x in re.findall(r'0x([0-9a-f]{2})',code))
    assert logo==(ROOT/'assets/logo_transparent.png').read_bytes()
    wm=(P/'everness/src/alice/ui/widgets/watermark/watermark.cpp').read_text()
    assert '"Admin"' in wm and 'ShadeVertsLinearColorGradientKeepAlpha' in wm
    assert 'neutral_rgb' in (P/'everness/src/graphics/graphics.cpp').read_text()
    for f in re.findall(r'(?m)^\s+(product/[^\s]+\.cpp|assets/[^\s]+\.rc)$',(ROOT/'CMakeLists.txt').read_text()):assert (ROOT/f).is_file(),f
    print('PASS: standalone sources, original UI files, embedded logo, Admin watermark.')

def check_pe(path):
    b=path.read_bytes(); assert b[:2]==b'MZ','Not a PE file'
    pe=struct.unpack_from('<I',b,0x3c)[0];assert b[pe:pe+4]==b'PE\0\0'
    machine,nsec=struct.unpack_from('<HH',b,pe+4)
    optsize,flags=struct.unpack_from('<HH',b,pe+20);o=pe+24
    assert machine==0x8664 and not flags&0x2000,'Expected x64 EXE, not DLL'
    assert struct.unpack_from('<H',b,o)[0]==0x20b
    assert struct.unpack_from('<H',b,o+68)[0]==2,'Expected Windows GUI subsystem'
    sections=[]
    for i in range(nsec):
        vs,rva,rs,rp=struct.unpack_from('<IIII',b,o+optsize+i*40+8);sections.append((rva,max(vs,rs),rp))
    def offset(rva):
        for base,size,raw in sections:
            if base<=rva<base+size:return raw+rva-base
        raise AssertionError('Invalid import RVA')
    def string(pos):return b[pos:b.index(b'\0',pos)].decode('ascii','replace')
    irva=struct.unpack_from('<I',b,o+120)[0]
    badfn={'OpenProcess','ReadProcessMemory','WriteProcessMemory','CreateRemoteThread','VirtualAllocEx','VirtualProtectEx','DeviceIoControl','SendInput','mouse_event','keybd_event','SetWindowDisplayAffinity','CreateToolhelp32Snapshot','CreateServiceA','CreateServiceW','StartServiceA','StartServiceW'}
    count=0
    if irva:
        d=offset(irva)
        while any(b[d:d+20]):
            lut,_,_,name,iat=struct.unpack_from('<IIIII',b,d)
            dll=string(offset(name));assert dll.lower() not in {'winhttp.dll','wininet.dll','ws2_32.dll'},dll
            t=offset(lut or iat)
            while True:
                v=struct.unpack_from('<Q',b,t)[0]
                if not v:break
                if not v&(1<<63):
                    fn=string(offset(v)+2);assert fn not in badfn,fn;count+=1
                t+=8
            d+=20
    print(f'PASS: x64 GUI EXE, {count} imports checked. SHA256: '+hashlib.sha256(b).hexdigest())

if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--exe',type=Path);args=ap.parse_args()
    check_source()
    if args.exe:check_pe(args.exe)
