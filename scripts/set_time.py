import serial
import time

def main():
    with serial.Serial("/dev/cu.SLAB_USBtoUART", 115200) as s:
        current_time = int(time.time())
        s.write(f"{current_time}\n".encode())

        while True:
            time.sleep(100)

if __name__ == '__main__':
    main()