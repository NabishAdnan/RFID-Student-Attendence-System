import serial
import time
import pandas as pd
from datetime import datetime
import os

# Configure the serial port to match the Arduino's port
ser = serial.Serial('COM3', 9600, timeout=1)  # Update 'COM3' to match your Arduino port
time.sleep(2)  # Give some time for the connection to establish

# Create a DataFrame to store the logs
columns = ['Date', 'Time', 'UID', 'Name', 'Action']
log_data = pd.DataFrame(columns=columns)

# Define the new base directory for saving the Excel files
base_directory = r'D:\Student Attendence System\excel'

# Ensure the directory exists
if not os.path.exists(base_directory):
    os.makedirs(base_directory)

def get_filename():
    """Generate a filename based on the current date."""
    today = datetime.now().strftime('%Y-%m-%d')
    return os.path.join(base_directory, f'Access_Log_{today}.xlsx')

# Dictionary to track the last action for each UID
last_action = {}

try:
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            if line:
                # Get the current date and time from the laptop
                current_time = datetime.now()
                date = current_time.strftime('%Y-%m-%d')
                time_str = current_time.strftime('%H:%M:%S')

                # Split the received data
                try:
                    uid, name, _ = line.split(', ')
                except ValueError:
                    print("Received data is not in the expected format.")
                    continue

                # Determine the action based on the last action
                if uid in last_action:
                    action = 'Exit' if last_action[uid] == 'Entry' else 'Entry'
                else:
                    action = 'Entry'

                # Update the last action for this UID
                last_action[uid] = action

                # Add data to the DataFrame
                new_log = {'Date': date, 'Time': time_str, 'UID': uid, 'Name': name, 'Action': action}
                log_data = pd.concat([log_data, pd.DataFrame([new_log])], ignore_index=True)

                # Save data to an Excel file with date-based filename
                filename = get_filename()
                log_data.to_excel(filename, index=False)

                print(f"Logged: {new_log}")

except KeyboardInterrupt:
    print("Logging stopped.")
finally:
    ser.close()
