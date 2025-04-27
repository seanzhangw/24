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

    # Generate array name from file name
    array_name = os.path.splitext(os.path.basename(wav_filename))[0]

    # Write to .h file
    output_filename = f"{array_name}.h"
    with open(output_filename, 'w') as f:
        f.write(f"const uint8_t {array_name}_audio[] = {{\n")
        for i, byte in enumerate(pcm_data):
            if i % 12 == 0:
                f.write("    ")
            f.write(f"0x{byte:02x}")
            if i < len(pcm_data) - 1:
                f.write(", ")
            if (i + 1) % 12 == 0:
                f.write("\n")
        f.write("\n};\n")
        f.write(f"const uint32_t {array_name}_audio_len = {len(pcm_data)};\n")
    print(f"Generated {output_filename}")

# Process all .wav files in the current directory
for filename in os.listdir('.'):
    if filename.endswith('.wav'):
        try:
            wav_to_c_array(filename)
        except Exception as e:
            print(f"Error processing {filename}: {e}")
