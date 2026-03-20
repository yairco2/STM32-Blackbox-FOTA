import struct
import matplotlib.pyplot as plt

def plot_telemetry(filename):
    timestamps, acc_x, acc_y, acc_z = [], [], [], []
    
    with open(filename, "rb") as f:
        data = f.read()

    i = 0
    while i < len(data) - 20:
        if data[i] == 0xAA:
            header = struct.unpack("<BIBB", data[i:i+7])
            ts, length = header[1], header[3]
            payload_end = i + 7 + length
            
            # Checksum check
            calc_crc = 0
            for b in data[i:payload_end]: calc_crc ^= b
            
            if calc_crc == data[payload_end]:
                imu = struct.unpack("<hhhhhh", data[i+7:payload_end])
                timestamps.append(ts)
                acc_x.append(imu[0])
                acc_y.append(imu[1])
                acc_z.append(imu[2])
                i = payload_end + 1
            else:
                i += 1
        else:
            i += 1

    # Plotting
    plt.figure(figsize=(10, 6))
    plt.plot(timestamps, acc_x, label='Accel X', color='red')
    plt.plot(timestamps, acc_y, label='Accel Y', color='green')
    plt.plot(timestamps, acc_z, label='Accel Z', color='blue')
    
    plt.title("Blackbox IMU Telemetry (Simulated Sawtooth)")
    plt.xlabel("Time (ms)")
    plt.ylabel("Raw Sensor Value")
    plt.legend()
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    plot_telemetry("blackbox_log.bin")