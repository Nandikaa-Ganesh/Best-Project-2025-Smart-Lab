#!/bin/bash

# Paths
VENV_PATH="/home/mukeshbala/Desktop/scripts/venv"
SCRIPT_DIR="/home/mukeshbala/Desktop/scripts"

# Start the IP webcam script
x-terminal-emulator -e bash -c "cd $SCRIPT_DIR && source $VENV_PATH/bin/activate && python3 ipwebcam.py" &

# Wait 5 seconds
sleep 5

# Run the detector in one terminal
x-terminal-emulator -e bash -c "cd $SCRIPT_DIR && source $VENV_PATH/bin/activate && python3 run_detector_clean_2.py"

# Run the transmitter in another terminal
x-terminal-emulator -e bash -c "cd $SCRIPT_DIR && source $VENV_PATH/bin/activate && python3 transmitter_lora.py"
