import struct
import os

# These constants MUST match our C code exactly
SYNC_BYTE = 0xAA
IMU_SENSOR_ID = 0x01
IMU_PAYLOAD_LEN = 12  # 6 int16_t values (2 bytes each)

def parse_blackbox_log(filepath):
    if not os.path.exists(filepath):
        print(f"Error: {filepath} not found. Run the integration test first!")
        return

    with open(filepath, "rb") as f:
        data = f.read()

    print(f"--- Decoded Blackbox Telemetry ({len(data)} bytes) ---")
    print(f"{'TS (ms)':<8} | {'AccX':<6} | {'AccY':<6} | {'AccZ':<6} | {'GyrX':<6} | {'GyrY':<6} | {'GyrZ':<6} | {'Status'}")
    print("-" * 75)

    i = 0
    valid_packets = 0
    corrupt_packets = 0

    while i < len(data):
        # 1. Look for the Sync Byte
        if data[i] == SYNC_BYTE:
            # We need at least: Sync(1) + Time(4) + ID(1) + Len(1) + Payload(12) + Checksum(1) = 20 bytes
            if i + 20 > len(data):
                break 

            # 2. Extract Header
            # Format string "<BIBB": 
            # < = Little Endian
            # B = unsigned char (1 byte)
            # I = unsigned int (4 bytes)
            try:
                header = struct.unpack("<BIBB", data[i:i+7])
                sync, timestamp, s_id, length = header
                
                # 3. Extract Payload (6 signed shorts 'h')
                payload_start = i + 7
                payload_end = payload_start + length
                payload_raw = data[payload_start:payload_end]
                
                imu_data = struct.unpack("<hhhhhh", payload_raw)
                
                # 4. Verify Checksum (XOR all bytes including header and payload)
                received_checksum = data[payload_end]
                calculated_checksum = 0
                for b in data[i:payload_end]:
                    calculated_checksum ^= b
                
                status = "OK"
                if calculated_checksum == received_checksum:
                    valid_packets += 1
                    # Only print every 10th packet to keep the terminal clean
                    if valid_packets % 10 == 0:
                        print(f"{timestamp:<8} | {imu_data[0]:<6} | {imu_data[1]:<6} | {imu_data[2]:<6} | "
                              f"{imu_data[3]:<6} | {imu_data[4]:<6} | {imu_data[5]:<6} | {status}")
                else:
                    status = "BAD_CRC"
                    corrupt_packets += 1
                    print(f"{timestamp:<8} | ERROR: Checksum Mismatch | {status}")

                # Move index to the start of the next potential packet
                i = payload_end + 1
                
            except struct.error:
                i += 1 # Something went wrong, skip one byte and keep looking
        else:
            i += 1

    print("-" * 75)
    print(f"Summary: {valid_packets} valid packets found, {corrupt_packets} corrupt.")

if __name__ == "__main__":
    parse_blackbox_log("blackbox_log.bin")