#!/usr/bin/env python3
"""
RTD2660 Firmware Configurator & Tool
GUI utility to customize RTD2660 firmware parameters (Brightness, Backlight,
OSD, Button/ADC Mappings, Panel Selection, PinShare) and trigger 1-click builds.
"""

import os
import sys
import re
import shutil
import hashlib
import threading
import subprocess
from pathlib import Path

try:
    import customtkinter as ctk
    import tkinter as tk
    from tkinter import messagebox, filedialog
except ImportError:
    import tkinter as tk
    from tkinter import ttk, messagebox, filedialog
    ctk = None

# Workspace Paths
BASE_DIR = Path(__file__).resolve().parent
CORE_DIR = BASE_DIR / "Core"
HEADER_DIR = CORE_DIR / "header"
PCB_DIR = BASE_DIR / "PCB"
PANEL_DIR = BASE_DIR / "Panel"
KEYS_DIR = BASE_DIR / "Keys"
OUTPUT_DIR = BASE_DIR / "Output"

EEPROM_FILE = HEADER_DIR / "EepromDefaultCustomer.h"
PCB_FILE = PCB_DIR / "PCB_EYA.h"
KEY_SCAN_FILE = KEYS_DIR / "Key_PCB2660_003.c"
PANEL_CONFIG_FILE = PANEL_DIR / "Panel.h"
ST7701S_PANEL_FILE = PANEL_DIR / "PNL_TM043_ST7701S.h"
BUILD_SCRIPT = BASE_DIR / "build.py"

DESKTOP_DIR = Path(os.path.expanduser("~")) / "Desktop"
DESKTOP_BIN = DESKTOP_DIR / "FIRMWARE_256K.BIN"


def parse_val(text: str, default=0) -> int:
    text = str(text).strip()
    try:
        if text.startswith(("0x", "0X")):
            return int(text, 16)
        return int(text)
    except Exception:
        return default


def update_define(content: str, define_name: str, new_val_str: str) -> str:
    """Safely updates a #define parameter value while preserving comments and spacing."""
    pattern = rf'(#define\s+{re.escape(define_name)}\s+)[^\r\n/]*?\S(\s*(?://.*)?)$'
    if re.search(pattern, content, flags=re.MULTILINE):
        return re.sub(pattern, rf'\g<1>{new_val_str}\g<2>', content, count=1, flags=re.MULTILINE)
    return content


