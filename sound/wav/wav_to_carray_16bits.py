#!/usr/bin/env python3

import os
import struct

def wav_to_c_array(wav_filename):
    with open(wav_filename, 'rb') as f:
        content = f.read()

    # Parse WAV header
    if content[:4] != b'RIFF' or content[8:12] != b'WAVE':
        raise ValueError(f"{wav_filename} is not a valid WAV file.")
    
    # Find "data" chunk
    data_chunk_offset = content.find(b'data')
    if data_chunk_offset == -1:
        raise ValueError(f"No 'data' chunk found in {wav_filename}.")
    
    pcm_data_offset = data_chunk_offset + 8
    pcm_data = content[pcm_data_offset:]

    # Rescale to 12-bit and OR with DAC_config_chan_A (0x3000)
    dac_config = 0x3000
    dac_data = [(dac_config | (byte << 4)) & 0xffff for byte in pcm_data]

    # Output file
    array_name = os.path.splitext(os.path.basename(wav_filename))[0]
    output_filename = f"{array_name}.h"
    
    with open(output_filename, 'w') as f:
        f.write("#include <stdint.h>\n\n")
        f.write("#define DAC_config_chan_A 0b0011000000000000\n\n")
        f.write(f"const uint16_t {array_name}_audio[] = {{\n")
        for i, value in enumerate(dac_data):
            if i % 12 == 0:
                f.write("    ")
            f.write(f"0x{value:04x}")
            if i < len(dac_data) - 1:
                f.write(", ")
            if (i + 1) % 12 == 0:
                f.write("\n")
        f.write("\n};\n")
        f.write(f"const uint32_t {array_name}_audio_len = {len(dac_data)};\n")

    print(f"Generated {output_filename}")

# Process all .wav files in the directory
for filename in os.listdir('.'):
    if filename.endswith('.wav'):
        try:
            wav_to_c_array(filename)
        except Exception as e:
            print(f"Error processing {filename}: {e}")
