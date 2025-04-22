import socket
import time

ESP32_IP = "192.168.48.32"
PORT = 1234
file_location = "/home/mukeshbala/Desktop/comp_output" 

def send_data(encoded_line):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(10)
        s.connect((ESP32_IP, PORT))
        s.sendall((encoded_line + "\n").encode())

        ack = s.recv(1024).decode().strip()
        if ack == "ACK":
            print("ACK received. Data sent successfully.")
        else:
            print("No ACK received.")

if __name__ == "__main__":
    filepath = file_location + "/encoded_data.txt"
    while True:
        with open(filepath, "r") as f:
            lines = f.readlines()
            if lines:
                latest_line = lines[-1].strip()
                print(f"Sending: {latest_line}")
                send_data(latest_line)
        time.sleep(10)
