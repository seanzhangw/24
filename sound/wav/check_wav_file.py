import wave

wav_filename1 = 'deal_cards.wav'

def inspect_wav_file(wav_filename):
    with wave.open(wav_filename, 'rb') as wav_file:
        # Print basic information about the WAV file
        print(f"File Name: {wav_filename}")
        print(f"Channels: {wav_file.getnchannels()}")
        print(f"Sample Width (bytes): {wav_file.getsampwidth()}")
        print(f"Frame Rate (Hz): {wav_file.getframerate()}")
        print(f"Number of Frames: {wav_file.getnframes()}")
        print(f"Compression Type: {wav_file.getcomptype()}")
        print(f"Compression Name: {wav_file.getcompname()}")

# Inspect WAV file
inspect_wav_file(wav_filename1)

# Extract and print all PCM data
with wave.open(wav_filename1, 'rb') as wav_file:
    frames = wav_file.readframes(wav_file.getnframes())
    sample_width = wav_file.getsampwidth()

    # Convert PCM data to integers based on sample width
    if sample_width == 1:  # 8-bit audio
        pcm_data = list(frames)  # Already unsigned bytes
    elif sample_width == 2:  # 16-bit audio
        pcm_data = list(int.from_bytes(frames[i:i+2], byteorder='little', signed=True)
                        for i in range(0, len(frames), 2))

# Print first 100 PCM data values
# print(f"First 100 PCM data values from {wav_filename1}: {pcm_data[:10000]}")

# If needed, adjust the range to print more data