#!/usr/bin/env python3
"""
Open-source Intel HEX to Multi-Bank Binary Merger for RTD2660.
Replaces proprietary CreateBin.exe.
Stitches banked Intel HEX files (e.g. H00, H01, H02) into a contiguous
FIRMWARE.BIN matching the RTD2660 bank layout.

Usage:
    python create_bin.py <output.bin> <bank0.hex> <bank1.hex> [bank2.hex ...]
"""

import sys
import os

def parse_intel_hex(hex_path):
    """Parses a 64KB banked Intel HEX file into a record list and calculates max address."""
    records = []
    max_addr = 0
    with open(hex_path, 'r', encoding='latin-1') as f:
        for line in f:
            line = line.strip()
            if not line.startswith(':'):
                continue
            byte_count = int(line[1:3], 16)
            address = int(line[3:7], 16)
            record_type = int(line[7:9], 16)
            
            if record_type == 0:  # Data record
                data = bytes.fromhex(line[9:9 + byte_count * 2])
                records.append((address, data))
                if address + byte_count > max_addr:
                    max_addr = address + byte_count
            elif record_type == 1:  # End of File
                break
                
    return records, max_addr

def merge_hex_files_to_bin(output_path, hex_paths):
    """
    Merges banked Intel HEX files into a binary file.
    - Each bank before the final bank is 64KB (65536 bytes).
    - Unwritten code areas up to max_addr are filled with 0x00.
    - Tail areas from max_addr to 64KB are padded with 0xFF.
    - The final bank is truncated at its highest used address.
    """
    output_data = bytearray()
    
    for i, hex_path in enumerate(hex_paths):
        is_last = (i == len(hex_paths) - 1)
        if not os.path.exists(hex_path):
            raise FileNotFoundError(f"HEX file not found: {hex_path}")
            
        records, max_addr = parse_intel_hex(hex_path)
        
        # 64KB bank initialized with 0xFF padding
        bank = bytearray([0xFF] * 65536)
        
        # Areas up to max_addr default to 0x00 for unassigned code holes
        for addr in range(max_addr):
            bank[addr] = 0x00
            
        # Place hex record bytes
        for addr, data in records:
            bank[addr:addr + len(data)] = data
            
        if is_last:
            output_data.extend(bank[:max_addr])
        else:
            output_data.extend(bank)
            
    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    with open(output_path, 'wb') as f:
        f.write(output_data)
        
    print(f"Created file: {output_path}. Size: 0x{len(output_data):X} ({len(output_data):,} bytes)")
    return output_data

def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
        
    output_path = sys.argv[1]
    hex_paths = sys.argv[2:]
    merge_hex_files_to_bin(output_path, hex_paths)

if __name__ == '__main__':
    main()
