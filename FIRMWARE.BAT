@echo off
rem Split bank object into separate bank files
F:\Keil_v5\C51\BIN\OC51.exe Output\KR266Xseries

rem Generate Intel HEX files for each bank
F:\Keil_v5\C51\BIN\OH51.exe Output\KR266Xseries.B00 HEXFILE(Output\KR266Xseries.H00)
F:\Keil_v5\C51\BIN\OH51.exe Output\KR266Xseries.B01 HEXFILE(Output\KR266Xseries.H01)
F:\Keil_v5\C51\BIN\OH51.exe Output\KR266Xseries.B02 HEXFILE(Output\KR266Xseries.H02)

rem Package into unified binary and pad to standard SPI flash sizes
python create_bin.py Output\FIRMWARE.BIN Output\KR266Xseries.H00 Output\KR266Xseries.H01 Output\KR266Xseries.H02
python pad_firmware.py Output\FIRMWARE.BIN Output\FIRMWARE_512K.BIN
