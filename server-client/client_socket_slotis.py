import socket
import subprocess
import time
import os
import csv
import argparse
import re
import signal

def open_socket(server_ip, server_port):
    """Create and return a socket object connected to the specified server."""
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((server_ip, server_port))
    print("Connected to {}:{}".format(server_ip, server_port))
    return client_socket


def start_server():
    subprocess.call(["gnome-terminal", "--", "bash", "-c", "./bin/camserver_cit; exec bash"])
    # return server_process

def terminate_existing_server():
    result = subprocess.Popen(
        ["pgrep", "-f", "./bin/camserver_cit"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    stdout, stderr = result.communicate()  # Get output and error
    if stdout:
        pid = stdout.strip().split('\n')[0]  # Split output by new lines
        os.kill(int(pid), signal.SIGTERM)  # Terminate each process
        print("Terminated existing server process with PID: {}".format(pid))
        time.sleep(1)  # Wait a moment for the processes to terminate



# def set_rois(client_socket):
# def get_rois(client_socket):

def setexp(client_socket, exposure_time):
    print('Setting exposure time to: {}'.format(exposure_time))
    subprocess.call(["gnome-terminal", "--", "bash", "-c", "./bin/cam_cit exptime {}; sleep 5; pkill -f 'camserver_cit'; exit".format(exposure_time)])
    status_message = "Exposure time is: {}".format(exposure_time)
    client_socket.sendall(status_message.encode())


def status(client_socket):
    print("Getting status information...")
    server_process = subprocess.Popen(["gnome-terminal", "--", "bash", "-c", "./bin/camserver_cit > output.txt 2>&1; exec bash"])
    time.sleep(2)
    status_process = subprocess.Popen(["gnome-terminal", "--", "bash", "-c", 
                    "truncate -s 0 status.txt; "  # Clear the contents of status.txt
                    "temp_output=$(./bin/cam_cit temp); "
                    "analog_gain_output=$(./bin/cam_cit analog_gain); "
                    "exptime_output=$(./bin/cam_cit exptime); "
                    "shutter_mode_output=$(./bin/cam_cit shutter_mode); "  
                    "echo \"temp = $temp_output\" >> status.txt; "
                    "echo \"analog_gain = $analog_gain_output\" >> status.txt; "
                    "echo \"exptime = $exptime_output\" >> status.txt; "
                    "echo \"shutter_mode = $shutter_mode_output\" >> status.txt; "  # Append shutter mode output
                    "exec bash"])
    time.sleep(2)
    status_process.wait()
    server_process.terminate()
    print("Status data acquired")
    status_message = "Data acquired successfully"
    client_socket.sendall(status_message.encode())

def generate_csv(client_socket, csv_filename):
    # Read the data from status.txt
    with open("status.txt", "r") as txt_file:
        lines = txt_file.readlines()
    # Prepare the data for CSV by extracting the values
    data = {}
    for line in lines:
        key, value = line.strip().split(" = ")
        data[key] = value
    # Write the data to a CSV file (for Python 2, use 'wb' mode)
    with open(csv_filename, "wb") as csv_file:  # Open in binary mode 'wb'
        writer = csv.writer(csv_file)
        # Write the header
        writer.writerow(data.keys())
        # Write the row with the values
        writer.writerow(data.values())
        return csv_filename

def send_status(client_socket, csv_filename):
    status(client_socket)
    csv_filename = generate_csv(client_socket, csv_filename)
    send_file(client_socket)


def expose(client_socket):
    print("Taking exposure...")
    subprocess.call(["gnome-terminal", "--", "bash", "-c", "./bin/cam_cit expose; sleep 5; pkill -f 'camserver_cit'; exit"])
    print("Science image acquired, .raw file generated")
    status_message = "Exposure completed successfully"
    client_socket.sendall(status_message.encode())
    file_name = 'exposure_file.raw'
    return file_name

def bias(client_socket):
    print("Taking bias...")
    subprocess.call(["gnome-terminal", "--", "bash", "-c", "./bin/cam_cit bias; sleep 5; pkill -f 'camserver_cit'; exit"])
    print("Bias image acquired, .raw file generated")
    status_message = "Bias completed successfully"
    client_socket.sendall(status_message.encode())
    file_name = 'bias_file.raw'
    return file_name

def dark(client_socket):
    print("Taking dark...")
    subprocess.call(["gnome-terminal", "--", "bash", "-c", "./bin/cam_cit dark; sleep 5; pkill -f 'camserver_cit'; exit"])
    print("Dark image acquired, .raw file generated")
    status_message = "Dark completed successfully"
    client_socket.sendall(status_message.encode())
    file_name = 'dark_file.raw'
    return file_name

def send_file(client_socket, file_name):
    """Send a file over the given socket connection."""
    with open('{}.raw'.format(file_name), 'rb') as f:
        print("Sending the file...")
        total_sent = 0
        while True:
            data = f.read(1024)
            if not data:
                break
            client_socket.sendall(data)
            total_sent += len(data)

    print("File sent: {}".format(file_name))
    return file_name

def close_socket(client_socket):
    """Close the socket connection."""
    client_socket.close()
    print("Socket closed.")

def run_command(client_socket, command):
    print("Taking {}...".format(command))
    subprocess.call(["gnome-terminal", "--", "bash", "-c", "./bin/cam_cit {}; sleep 5; pkill -f 'camserver_cit'; exit".format(command)])
    print("{} image acquired, file generated".format(command.capitalize()))
    status_message = "{} completed successfully".format(command.capitalize())  # Added missing closing parenthesis
    client_socket.sendall(status_message.encode())

def listen_for_commands(client_socket, command):
    file_name = None
    print("Listening for command")

    if len(command.split()) == 1:
        if command == 'expose':
            print(command)
            time.sleep(2)
            terminate_existing_server()
            start_server()
            time.sleep(2)
            run_command(client_socket, 'expose')
            time.sleep(2)
            file_name = 'exposure_file.raw'
        elif command == 'bias':
            time.sleep(2)
            terminate_existing_server()
            start_server()
            time.sleep(2)
            run_command(client_socket, 'bias')
            time.sleep(2)
            file_name = 'bias_file.raw'
        elif command == 'dark':
            time.sleep(2)
            terminate_existing_server()
            start_server()
            time.sleep(2)
            run_command(client_socket, 'dark')
            time.sleep(2)
            file_name = 'dark_file.raw'
        elif command == 'status':
            time.sleep(2)
            terminate_existing_server()
            start_server()
            time.sleep(2)
            run_command(client_socket, 'status')
            time.sleep(2)
            file_name = 'status.csv'
        elif command == 'close_socket':
            close_socket(client_socket)
        elif command == 'gfile':
            if file_name:
                file_name = send_file(client_socket, file_name)


    elif len(command.split()) > 1:
        if command[0] == 'setexp':
            value = command[1]
            time.sleep(2)
            terminate_existing_server()  # Ensure this function is defined elsewhere
            start_server()  # Ensure this function is defined elsewhere
            time.sleep(2)
            run_command(client_socket, 'setexp {}'.format(command[1]))  # Pass the exposure time
            file_name = None


if __name__ == "__main__":
    server_ip = "192.168.0.125"  # Replace with server IP address
    server_port = 12345
    client_socket = open_socket(server_ip, server_port)
    try:
        client_socket.settimeout(15)  # Adjust timeout as needed
        command = client_socket.recv(1024)  # Receive command

        print("Command received: '{}'".format(command))
        file_name = None
        file_name = listen_for_commands(client_socket, command)