class RTDConfigManager:
    """Handles reading and writing firmware source definitions."""

    def __init__(self):
        self.load_all()

    def load_all(self):
        # 1. EEPROM / Display Quality & OSD Defaults
        if EEPROM_FILE.exists():
            txt = EEPROM_FILE.read_text(encoding="latin1", errors="ignore")
            m_bri = re.search(r'#define\s+_BRIGHTNESS_DEF\s+(\w+)', txt)
            self.brightness_def = parse_val(m_bri.group(1), 50) if m_bri else 50

            m_con = re.search(r'#define\s+_CONTRAST_DEF\s+(\w+)', txt)
            self.contrast_def = parse_val(m_con.group(1), 50) if m_con else 50

            m_bl = re.search(r'#define\s+_BACKLIGHT\s+(\w+)', txt)
            self.backlight = parse_val(m_bl.group(1), 80) if m_bl else 80

            m_lang = re.search(r'#define\s+_LANGUAGE\s+(\w+)', txt)
            self.language = parse_val(m_lang.group(1), 0) if m_lang else 0

            m_tout = re.search(r'#define\s+_OSD_TIMEOUT\s+(\w+)', txt)
            self.osd_timeout = parse_val(m_tout.group(1), 10) if m_tout else 10

            m_hpos = re.search(r'#define\s+_OSD_H_POSITION\s+(\w+)', txt)
            self.osd_h_pos = parse_val(m_hpos.group(1), 50) if m_hpos else 50

            m_vpos = re.search(r'#define\s+_OSD_V_POSITION\s+(\w+)', txt)
            self.osd_v_pos = parse_val(m_vpos.group(1), 50) if m_vpos else 50

            m_blend = re.search(r'#define\s+_OSD_BLEND\s+(\w+)', txt)
            self.osd_blend = parse_val(m_blend.group(1), 0) if m_blend else 0

            m_vol = re.search(r'#define\s+_VOLUME\s+(\w+)', txt)
            self.volume = parse_val(m_vol.group(1), 32) if m_vol else 32
        else:
            self.brightness_def = 50
            self.contrast_def = 50
            self.backlight = 80
            self.language = 0
            self.osd_timeout = 10
            self.osd_h_pos = 50
            self.osd_v_pos = 50
            self.osd_blend = 0
            self.volume = 32

        # 2. PCB / Button Mappings & Pin 65
        if PCB_FILE.exists():
            txt = PCB_FILE.read_text(encoding="latin1", errors="ignore")
            self.keys = {}
            for k in range(1, 8):
                m = re.search(rf'#define\s+bAD0_Key_{k}\s+([0-9]+)', txt)
                self.keys[k] = parse_val(m.group(1), 255) if m else 255

            m_p65 = re.search(r'#define\s+_PIN_65\s+\(([0-9]+)\s*&', txt)
            self.pin65_mode = int(m_p65.group(1)) if m_p65 else 0

            m_pwm_inv = re.search(r'#define\s+_BACKLIGHT_PWM_INVERSE\s+(\w+)', txt)
            self.backlight_pwm_inverse = m_pwm_inv.group(1) if m_pwm_inv else "_DISABLE"
        else:
            self.keys = {1: 0, 2: 43, 3: 255, 4: 28, 5: 10, 6: 18, 7: 255}
            self.pin65_mode = 0
            self.backlight_pwm_inverse = "_DISABLE"

        # 3. Key Scan Step / Tolerance
        if KEY_SCAN_FILE.exists():
            txt = KEY_SCAN_FILE.read_text(encoding="latin1", errors="ignore")
            m_step = re.search(r'#define\s+AD_KEY_STEP\s+([0-9]+)', txt)
            self.ad_key_step = int(m_step.group(1)) if m_step else 3
        else:
            self.ad_key_step = 3

        # 4. Panel Selection
        if PANEL_CONFIG_FILE.exists():
            txt = PANEL_CONFIG_FILE.read_text(encoding="latin1", errors="ignore")
            m_pnl = re.search(r'#define\s+_PANEL_TYPE\s+(\w+)', txt)
            self.panel_type = m_pnl.group(1) if m_pnl else "_PNL_TM043_ST7701S"
        else:
            self.panel_type = "_PNL_TM043_ST7701S"

        # 5. ST7701S Red/Blue Swap
        if ST7701S_PANEL_FILE.exists():
            txt = ST7701S_PANEL_FILE.read_text(encoding="latin1", errors="ignore")
            m_swap = re.search(r'#define\s+_DISP_RB_SWAP\s+([^\\r\\n/]+)', txt)
            if m_swap and "(1 << 5)" in m_swap.group(1):
                self.st7701s_rb_swap = True
            else:
                self.st7701s_rb_swap = False
        else:
            self.st7701s_rb_swap = False

    def save_all(self):
        """Writes current config values back into the source files."""
        # 1. Update EepromDefaultCustomer.h
        if EEPROM_FILE.exists():
            txt = EEPROM_FILE.read_text(encoding="latin1", errors="ignore")
            txt = update_define(txt, "_BRIGHTNESS_DEF", f"0x{self.brightness_def:02x}")
            txt = update_define(txt, "_CONTRAST_DEF", f"0x{self.contrast_def:02x}")
            txt = update_define(txt, "_BRIGHTNESS", str(self.brightness_def))
            txt = update_define(txt, "_CONTRAST", str(self.contrast_def))
            txt = update_define(txt, "_BACKLIGHT", str(self.backlight))
            txt = update_define(txt, "_LANGUAGE", f"0x{self.language:02x}")
            txt = update_define(txt, "_OSD_TIMEOUT", f"0x{self.osd_timeout:02x}")
            txt = update_define(txt, "_OSD_H_POSITION", f"0x{self.osd_h_pos:02x}")
            txt = update_define(txt, "_OSD_V_POSITION", f"0x{self.osd_v_pos:02x}")
            txt = update_define(txt, "_OSD_BLEND", f"0x{self.osd_blend:02x}")
            txt = update_define(txt, "_VOLUME", f"0x{self.volume:02x}")
            EEPROM_FILE.write_text(txt, encoding="latin1")

        # 2. Update PCB_EYA.h
        if PCB_FILE.exists():
            txt = PCB_FILE.read_text(encoding="latin1", errors="ignore")
            for k, val in self.keys.items():
                pattern = rf'(#define\s+bAD0_Key_{k}\s+)[0-9]+'
                txt = re.sub(pattern, rf'\g<1>{val}', txt, count=1)

            txt = update_define(txt, "_PIN_65", f"({self.pin65_mode} & 0x07)")
            txt = update_define(txt, "_BACKLIGHT_PWM_INVERSE", self.backlight_pwm_inverse)
            PCB_FILE.write_text(txt, encoding="latin1")

        # 3. Update Key_PCB2660_003.c
        if KEY_SCAN_FILE.exists():
            txt = KEY_SCAN_FILE.read_text(encoding="latin1", errors="ignore")
            txt = update_define(txt, "AD_KEY_STEP", str(self.ad_key_step))
            KEY_SCAN_FILE.write_text(txt, encoding="latin1")

        # 4. Update Panel.h
        if PANEL_CONFIG_FILE.exists():
            txt = PANEL_CONFIG_FILE.read_text(encoding="latin1", errors="ignore")
            txt = update_define(txt, "_PANEL_TYPE", self.panel_type)
            PANEL_CONFIG_FILE.write_text(txt, encoding="latin1")

        # 5. Update PNL_TM043_ST7701S.h
        if ST7701S_PANEL_FILE.exists():
            txt = ST7701S_PANEL_FILE.read_text(encoding="latin1", errors="ignore")
            swap_val = "(1 << 5)" if self.st7701s_rb_swap else "0"
            pattern = r'(#define\s+_DISP_RB_SWAP\s+)[^\r\n/]+'
            txt = re.sub(pattern, rf'\g<1>{swap_val}', txt, count=1)
            ST7701S_PANEL_FILE.write_text(txt, encoding="latin1")


