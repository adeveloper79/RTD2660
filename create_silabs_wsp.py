import os
import xml.etree.ElementTree as ET

hh_dir = r"F:\Keil_v5\hh"
ws_src = r"F:\Keil_v5\wsp\ws.wsp"
out_wsp = os.path.join(hh_dir, "KR266Xseries.wsp")

# Read groups and files from uvproj
tree = ET.parse(os.path.join(hh_dir, "KR266Xseries.uvproj"))
root = tree.getroot()

groups = []
for group in root.findall('.//Group'):
    gname = group.find('GroupName').text
    # sanitize group name for silabs (no spaces or hyphens)
    gname_clean = gname.replace('-', '_').replace(' ', '_')
    files = []
    for file in group.findall('.//File'):
        ftype = file.find('FileType').text
        fpath = file.find('FilePath').text
        # We only want source C/ASM files in Silicon Labs IDE file tree
        if ftype in ('1', '2'):
            # Convert forward slashes to Windows backslashes
            clean_path = fpath.replace('/', '\\')
            if clean_path.startswith('.\\'):
                clean_path = clean_path[2:]
            files.append(clean_path)
    if files:
        groups.append((gname_clean, files))

print(f"Loaded {len(groups)} groups with total {sum(len(f) for _, f in groups)} files.")

# Read template ws.wsp up to [WorkState_v1_1.Vendor]
with open(ws_src, 'r', encoding='utf-8', errors='ignore') as f:
    ws_text = f.read()

# Find the header section [WorkState_v1_1]
# We need to rewrite the ptn_Child entries of [WorkState_v1_1]
header_items = [
    "DockState", "ToolBarMgr", "Frames", "SerialPort", "StepInc",
    "DisassemblyAutoView", "Watch0Base", "Watch1Base", "Vendor", "HITECHPRO",
    "Assembler", "AssFlag", "AssFormat", "Compiler", "CompFlag",
    "CompFormat", "RunOptimizer", "Linker", "LinkFlag", "LinkFormat",
    "PreprocFlag", "PreprocFormat", "DisList", "DisOP", "ParseErr",
    "Download", "AutoSave", "UseMake", "ErrorFormat", "ErrorString",
    "MultiDeviceJTAG", "BankingPN", "OutputFile", "MakeFile", "HexGenerator",
    "GenHex", "CExt", "IDEVer", "ECProtocol", "C2StrobeTime",
    "Adapter", "AdapterSN", "USB Adapter Power", "PFiles", "AFiles",
    "CFiles", "LFiles", "BankMap", "Folders"
]

for gname, _ in groups:
    header_items.append(gname)
header_items.append("CygnalCloseFileFlag")

header_block = "[WorkState_v1_1]\n"
for i, item in enumerate(header_items, 1):
    header_block += f"ptn_Child{i}={item}\n"

# Extract the middle chunk: from [WorkState_v1_1.DockState] up to [WorkState_v1_1.Vendor]
dock_start = ws_text.find("[WorkState_v1_1.DockState]")
vendor_start = ws_text.find("[WorkState_v1_1.Vendor]")
middle_dock_block = ws_text[dock_start:vendor_start]

# Toolchain configuration block
c51_bin = r"F:\Keil_v5\C51\BIN"
inc_dirs = r".\Core\header;.\PCB;.\Panel;.\OSD\OSD003;.\Keys;.\Func;.\DDCCI;.\Lib;.\Logo;.\TV;.\Flash"

