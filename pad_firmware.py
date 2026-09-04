#!/usr/bin/env python3
"""
RTD2660 Firmware Padding Tool
==============================
Pads a compiled FIRMWARE.BIN to exactly 512KB (524288 bytes)
with 0xFF fill bytes, ready for flashing to SPI flash chip.

Usage:
    python pad_firmware.py                        # auto: FIRMWARE.BIN -> FIRMWARE_512K.BIN
    python pad_firmware.py input.bin              # pads  input.bin    -> input_512K.bin
    python pad_firmware.py input.bin output.bin   # pads  input.bin    -> output.bin
"""

import sys
import os

FLASH_SIZE = 524288  # 512KB
FILL_BYTE  = 0xFF

def check_8051(data):
    if len(data) < 8:
        return False, "File too small"
    if data[0] != 0x02:
        return False, f"Reset vector is not LJMP (0x{data[0]:02X})"
    target = (data[1] << 8) | data[2]
    if target < 0x100:
        return False, f"Suspicious LJMP target 0x{target:04X}"
    return True, f"LJMP 0x{target:04X}"

def pad_firmware(input_path, output_path):
    with open(input_path, 'rb') as f:
        data = bytearray(f.read())

    # RTD2660 bootloader signature bytes required at offsets 0x03 to 0x0A
    sig = bytes([0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x10, 0x04, 0x04])
    if len(data) >= 11:
        if data[3:11] != sig:
            print("Patching RTD2660 bootloader signature at offset 0x03...")
            data[3:11] = sig

    size = len(data)
    print(f"Input  : {input_path}")
    print(f"Size   : {size:,} bytes ({size/1024:.1f} KB)")

    ok, msg = check_8051(data)
    if ok:
        print(f"Format : Valid Keil C51 8051 binary ({msg})")
    else:
        print(f"WARNING: {msg} — may not be a valid RTD2660 binary!")

    # Generate 256K binary (Banks 0-3 / 256KB)
    pad_256 = 262144 - size if size < 262144 else 0
    bin_256 = bytearray(data[:262144] + bytes([FILL_BYTE] * pad_256))
    
    # Ensure Bank 3 (0x30000..0x31E0C) has the common code and reset vector
    common_block = data[0:0x1E0C]
    if len(bin_256) >= 0x31E0C:
        bin_256[0x30000:0x31E0C] = common_block

    # Ensure all banks (0, 1, 2, 3) have the required RTD2660 bootloader signature at offset +0x03
    for bank in range(4):
        b_off = bank * 0x10000 + 3
        if len(bin_256) >= b_off + 8:
            bin_256[b_off:b_off+8] = sig

    out_256 = output_path.replace("512K", "256K").replace("512k", "256k")
    with open(out_256, 'wb') as f:
        f.write(bin_256)
    print(f"Output : {out_256} ({len(bin_256):,} bytes / 256 KB)")

    if size > FLASH_SIZE:
        print(f"ERROR  : File ({size:,} bytes) exceeds flash size ({FLASH_SIZE:,} bytes)!")
        sys.exit(1)

    pad_bytes = FLASH_SIZE - size if size < FLASH_SIZE else 0
    padded = data[:FLASH_SIZE] + bytes([FILL_BYTE] * pad_bytes)

    with open(output_path, 'wb') as f:
        f.write(padded)

    print(f"Output : {output_path} ({len(padded):,} bytes / 512 KB)")

    # Multi-bank check
    if len(padded) >= 0x30000:
        b0 = padded[0:3]
        b1 = padded[0x10000:0x10003]
        b2 = padded[0x20000:0x20003]
        if b0 == b1 == b2 and b0[0] == 0x02:
            print("Banks  : Bank0/1/2 reset vectors match - multi-bank OK")

    print("\nDone! Flash the output file with CH341A / NeoProgrammer / AsProgrammer.")

def main():
    if len(sys.argv) == 1:
        input_path  = "FIRMWARE.BIN"
        output_path = "FIRMWARE_512K.BIN"
    elif len(sys.argv) == 2:
        input_path = sys.argv[1]
        base, ext = os.path.splitext(input_path)
        output_path = base + "_512K" + (ext if ext else ".bin")
    elif len(sys.argv) == 3:
        input_path  = sys.argv[1]
        output_path = sys.argv[2]
    else:
        print(__doc__)
        sys.exit(1)

    if not os.path.exists(input_path):
        print(f"ERROR: Input file '{input_path}' not found.")
        sys.exit(1)

    pad_firmware(input_path, output_path)

if __name__ == "__main__":
    main()
