#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Pre-generate audio waveform PCM buffers as const C arrays in Flash.
This eliminates all runtime software floating-point math on ESP32-C3 and guarantees instant boot.
"""
import math
import os

OUT_DIR = os.path.join(os.path.dirname(__file__), "../main")
SAMPLE_RATE = 16000

def gen_click():
    # 40ms chirp
    samples = int(SAMPLE_RATE * 0.040)
    data = []
    two_pi = 2 * math.pi
    for i in range(samples):
        t = i / SAMPLE_RATE
        env = math.exp(-45.0 * t)
        freq = 900.0 + 800.0 * (1.0 - i / samples)
        val = env * math.sin(two_pi * freq * t)
        data.append(int(max(-32767, min(32767, val * 12000))))
    return data

def gen_fan():
    # 70ms wooden tap
    samples = int(SAMPLE_RATE * 0.070)
    data = []
    two_pi = 2 * math.pi
    for i in range(samples):
        t = i / SAMPLE_RATE
        env = math.exp(-38.0 * t)
        val = env * (0.7 * math.sin(two_pi * 440.0 * t) + 0.3 * math.sin(two_pi * 880.0 * t))
        data.append(int(max(-32767, min(32767, val * 11000))))
    return data

def gen_qi():
    # 200ms bell chime (A5 + E6)
    samples = int(SAMPLE_RATE * 0.200)
    data = []
    two_pi = 2 * math.pi
    for i in range(samples):
        t = i / SAMPLE_RATE
        env = math.exp(-14.0 * t)
        val = env * (0.6 * math.sin(two_pi * 880.0 * t) + 0.4 * math.sin(two_pi * 1320.0 * t))
        data.append(int(max(-32767, min(32767, val * 12000))))
    return data

def gen_tian():
    # 350ms multi-harmonic chord (C5, E5, G5, C6)
    samples = int(SAMPLE_RATE * 0.350)
    data = []
    two_pi = 2 * math.pi
    for i in range(samples):
        t = i / SAMPLE_RATE
        env = math.exp(-8.0 * t)
        chord = (
            0.30 * math.sin(two_pi * 523.25 * t) +
            0.25 * math.sin(two_pi * 659.25 * t) +
            0.25 * math.sin(two_pi * 783.99 * t) +
            0.20 * math.sin(two_pi * 1046.50 * t)
        )
        data.append(int(max(-32767, min(32767, env * chord * 15000))))
    return data

def write_c_array(f, name, data):
    f.write(f"const int16_t {name}[{len(data)}] = {{\n")
    for i in range(0, len(data), 12):
        chunk = data[i:i+12]
        f.write("    " + ", ".join(f"{x}" for x in chunk) + ",\n")
    f.write("};\n\n")

def main():
    click = gen_click()
    fan = gen_fan()
    qi = gen_qi()
    tian = gen_tian()

    h_path = os.path.join(OUT_DIR, "tianshang_sfx_data.h")
    c_path = os.path.join(OUT_DIR, "tianshang_sfx_data.c")

    with open(h_path, "w", encoding="utf-8") as f:
        f.write("#ifndef TIANSHANG_SFX_DATA_H\n#define TIANSHANG_SFX_DATA_H\n\n#include <stdint.h>\n#include <stddef.h>\n\n")
        f.write(f"#define SFX_CLICK_SAMPLES {len(click)}\n")
        f.write(f"#define SFX_FAN_SAMPLES   {len(fan)}\n")
        f.write(f"#define SFX_QI_SAMPLES    {len(qi)}\n")
        f.write(f"#define SFX_TIAN_SAMPLES  {len(tian)}\n\n")
        f.write("extern const int16_t s_pcm_click[SFX_CLICK_SAMPLES];\n")
        f.write("extern const int16_t s_pcm_fan[SFX_FAN_SAMPLES];\n")
        f.write("extern const int16_t s_pcm_qi[SFX_QI_SAMPLES];\n")
        f.write("extern const int16_t s_pcm_tian[SFX_TIAN_SAMPLES];\n\n")
        f.write("#endif\n")

    with open(c_path, "w", encoding="utf-8") as f:
        f.write('#include "tianshang_sfx_data.h"\n\n')
        write_c_array(f, "s_pcm_click", click)
        write_c_array(f, "s_pcm_fan", fan)
        write_c_array(f, "s_pcm_qi", qi)
        write_c_array(f, "s_pcm_tian", tian)

    print(f"Generated {h_path} and {c_path}")
    print(f"Sizes: click={len(click)*2}B, fan={len(fan)*2}B, qi={len(qi)*2}B, tian={len(tian)*2}B")

if __name__ == "__main__":
    main()