toolchain_block = f"""[WorkState_v1_1.Vendor]
Vendor=0
[WorkState_v1_1.HITECHPRO]
HITECHPRO=1
[WorkState_v1_1.Assembler]
Assembler={c51_bin}\\A51.EXE
[WorkState_v1_1.AssFlag]
AssFlag=SET(LARGE) DEBUG
[WorkState_v1_1.AssFormat]
AssFormat=<Executable Name> <Input File(s)> <Flags> 
[WorkState_v1_1.Compiler]
Compiler={c51_bin}\\c51.exe
[WorkState_v1_1.CompFlag]
CompFlag=LARGE OPTIMIZE(8,SPEED) BROWSE INCDIR({inc_dirs})
[WorkState_v1_1.CompFormat]
CompFormat=<Executable Name> <Input File(s)> <Flags> 
[WorkState_v1_1.RunOptimizer]
RunOptimizer=0
[WorkState_v1_1.Linker]
Linker={c51_bin}\\BL51.EXE
[WorkState_v1_1.LinkFlag]
LinkFlag=@Output\\KR266Xseries.lnp
[WorkState_v1_1.LinkFormat]
LinkFormat=<Executable Name> <Flags>
[WorkState_v1_1.PreprocFlag]
PreprocFlag=
[WorkState_v1_1.PreprocFormat]
PreprocFormat=
[WorkState_v1_1.DisList]
DisList=1
[WorkState_v1_1.DisOP]
DisOP=1
[WorkState_v1_1.ParseErr]
ParseErr=1
[WorkState_v1_1.Download]
Download=0
[WorkState_v1_1.AutoSave]
AutoSave=1
[WorkState_v1_1.UseMake]
UseMake=1
[WorkState_v1_1.ErrorFormat]
ErrorFormat=CS CS CS CS CS LN
[WorkState_v1_1.ErrorString]
ErrorString=%s %s %s %s %s %d
[WorkState_v1_1.MultiDeviceJTAG]
DevicesAfterTarget=0
DevicesBeforeTarget=0
BitsAfterTargetIR=0
BitsBeforeTargetIR=0
[WorkState_v1_1.BankingPN]
BankingPN=32767
[WorkState_v1_1.OutputFile]
OutputFile=Output\\KR266Xseries
[WorkState_v1_1.MakeFile]
MakeFile=build.bat
[WorkState_v1_1.HexGenerator]
HexGenerator=
[WorkState_v1_1.GenHex]
GenHex=0
[WorkState_v1_1.CExt]
CExt=.obj
[WorkState_v1_1.IDEVer]
IDEVer=5.50.00
[WorkState_v1_1.ECProtocol]
ECProtocol=1
[WorkState_v1_1.C2StrobeTime]
C2StrobeTime=0
[WorkState_v1_1.Adapter]
Adapter=3
[WorkState_v1_1.AdapterSN]
AdapterSN=
[WorkState_v1_1.USB Adapter Power]
USB Adapter Power=0
[WorkState_v1_1.PFiles]
[WorkState_v1_1.AFiles]
[WorkState_v1_1.CFiles]
[WorkState_v1_1.LFiles]
[WorkState_v1_1.BankMap]
"""

# Folders definition block
folders_block = "[WorkState_v1_1.Folders]\n"
if groups:
    folders_block += "ptn_Child1=FolderName\n"

current_sub = "FolderName"
for i, (gname, _) in enumerate(groups):
    folders_block += f"[WorkState_v1_1.Folders.{current_sub}]\n"
    folders_block += f"FolderName={gname}\n"
    if i < len(groups) - 1:
        folders_block += "ptn_Child1=FolderName\n"
        current_sub += ".FolderName"

# Groups and Files block
groups_block = ""
for gname, file_list in groups:
    groups_block += f"[WorkState_v1_1.{gname}]\n"
    groups_block += "ptn_Child1=FolderFlags\n"
    if file_list:
        groups_block += "ptn_Child2=FileName\n"
    groups_block += f"[WorkState_v1_1.{gname}.FolderFlags]\n"
    groups_block += "FolderFlags=0\n"
    
    current_fn = "FileName"
    for j, fpath in enumerate(file_list):
        groups_block += f"[WorkState_v1_1.{gname}.{current_fn}]\n"
        groups_block += f"FileName={fpath}\n"
        if j < len(file_list) - 1:
            groups_block += "ptn_Child1=FileName\n"
            current_fn += ".FileName"

full_wsp_content = header_block + middle_dock_block + toolchain_block + folders_block + groups_block

with open(out_wsp, 'w', encoding='utf-8') as f:
    f.write(full_wsp_content)

print(f"Successfully generated {out_wsp} ({len(full_wsp_content)} chars)")
