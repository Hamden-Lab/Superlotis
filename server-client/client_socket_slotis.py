import socket
import subprocess
import time
import os

def open_socket(server_ip, server_port):
    """Create and return a socket object connected to the specified server."""
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((server_ip, server_port))
    print("Connected to {}:{}".format(server_ip, server_port))
    return client_socket

def send_file(client_socket, file_to_send):
    """Send a file over the given socket connection."""
    with open(file_to_send, 'rb') as f:
        print("Sending the file...")
        total_sent = 0
        while True:
            data = f.read(1024)
            if not data:
                break
            client_socket.sendall(data)
            total_sent += len(data)

    print("File {} sent successfully. Total bytes sent: {}".format(file_to_send, total_sent))

def expose(client_socket):
    print("Running exposure...")
    server_process = subprocess.Popen(["gnome-terminal", "--", "bash", "-c", "./bin/camserver_cit; exec bash"])
    time.sleep(2)  # Ensure the server is ready
    expose_process = subprocess.Popen(["gnome-terminal", "--", "bash", "-c", "./bin/cam_cit expose; exec bash"])
    expose_process.wait()
    server_process.terminate()
    print("Exposure complete, .raw file generated")

    # Send completion status to the server
    status_message = "Exposure completed successfully"
    client_socket.sendall(status_message.encode())

def listen_for_commands(client_socket):
    while True:
        command = client_socket.recv(1024).decode()  # Receive command
        if command:
            print("Command received: '{}'".format(command))
            if command == 'expose':
                expose(client_socket)  # Pass the socket to expose function
            else:
                print("Unknown command: {}".format(command))

def close_socket(client_socket):
    """Close the socket connection."""
    client_socket.close()
    print("Socket closed.")

if __name__ == "__main__":
    server_ip = "192.168.0.125"  # Replace with server IP address
    server_port = 12345

    # Open socket and listen for commands
    client_socket = open_socket(server_ip, server_port)
    try:
        listen_for_commands(client_socket)
    finally:
        raw_exists = os.path.isfile('exposure_file.raw')
        if raw_exists:
            print('.raw file exists')
            # send_file(client_socket, 'exposure_file.raw')

        close_socket(client_socket)
