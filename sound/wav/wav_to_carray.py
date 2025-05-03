#!/usr/bin/env python3

import os
import struct

def wav_to_c_array(wav_filename):
    with open(wav_filename, 'rb') as f:
        content = f.read()

    # Parse WAV header to find PCM data offset
    if content[:4] != b'RIFF' or content[8:12] != b'WAVE':
        raise ValueError(f"{wav_filename} is not a valid WAV file.")
    
    # Find the "data" chunk
    data_chunk_offset = content.find(b'data')
    if data_chunk_offset == -1:
        raise ValueError(f"No 'data' chunk found in {wav_filename}.")
    
    # Extract PCM data
    pcm_data_offset = data_chunk_offset + 8
    pcm_data = content[pcm_data_offset:]

    # Rescale PCM data to 12-bit range (0 to 4096)
    # rescaled_data = [(byte / 255.0) * 4095 for byte in pcm_data]
    # rescaled_data = [int(value) for value in rescaled_data]  # Convert to integers
    rescaled_data = [byte << 4 for byte in pcm_data]  # 8-bit → 12-bit scale

    # Generate array name from file name
    array_name = os.path.splitext(os.path.basename(wav_filename))[0]

    # Write to .h file
    output_filename = f"{array_name}.h"
    with open(output_filename, 'w') as f:
        f.write(f"const uint16_t {array_name}_audio[] = {{\n")  # Use uint16_t for 12-bit data
        for i, value in enumerate(rescaled_data):
            if i % 12 == 0:
                f.write("    ")
            f.write(f"0x{value:03x}")  # Write as 3-digit hex for 12-bit values
            if i < len(rescaled_data) - 1:
                f.write(", ")
            if (i + 1) % 12 == 0:
                f.write("\n")
        f.write("\n};\n")
        f.write(f"const uint32_t {array_name}_audio_len = {len(rescaled_data)};\n")
    print(f"Generated {output_filename}")

# Process all .wav files in the current directory
for filename in os.listdir('.'):
    if filename.endswith('.wav'):
        try:
            wav_to_c_array(filename)
        except Exception as e:
            print(f"Error processing {filename}: {e}")
