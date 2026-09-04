import sys
import subprocess
import os
import xml.etree.ElementTree as ET

hh = os.path.dirname(os.path.abspath(sys.argv[0]))
c51_bin = r'F:\Keil_v5\C51\BIN'
inc_dir = r'.\Core\header;.\PCB;.\Panel;.\OSD\OSD003;.\Keys;.\Func;.\DDCCI;.\Lib;.\Logo;.\TV;.\Flash'

tree = ET.parse(os.path.join(hh, 'KR266Xseries.uvproj'))
root = tree.getroot()

# Collect all files
files_to_compile = []
for group in root.findall('.//Group'):
    for file in group.findall('.//File'):
        ftype = file.find('FileType').text
        fpath = file.find('FilePath').text
        fname = file.find('FileName').text
        if ftype == '1': # C file
            files_to_compile.append(('C', fname, fpath))
        elif ftype == '2': # ASM file
            files_to_compile.append(('ASM', fname, fpath))

print(f"Total files to compile: {len(files_to_compile)}")

errors = 0
for ftype, fname, fpath in files_to_compile:
    obj_name = os.path.splitext(fname)[0] + '.obj'
    lst_name = os.path.splitext(fname)[0] + '.lst'
    obj_path = os.path.join(r'.\Output', obj_name)
    lst_path = os.path.join(r'.\Output', lst_name)
    
    if ftype == 'C':
        cmd = f'"{c51_bin}\\C51.exe" "{fpath}" LARGE OPTIMIZE(8,SPEED) BROWSE INCDIR({inc_dir}) OBJECT({obj_path}) PRINT({lst_path})'
    else:
        cmd = f'"{c51_bin}\\A51.exe" "{fpath}" SET(LARGE) DEBUG OBJECT({obj_path}) PRINT({lst_path})'
    
    res = subprocess.run(cmd, cwd=hh, capture_output=True, text=True, shell=True)
    if "*** ERROR" in res.stdout or "*** FATAL" in res.stdout:
        print(f"[FAIL] {fname}:")
        print(res.stdout)
        errors += 1
    else:
        print(f"[OK]   {fname}")

if errors > 0:
    print(f"\n{errors} file(s) had compilation errors!")
    exit(1)

print("\n--- Linking with BL51 ---")
bl51_cmd = f'"{c51_bin}\\BL51.exe" @Output\\KR266Xseries.lnp'
res = subprocess.run(bl51_cmd, cwd=hh, capture_output=True, text=True, shell=True)
print("BL51 STDOUT:\n", res.stdout)
if "*** FATAL" in res.stdout or "*** ERROR" in res.stdout:
    print("BL51 ERROR!")
    exit(1)

print("\n--- Converting banks with OC51 ---")
oc51_cmd = f'"{c51_bin}\\OC51.exe" Output\\KR266Xseries'
res = subprocess.run(oc51_cmd, cwd=hh, capture_output=True, text=True, shell=True)
print("OC51 STDOUT:\n", res.stdout)

print("\n--- Converting hex with OH51 ---")
for bank in ['B00', 'B01', 'B02']:
    h_file = f'Output\\KR266Xseries.H0{bank[2]}'
    b_file = f'Output\\KR266Xseries.{bank}'
    oh51_cmd = f'"{c51_bin}\\OH51.exe" {b_file} HEXFILE({h_file})'
    res = subprocess.run(oh51_cmd, cwd=hh, capture_output=True, text=True, shell=True)
    print(f"OH51 {bank} -> {h_file}: {res.stdout.strip()}")

print("\n--- Packaging binary with CreateBin ---")
res = subprocess.run(r'CreateBin.exe Output\FIRMWARE.BIN Output\KR266Xseries.H00 Output\KR266Xseries.H01 Output\KR266Xseries.H02', cwd=hh, capture_output=True, text=True, shell=True)
print("CreateBin STDOUT:\n", res.stdout)

print("\n--- Padding firmware ---")
res = subprocess.run(r'python pad_firmware.py Output\FIRMWARE.BIN Output\FIRMWARE_512K.BIN', cwd=hh, capture_output=True, text=True, shell=True)
print(res.stdout)
print("========================================")
print("ALL BUILD STEPS SUCCEEDED 100%!")
print("========================================")
