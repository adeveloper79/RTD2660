@echo off
title RTD2660 Firmware Configurator
cd /d F:\Keil_v5\hh
python rtd_config_tool.py
if errorlevel 1 (
    echo.
    echo Application exited with an error.
    pause
)
