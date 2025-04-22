#!/bin/bash

# Paths
VENV_PATH="/home/mukeshbala/Desktop/scripts/venv"
SCRIPT_DIR="/home/mukeshbala/Desktop/scripts"

# Start the IP webcam script
x-terminal-emulator -e bash -c "cd $SCRIPT_DIR && source $VENV_PATH/bin/activate && python3 ipwebcam_headless.py; exec bash" &

# Wait 5 seconds
sleep 5

# Run the detector in a loop every 40 seconds (with a 5-second sleep between runs)
x-terminal-emulator -e bash -c "
cd $SCRIPT_DIR && source $VENV_PATH/bin/activate;
while true; do
    python3 run_detector_clean_2.py;
done;
exec bash" &

# Start the flask server
x-terminal-emulator -e bash -c "cd $SCRIPT_DIR && source $VENV_PATH/bin/activate && python3 transmitter_user.py; exec bash"