class RTDConfigApp:
    def __init__(self, root):
        self.root = root
        self.root.title("RTD2660 Firmware Configurator - EYA Board")
        self.root.geometry("1040x780")
        self.root.minsize(880, 660)

        self.cfg = RTDConfigManager()
        self.is_building = False

        self._setup_ui()
        self._load_values_into_ui()

    def _setup_ui(self):
        ctk.set_appearance_mode("Dark")
        ctk.set_default_color_theme("blue")

        # Main Grid Layout
        self.root.grid_columnconfigure(0, weight=1)
        self.root.grid_rowconfigure(1, weight=1)

        # 1. Header Frame
        header_frame = ctk.CTkFrame(self.root, corner_radius=8, fg_color=('#1e293b', '#0f172a'))
        header_frame.grid(row=0, column=0, padx=15, pady=(12, 6), sticky="ew")
        header_frame.grid_columnconfigure(1, weight=1)

        title_lbl = ctk.CTkLabel(
            header_frame,
            text="⚡ RTD2660 Firmware Configurator",
            font=ctk.CTkFont(size=20, weight="bold"),
            text_color="#38bdf8"
        )
        title_lbl.grid(row=0, column=0, padx=15, pady=10, sticky="w")

        self.badge_lbl = ctk.CTkLabel(
            header_frame,
            text="Panel: TM043 ST7701S (480x800) | Board: EYA",
            font=ctk.CTkFont(size=12, weight="bold"),
            fg_color="#0284c7",
            corner_radius=6,
            padx=12,
            pady=5
        )
        self.badge_lbl.grid(row=0, column=2, padx=15, pady=10, sticky="e")

        # 2. Tabs Frame
        self.tabview = ctk.CTkTabview(self.root, corner_radius=8)
        self.tabview.grid(row=1, column=0, padx=15, pady=6, sticky="nsew")

        tab_display = self.tabview.add("🎛️ Backlight & Brightness")
        tab_osd = self.tabview.add("📺 OSD Settings")
        tab_keys = self.tabview.add("🔘 Button Mapping (ADC)")
        tab_panel = self.tabview.add("📱 Panel & Hardware")
        tab_build = self.tabview.add("🔨 Build & Output")

        self._build_tab_display(tab_display)
        self._build_tab_osd(tab_osd)
        self._build_tab_keys(tab_keys)
        self._build_tab_panel(tab_panel)
        self._build_tab_build(tab_build)

        # 3. Bottom Action Bar
        bottom_frame = ctk.CTkFrame(self.root, corner_radius=8, fg_color=('#1e293b', '#0f172a'))
        bottom_frame.grid(row=2, column=0, padx=15, pady=(6, 12), sticky="ew")
        bottom_frame.grid_columnconfigure(0, weight=1)

        self.status_lbl = ctk.CTkLabel(
            bottom_frame,
            text="Ready. Synchronized with F:\\Keil_v5\\hh sources.",
            font=ctk.CTkFont(size=13),
            text_color="#94a3b8"
        )
        self.status_lbl.grid(row=0, column=0, padx=15, pady=10, sticky="w")

        btn_reload = ctk.CTkButton(
            bottom_frame,
            text="🔄 Reload",
            width=100,
            command=self.on_reload,
            fg_color="#475569",
            hover_color="#334155"
        )
        btn_reload.grid(row=0, column=1, padx=6, pady=10)

        btn_save = ctk.CTkButton(
            bottom_frame,
            text="💾 Save Configurations",
            width=160,
            command=self.on_save,
            fg_color="#0284c7",
            hover_color="#0369a1"
        )
        btn_save.grid(row=0, column=2, padx=6, pady=10)

        btn_build = ctk.CTkButton(
            bottom_frame,
            text="🚀 1-Click Build",
            width=140,
            command=self.on_start_build,
            fg_color="#16a34a",
            hover_color="#15803d"
        )
        btn_build.grid(row=0, column=3, padx=(6, 15), pady=10)

    # -------------------------------------------------------------
    # Tab 1: Backlight & Display Quality
    # -------------------------------------------------------------
    def _build_tab_display(self, parent):
        parent.grid_columnconfigure((0, 1), weight=1)

        # Left Card: Sliders
        left_card = ctk.CTkFrame(parent, corner_radius=8)
        left_card.grid(row=0, column=0, padx=10, pady=10, sticky="nsew")
        left_card.grid_columnconfigure(1, weight=1)

        ctk.CTkLabel(left_card, text="Default Luminance & Contrast", font=ctk.CTkFont(size=15, weight="bold")).grid(
            row=0, column=0, columnspan=3, padx=15, pady=(15, 10), sticky="w"
        )

        # Backlight
        ctk.CTkLabel(left_card, text="Default Backlight (0-100%):").grid(row=1, column=0, padx=15, pady=10, sticky="w")
        self.slider_bl = ctk.CTkSlider(left_card, from_=0, to=100, number_of_steps=100, command=self._on_bl_slider)
        self.slider_bl.grid(row=1, column=1, padx=10, pady=10, sticky="ew")
        self.lbl_bl_val = ctk.CTkLabel(left_card, text="80%", width=45, font=ctk.CTkFont(weight="bold"))
        self.lbl_bl_val.grid(row=1, column=2, padx=(5, 15), pady=10)

        # Brightness
        ctk.CTkLabel(left_card, text="Default Brightness (0-100%):").grid(row=2, column=0, padx=15, pady=10, sticky="w")
        self.slider_bri = ctk.CTkSlider(left_card, from_=0, to=100, number_of_steps=100, command=self._on_bri_slider)
        self.slider_bri.grid(row=2, column=1, padx=10, pady=10, sticky="ew")
        self.lbl_bri_val = ctk.CTkLabel(left_card, text="50%", width=45, font=ctk.CTkFont(weight="bold"))
        self.lbl_bri_val.grid(row=2, column=2, padx=(5, 15), pady=10)

        # Contrast
        ctk.CTkLabel(left_card, text="Default Contrast (0-100%):").grid(row=3, column=0, padx=15, pady=10, sticky="w")
        self.slider_con = ctk.CTkSlider(left_card, from_=0, to=100, number_of_steps=100, command=self._on_con_slider)
        self.slider_con.grid(row=3, column=1, padx=10, pady=10, sticky="ew")
        self.lbl_con_val = ctk.CTkLabel(left_card, text="50%", width=45, font=ctk.CTkFont(weight="bold"))
        self.lbl_con_val.grid(row=3, column=2, padx=(5, 15), pady=10)

        # Quick Preset Buttons
        preset_frame = ctk.CTkFrame(left_card, fg_color="transparent")
        preset_frame.grid(row=4, column=0, columnspan=3, padx=15, pady=15, sticky="ew")
        preset_frame.grid_columnconfigure((0, 1, 2), weight=1)

        ctk.CTkButton(preset_frame, text="Dim (40% BL)", command=lambda: self._set_bl_preset(40), fg_color="#334155").grid(row=0, column=0, padx=4)
        ctk.CTkButton(preset_frame, text="Standard (80% BL)", command=lambda: self._set_bl_preset(80), fg_color="#334155").grid(row=0, column=1, padx=4)
        ctk.CTkButton(preset_frame, text="Max (100% BL)", command=lambda: self._set_bl_preset(100), fg_color="#334155").grid(row=0, column=2, padx=4)

        # Right Card: Hardware & Inverter Settings
        right_card = ctk.CTkFrame(parent, corner_radius=8)
        right_card.grid(row=0, column=1, padx=10, pady=10, sticky="nsew")
        right_card.grid_columnconfigure(1, weight=1)

        ctk.CTkLabel(right_card, text="Backlight Inverter Hardware", font=ctk.CTkFont(size=15, weight="bold")).grid(
            row=0, column=0, columnspan=2, padx=15, pady=(15, 10), sticky="w"
        )

        ctk.CTkLabel(right_card, text="PWM Polarity:").grid(row=1, column=0, padx=15, pady=10, sticky="w")
        self.seg_pwm_inv = ctk.CTkSegmentedButton(
            right_card,
            values=["Normal (_DISABLE)", "Inverted (_ENABLE)"],
            command=self._on_pwm_inv_change
        )
        self.seg_pwm_inv.grid(row=1, column=1, padx=15, pady=10, sticky="ew")

        ctk.CTkLabel(right_card, text="PWM Pin/Channel:").grid(row=2, column=0, padx=15, pady=10, sticky="w")
        self.lbl_pwm_pin = ctk.CTkLabel(right_card, text="Pin 100 (RTD PWM4) [Active]", text_color="#38bdf8")
        self.lbl_pwm_pin.grid(row=2, column=1, padx=15, pady=10, sticky="w")

        ctk.CTkLabel(right_card, text="Enable Pin (BL ON):").grid(row=3, column=0, padx=15, pady=10, sticky="w")
        self.lbl_bl_en = ctk.CTkLabel(right_card, text="Pin 64 (bLIGHTPOWER = HIGH)", text_color="#38bdf8")
        self.lbl_bl_en.grid(row=3, column=1, padx=15, pady=10, sticky="w")

        info_box = ctk.CTkTextbox(right_card, height=130, corner_radius=6, font=ctk.CTkFont(size=12))
        info_box.grid(row=4, column=0, columnspan=2, padx=15, pady=15, sticky="ew")
        info_box.insert(
            "1.0",
            "Hardware Inverter Notes:\n"
            "• Default Backlight controls duty cycle at boot time.\n"
            "• PWM4 runs at standard RTD frequency scaled from 24MHz crystal.\n"
            "• On the EYA board, Pin 64 actively asserts backlight high on valid video.\n"
            "• Normal polarity: 100% = max high duty cycle; Inverted = active low."
        )
        info_box.configure(state="disabled")

    def _set_bl_preset(self, val):
        self.slider_bl.set(val)
        self._on_bl_slider(val)

    def _on_bl_slider(self, val):
        self.lbl_bl_val.configure(text=f"{int(val)}%")
        self.cfg.backlight = int(val)

    def _on_bri_slider(self, val):
        self.lbl_bri_val.configure(text=f"{int(val)}%")
        self.cfg.brightness_def = int(val)

    def _on_con_slider(self, val):
        self.lbl_con_val.configure(text=f"{int(val)}%")
        self.cfg.contrast_def = int(val)

    def _on_pwm_inv_change(self, val):
        self.cfg.backlight_pwm_inverse = "_ENABLE" if "Inverted" in val else "_DISABLE"

    # -------------------------------------------------------------
    # Tab 2: OSD Settings
    # -------------------------------------------------------------
    def _build_tab_osd(self, parent):
        parent.grid_columnconfigure((0, 1), weight=1)

        card = ctk.CTkFrame(parent, corner_radius=8)
        card.grid(row=0, column=0, columnspan=2, padx=10, pady=10, sticky="nsew")
        card.grid_columnconfigure((1, 3), weight=1)

        ctk.CTkLabel(card, text="On-Screen Display (OSD) Preferences", font=ctk.CTkFont(size=15, weight="bold")).grid(
            row=0, column=0, columnspan=4, padx=15, pady=(15, 10), sticky="w"
        )

        # OSD Timeout
        ctk.CTkLabel(card, text="OSD Timeout (seconds):").grid(row=1, column=0, padx=15, pady=10, sticky="w")
        self.slider_tout = ctk.CTkSlider(card, from_=5, to=60, number_of_steps=55, command=self._on_tout_slider)
        self.slider_tout.grid(row=1, column=1, padx=10, pady=10, sticky="ew")
        self.lbl_tout_val = ctk.CTkLabel(card, text="10s", width=40, font=ctk.CTkFont(weight="bold"))
        self.lbl_tout_val.grid(row=1, column=2, padx=5, pady=10)

        # Language
        ctk.CTkLabel(card, text="Default Language:").grid(row=2, column=0, padx=15, pady=10, sticky="w")
        self.opt_lang = ctk.CTkOptionMenu(
            card,
            values=["English (0x00)", "Simplified Chinese (0x01)", "Traditional Chinese (0x02)", "French (0x03)", "German (0x04)"],
            command=self._on_lang_change
        )
        self.opt_lang.grid(row=2, column=1, padx=10, pady=10, sticky="w")

        # OSD Transparency (Blend)
        ctk.CTkLabel(card, text="Transparency (Blend):").grid(row=3, column=0, padx=15, pady=10, sticky="w")
        self.opt_blend = ctk.CTkSegmentedButton(
            card,
            values=["Opaque (0%)", "25%", "50%", "75%"],
            command=self._on_blend_change
        )
        self.opt_blend.grid(row=3, column=1, padx=10, pady=10, sticky="ew")

        # Position H & V
        ctk.CTkLabel(card, text="OSD H-Position (%):").grid(row=4, column=0, padx=15, pady=10, sticky="w")
        self.slider_hpos = ctk.CTkSlider(card, from_=0, to=100, number_of_steps=100, command=self._on_hpos_slider)
        self.slider_hpos.grid(row=4, column=1, padx=10, pady=10, sticky="ew")
        self.lbl_hpos_val = ctk.CTkLabel(card, text="50%", width=40, font=ctk.CTkFont(weight="bold"))
        self.lbl_hpos_val.grid(row=4, column=2, padx=5, pady=10)

        ctk.CTkLabel(card, text="OSD V-Position (%):").grid(row=5, column=0, padx=15, pady=10, sticky="w")
        self.slider_vpos = ctk.CTkSlider(card, from_=0, to=100, number_of_steps=100, command=self._on_vpos_slider)
        self.slider_vpos.grid(row=5, column=1, padx=10, pady=10, sticky="ew")
        self.lbl_vpos_val = ctk.CTkLabel(card, text="50%", width=40, font=ctk.CTkFont(weight="bold"))
        self.lbl_vpos_val.grid(row=5, column=2, padx=5, pady=10)

        # Volume
        ctk.CTkLabel(card, text="Default Audio Volume:").grid(row=6, column=0, padx=15, pady=10, sticky="w")
        self.slider_vol = ctk.CTkSlider(card, from_=0, to=100, number_of_steps=100, command=self._on_vol_slider)
        self.slider_vol.grid(row=6, column=1, padx=10, pady=10, sticky="ew")
        self.lbl_vol_val = ctk.CTkLabel(card, text="32%", width=40, font=ctk.CTkFont(weight="bold"))
        self.lbl_vol_val.grid(row=6, column=2, padx=5, pady=10)

    def _on_tout_slider(self, val):
        self.lbl_tout_val.configure(text=f"{int(val)}s")
        self.cfg.osd_timeout = int(val)

    def _on_lang_change(self, val):
        idx = int(re.search(r'0x([0-9a-fA-F]+)', val).group(1), 16)
        self.cfg.language = idx

    def _on_blend_change(self, val):
        mapping = {"Opaque (0%)": 0, "25%": 1, "50%": 2, "75%": 3}
        self.cfg.osd_blend = mapping.get(val, 0)

    def _on_hpos_slider(self, val):
        self.lbl_hpos_val.configure(text=f"{int(val)}%")
        self.cfg.osd_h_pos = int(val)

    def _on_vpos_slider(self, val):
        self.lbl_vpos_val.configure(text=f"{int(val)}%")
        self.cfg.osd_v_pos = int(val)

    def _on_vol_slider(self, val):
        self.lbl_vol_val.configure(text=f"{int(val)}%")
        self.cfg.volume = int(val)

    # -------------------------------------------------------------
    # Tab 3: Button Mapping (ADC Keypad)
    # -------------------------------------------------------------
    def _build_tab_keys(self, parent):
        parent.grid_columnconfigure((0, 1), weight=1)

        # Left Card: Button Map Table
        card_table = ctk.CTkFrame(parent, corner_radius=8)
        card_table.grid(row=0, column=0, padx=10, pady=10, sticky="nsew")
        card_table.grid_columnconfigure(1, weight=1)

        ctk.CTkLabel(card_table, text="ADC Resistor Ladder Key Mapping", font=ctk.CTkFont(size=15, weight="bold")).grid(
            row=0, column=0, columnspan=3, padx=15, pady=(15, 10), sticky="w"
        )

        btn_specs = [
            (1, "Power Key", "bAD0_Key_1"),
            (5, "Menu Key", "bAD0_Key_5"),
            (6, "Right (+) Key", "bAD0_Key_6"),
            (4, "Left (-) Key", "bAD0_Key_4"),
            (2, "Source Key", "bAD0_Key_2"),
            (3, "Sleep Key", "bAD0_Key_3"),
            (7, "Reset / Mute", "bAD0_Key_7"),
        ]

        self.key_entries = {}
        for row_idx, (k_id, k_label, macro) in enumerate(btn_specs, start=1):
            ctk.CTkLabel(card_table, text=f"{k_label} ({macro}):").grid(row=row_idx, column=0, padx=15, pady=6, sticky="w")
            entry = ctk.CTkEntry(card_table, width=70)
            entry.grid(row=row_idx, column=1, padx=10, pady=6, sticky="w")
            self.key_entries[k_id] = entry

            calc_lbl = ctk.CTkLabel(card_table, text="0.00 V", font=ctk.CTkFont(size=11), text_color="#94a3b8")
            calc_lbl.grid(row=row_idx, column=2, padx=(5, 15), pady=6, sticky="w")
            entry.bind("<KeyRelease>", lambda e, en=entry, cl=calc_lbl: self._calc_voltage(en, cl))

        btn_default_keys = ctk.CTkButton(
            card_table,
            text="⚡ Reset to EYA Board Defaults",
            command=self._on_reset_key_defaults,
            fg_color="#334155",
            hover_color="#1e293b"
        )
        btn_default_keys.grid(row=len(btn_specs)+1, column=0, columnspan=3, padx=15, pady=15, sticky="ew")

        # Right Card: ADC Tolerance and Scan info
        card_info = ctk.CTkFrame(parent, corner_radius=8)
        card_info.grid(row=0, column=1, padx=10, pady=10, sticky="nsew")
        card_info.grid_columnconfigure(1, weight=1)

        ctk.CTkLabel(card_info, text="ADC Channel & Tolerance Window", font=ctk.CTkFont(size=15, weight="bold")).grid(
            row=0, column=0, columnspan=3, padx=15, pady=(15, 10), sticky="w"
        )

        ctk.CTkLabel(card_info, text="ADC Channel:").grid(row=1, column=0, padx=15, pady=10, sticky="w")
        ctk.CTkLabel(card_info, text="MCU_ADC3 (Pin 83 on EYA)", text_color="#38bdf8", font=ctk.CTkFont(weight="bold")).grid(row=1, column=1, padx=15, pady=10, sticky="w")

        ctk.CTkLabel(card_info, text="Tolerance Window:").grid(row=2, column=0, padx=15, pady=10, sticky="w")
        self.slider_step = ctk.CTkSlider(card_info, from_=1, to=15, number_of_steps=14, command=self._on_step_slider)
        self.slider_step.grid(row=2, column=1, padx=10, pady=10, sticky="ew")
        self.lbl_step_val = ctk.CTkLabel(card_info, text="±3", width=35, font=ctk.CTkFont(weight="bold"))
        self.lbl_step_val.grid(row=2, column=2, padx=5, pady=10)

        step_desc = ctk.CTkTextbox(card_info, height=220, corner_radius=6, font=ctk.CTkFont(size=12))
        step_desc.grid(row=3, column=0, columnspan=3, padx=15, pady=15, sticky="ew")
        step_desc.insert(
            "1.0",
            "How RTD2660 ADC Key Detection Works:\n\n"
            "• Each physical button pulls the ADC3 line to a specific resistor voltage division.\n"
            "• When pressed: |ADC_Reading - Target| < AD_KEY_STEP triggers the key message.\n"
            "• Target = 255 disables that key (e.g. Sleep / Mute).\n"
            "• Power Button is set to 0 (direct GND pull on Pin 83 / ADC3).\n"
            "• If buttons occasionally trigger wrong menus, decrease tolerance step or remeasure ladder voltage."
        )
        step_desc.configure(state="disabled")

    def _calc_voltage(self, entry, label):
        val = parse_val(entry.get(), -1)
        if 0 <= val <= 255:
            volts = (val / 255.0) * 3.3
            label.configure(text=f"~{volts:.2f} V")
        else:
            label.configure(text="Disabled")

    def _on_step_slider(self, val):
        self.lbl_step_val.configure(text=f"±{int(val)}")
        self.cfg.ad_key_step = int(val)

    def _on_reset_key_defaults(self):
        defaults = {1: 0, 2: 43, 3: 255, 4: 28, 5: 10, 6: 18, 7: 255}
        for k, v in defaults.items():
            if k in self.key_entries:
                self.key_entries[k].delete(0, "end")
                self.key_entries[k].insert(0, str(v))
        self.slider_step.set(3)
        self.lbl_step_val.configure(text="±3")
        self.cfg.ad_key_step = 3
        self.status_lbl.configure(text="Restored EYA board button defaults.")

    # -------------------------------------------------------------
    # Tab 4: Panel & Hardware Selection
    # -------------------------------------------------------------
    def _build_tab_panel(self, parent):
        parent.grid_columnconfigure((0, 1), weight=1)

        card_panel = ctk.CTkFrame(parent, corner_radius=8)
        card_panel.grid(row=0, column=0, padx=10, pady=10, sticky="nsew")
        card_panel.grid_columnconfigure(1, weight=1)

        ctk.CTkLabel(card_panel, text="Display Panel Profile", font=ctk.CTkFont(size=15, weight="bold")).grid(
            row=0, column=0, columnspan=2, padx=15, pady=(15, 10), sticky="w"
        )

        ctk.CTkLabel(card_panel, text="Active Panel:").grid(row=1, column=0, padx=15, pady=10, sticky="w")
        self.opt_panel = ctk.CTkOptionMenu(
            card_panel,
            values=[
                'Tianma 4.3" 480x800 ST7701S (_PNL_TM043_ST7701S)',
                '5.0" 480x854 ST7701 Pure TTL (_PNL_TM043_ST7701)',
                'BOE 7.0" 1024x600 (_PNL_LVDS_HV070WSA)',
                'Toshiba/AT 10.2" 800x480 (_PNL_AT102TN03)',
                'HannStar 7.0" 800x480 (_PNL_HSD070IDW1)'
            ],
            command=self._on_panel_change
        )
        self.opt_panel.grid(row=1, column=1, padx=15, pady=10, sticky="ew")

        ctk.CTkLabel(card_panel, text="ST7701S Red/Blue Swap:").grid(row=2, column=0, padx=15, pady=10, sticky="w")
        self.switch_rb_swap = ctk.CTkSwitch(
            card_panel,
            text="Swap Red & Blue Channels",
            command=self._on_rb_swap_toggle
        )
        self.switch_rb_swap.grid(row=2, column=1, padx=15, pady=10, sticky="w")

        # Right Card: Pin 65 and Rails
        card_pins = ctk.CTkFrame(parent, corner_radius=8)
        card_pins.grid(row=0, column=1, padx=10, pady=10, sticky="nsew")
        card_pins.grid_columnconfigure(1, weight=1)

        ctk.CTkLabel(card_pins, text="PCB Rail & Pin Sharing", font=ctk.CTkFont(size=15, weight="bold")).grid(
            row=0, column=0, columnspan=2, padx=15, pady=(15, 10), sticky="w"
        )

        ctk.CTkLabel(card_pins, text="Pin 65 (3.3V Logic):").grid(row=1, column=0, padx=15, pady=10, sticky="w")
        self.switch_pin65 = ctk.CTkSwitch(
            card_pins,
            text="Disable Pin 65 (Safe for ST7701S)",
            command=self._on_pin65_toggle
        )
        self.switch_pin65.grid(row=1, column=1, padx=15, pady=10, sticky="w")

        pin_info = ctk.CTkTextbox(card_pins, height=200, corner_radius=6, font=ctk.CTkFont(size=12))
        pin_info.grid(row=2, column=0, columnspan=2, padx=15, pady=15, sticky="ew")
        pin_info.insert(
            "1.0",
            "Hardware Safety Notes:\n\n"
            "• ST7701S 4.3\" panel requires Pin 65 (P1.1) to be high-Z / 0V to prevent panel latchup.\n"
            "• Red/Blue Swap: When disabled (0), RTD2660 outputs normal RGB order. When enabled (1<<5), RTD2660 inverts to BGR.\n"
            "• The 5.0\" EHD-5008 panel uses reverse pin traces (BGR), while the 4.3\" ST7701S uses standard RGB."
        )
        pin_info.configure(state="disabled")

    def _on_panel_change(self, val):
        if "_PNL_TM043_ST7701S" in val:
            self.cfg.panel_type = "_PNL_TM043_ST7701S"
            self.badge_lbl.configure(text="Panel: TM043 ST7701S (480x800) | Board: EYA")
        elif "_PNL_TM043_ST7701" in val:
            self.cfg.panel_type = "_PNL_TM043_ST7701"
            self.badge_lbl.configure(text="Panel: EHD-5008 5.0\" (480x854) | Board: EYA")
        elif "_PNL_LVDS_HV070WSA" in val:
            self.cfg.panel_type = "_PNL_LVDS_HV070WSA"
            self.badge_lbl.configure(text="Panel: HV070WSA (1024x600) | Board: EYA")
        elif "_PNL_AT102TN03" in val:
            self.cfg.panel_type = "_PNL_AT102TN03"
            self.badge_lbl.configure(text="Panel: AT102TN03 (800x480) | Board: EYA")
        elif "_PNL_HSD070IDW1" in val:
            self.cfg.panel_type = "_PNL_HSD070IDW1"
            self.badge_lbl.configure(text="Panel: HSD070IDW1 (800x480) | Board: EYA")

    def _on_rb_swap_toggle(self):
        self.cfg.st7701s_rb_swap = bool(self.switch_rb_swap.get())

    def _on_pin65_toggle(self):
        # When switch is ON -> Disabled (0)
        # When switch is OFF -> Enabled Push-Pull (2)
        self.cfg.pin65_mode = 0 if self.switch_pin65.get() else 2

    # -------------------------------------------------------------
    # Tab 5: Build & Output
    # -------------------------------------------------------------
    def _build_tab_build(self, parent):
        parent.grid_columnconfigure(0, weight=1)
        parent.grid_rowconfigure(1, weight=1)

        top_ctrl = ctk.CTkFrame(parent, corner_radius=8)
        top_ctrl.grid(row=0, column=0, padx=10, pady=10, sticky="ew")
        top_ctrl.grid_columnconfigure(2, weight=1)

        btn_run_build = ctk.CTkButton(
            top_ctrl,
            text="▶ Start Compilation & Packaging",
            command=self.on_start_build,
            fg_color="#16a34a",
            hover_color="#15803d",
            font=ctk.CTkFont(weight="bold")
        )
        btn_run_build.grid(row=0, column=0, padx=15, pady=10)

        btn_open_out = ctk.CTkButton(
            top_ctrl,
            text="📂 Open Output Folder",
            command=self.on_open_output,
            fg_color="#475569",
            hover_color="#334155"
        )
        btn_open_out.grid(row=0, column=1, padx=10, pady=10)

        self.lbl_build_status = ctk.CTkLabel(
            top_ctrl,
            text="Build Status: Ready",
            font=ctk.CTkFont(size=13, weight="bold"),
            text_color="#94a3b8"
        )
        self.lbl_build_status.grid(row=0, column=2, padx=15, pady=10, sticky="e")

        # Terminal Console
        self.term_box = ctk.CTkTextbox(
            parent,
            corner_radius=8,
            font=ctk.CTkFont(family="Consolas", size=11),
            fg_color="#020617",
            text_color="#f1f5f9"
        )
        self.term_box.grid(row=1, column=0, padx=10, pady=(0, 10), sticky="nsew")
        self.term_box.insert("1.0", "Compiler output will be streamed here in real-time...\n")

    # -------------------------------------------------------------
    # UI Value Synchronization
    # -------------------------------------------------------------
    def _load_values_into_ui(self):
        # Display tab
        self.slider_bl.set(self.cfg.backlight)
        self.lbl_bl_val.configure(text=f"{self.cfg.backlight}%")

        self.slider_bri.set(self.cfg.brightness_def)
        self.lbl_bri_val.configure(text=f"{self.cfg.brightness_def}%")

        self.slider_con.set(self.cfg.contrast_def)
        self.lbl_con_val.configure(text=f"{self.cfg.contrast_def}%")

        inv_str = "Inverted (_ENABLE)" if self.cfg.backlight_pwm_inverse == "_ENABLE" else "Normal (_DISABLE)"
        self.seg_pwm_inv.set(inv_str)

        # OSD tab
        self.slider_tout.set(self.cfg.osd_timeout)
        self.lbl_tout_val.configure(text=f"{self.cfg.osd_timeout}s")

        for lang_str in self.opt_lang._values:
            if f"0x{self.cfg.language:02x}" in lang_str:
                self.opt_lang.set(lang_str)
                break

        blend_names = {0: "Opaque (0%)", 1: "25%", 2: "50%", 3: "75%"}
        self.opt_blend.set(blend_names.get(self.cfg.osd_blend, "Opaque (0%)"))

        self.slider_hpos.set(self.cfg.osd_h_pos)
        self.lbl_hpos_val.configure(text=f"{self.cfg.osd_h_pos}%")

        self.slider_vpos.set(self.cfg.osd_v_pos)
        self.lbl_vpos_val.configure(text=f"{self.cfg.osd_v_pos}%")

        self.slider_vol.set(self.cfg.volume)
        self.lbl_vol_val.configure(text=f"{self.cfg.volume}%")

        # Keys tab
        for k, entry in self.key_entries.items():
            val = self.cfg.keys.get(k, 255)
            entry.delete(0, "end")
            entry.insert(0, str(val))
            idx = list(self.key_entries.keys()).index(k) + 1
            lbl_candidates = entry.master.grid_slaves(row=idx, column=2)
            if lbl_candidates:
                self._calc_voltage(entry, lbl_candidates[0])

        self.slider_step.set(self.cfg.ad_key_step)
        self.lbl_step_val.configure(text=f"±{self.cfg.ad_key_step}")

        # Panel tab
        for opt in self.opt_panel._values:
            if self.cfg.panel_type in opt:
                self.opt_panel.set(opt)
                self._on_panel_change(opt)
                break

        if self.cfg.st7701s_rb_swap:
            self.switch_rb_swap.select()
        else:
            self.switch_rb_swap.deselect()

        if self.cfg.pin65_mode == 0:
            self.switch_pin65.select()
        else:
            self.switch_pin65.deselect()

    # -------------------------------------------------------------
    # User Actions
    # -------------------------------------------------------------
    def on_reload(self):
        self.cfg.load_all()
        self._load_values_into_ui()
        self.status_lbl.configure(text="Configuration reloaded from disk.")
        messagebox.showinfo("Reloaded", "Configuration values successfully reloaded from project files.")

    def on_save(self):
        # Read back key entries
        for k, entry in self.key_entries.items():
            self.cfg.keys[k] = parse_val(entry.get(), 255)

        try:
            self.cfg.save_all()
            self.status_lbl.configure(text="Saved all configurations to source files.")
            messagebox.showinfo("Saved", "All configuration parameters written to source files successfully.")
        except Exception as e:
            self.status_lbl.configure(text=f"Error saving files: {e}")
            messagebox.showerror("Save Error", f"Failed to save configurations:\n{e}")

    def on_start_build(self):
        if self.is_building:
            return

        # Read back keys
        for k, entry in self.key_entries.items():
            self.cfg.keys[k] = parse_val(entry.get(), 255)
        self.cfg.save_all()

        # Switch to build tab
        self.tabview.set("🔨 Build & Output")
        self.term_box.delete("1.0", "end")
        self.term_box.insert("end", f"Starting build process using {BUILD_SCRIPT}...\n\n")

        self.is_building = True
        self.lbl_build_status.configure(text="Build Status: Compiling...", text_color="#facc15")
        self.status_lbl.configure(text="Compiling Keil C51 sources and generating ROM binary...")

        threading.Thread(target=self._build_worker, daemon=True).start()

    def _build_worker(self):
        cmd = [sys.executable, str(BUILD_SCRIPT)]
        process = subprocess.Popen(
            cmd,
            cwd=str(BASE_DIR),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1
        )

        for line in iter(process.stdout.readline, ''):
            self.term_box.insert("end", line)
            self.term_box.see("end")

        process.stdout.close()
        return_code = process.wait()

        self.root.after(0, lambda: self._on_build_finished(return_code))

    def _on_build_finished(self, return_code):
        self.is_building = False
        if return_code == 0:
            bin_source = OUTPUT_DIR / "FIRMWARE_256K.BIN"
            sha256_hash = ""
            if bin_source.exists():
                shutil.copy(bin_source, DESKTOP_BIN)
                h = hashlib.sha256()
                h.update(bin_source.read_bytes())
                sha256_hash = h.hexdigest().upper()

            self.lbl_build_status.configure(text="Build Status: SUCCESS (0 Errors)", text_color="#4ade80")
            self.status_lbl.configure(text="Firmware built and copied to Desktop!")
            self.term_box.insert("end", f"\n[SUCCESS] Copied to: {DESKTOP_BIN}\n")
            self.term_box.insert("end", f"[CHECKSUM] SHA256: {sha256_hash}\n")
            self.term_box.see("end")

            messagebox.showinfo(
                "Build Succeeded",
                f"Firmware successfully built!\n\n"
                f"Binary: {DESKTOP_BIN}\n"
                f"SHA256: {sha256_hash[:16]}..."
            )
        else:
            self.lbl_build_status.configure(text="Build Status: FAILED", text_color="#f87171")
            self.status_lbl.configure(text="Compilation failed. Check the terminal log.")
            messagebox.showerror("Build Error", "Firmware compilation failed. Please inspect the log.")

    def on_open_output(self):
        if OUTPUT_DIR.exists():
            os.startfile(str(OUTPUT_DIR))


def main():
    root = ctk.CTk()
    app = RTDConfigApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()
