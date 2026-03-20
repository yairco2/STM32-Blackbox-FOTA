import struct
import time
import zlib
import os
import argparse
import serial # pip install pyserial

# Constants matching fota_handler.h
FOTA_MAGIC_WORD = 0xDEADBEEF
CHUNK_SIZE = 256

def calculate_crc32(data: bytes) -> int:
    """Calculates the standard IEEE 802.3 CRC32 (Matches C implementation)."""
    return zlib.crc32(data) & 0xFFFFFFFF

def push_firmware(filepath: str, com_port: str = None, version: int = 100):
    if not os.path.exists(filepath):
        print(f"Error: File '{filepath}' not found.")
        return

    # 1. Read the entire firmware file
    with open(filepath, "rb") as f:
        firmware_data = f.read()

    file_size = len(firmware_data)
    expected_crc = calculate_crc32(firmware_data)

    print(f"--- FOTA Update Initiated ---")
    print(f"File: {filepath}")
    print(f"Size: {file_size} bytes")
    print(f"Version: {version}")
    print(f"CRC32: 0x{expected_crc:08X}")
    print("-" * 30)

    # 2. Pack the Header (<IIII = Little Endian, 4 unsigned 32-bit ints)
    header = struct.unpack("<IIII", struct.pack("<IIII", FOTA_MAGIC_WORD, version, file_size, expected_crc))
    header_bytes = struct.pack("<IIII", FOTA_MAGIC_WORD, version, file_size, expected_crc)

    # 3. Setup Serial or Simulation
    ser = None
    if com_port:
        try:
            ser = serial.Serial(com_port, 115200, timeout=2)
            print(f"Connected to {com_port}. Sending Header...")
            ser.write(header_bytes)
            time.sleep(1) # Wait for STM32 to erase flash
        except Exception as e:
            print(f"Failed to open port {com_port}: {e}")
            return
    else:
        print("[SIMULATION MODE] - Header generated. Ready to transmit.")

    # 4. Stream Chunks
    chunks_sent = 0
    for i in range(0, file_size, CHUNK_SIZE):
        chunk = firmware_data[i : i + CHUNK_SIZE]
        
        if ser:
            ser.write(chunk)
            # In a real robust system, we would wait for an 'ACK' byte from STM32 here
            time.sleep(0.05) 
        
        chunks_sent += 1
        progress = min(100, (i + CHUNK_SIZE) / file_size * 100)
        print(f"Sent Chunk {chunks_sent:03} | Size: {len(chunk):03} bytes | Progress: {progress:05.1f}%")

    if ser:
        ser.close()
        print("\nFOTA Transmission Complete. Awaiting device reboot.")
    else:
        print("\n[SIMULATION MODE] - Transmission Complete.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="STM32 Blackbox FOTA Pusher")
    parser.add_argument("file", help="Path to the .bin firmware file")
    parser.add_argument("-p", "--port", help="COM port (e.g., COM3 or /dev/ttyUSB0)", default=None)
    parser.add_argument("-v", "--version", help="Firmware version number", type=int, default=101)
    
    args = parser.parse_args()
    push_firmware(args.file, args.port, args.version)