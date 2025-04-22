#!/bin/bash

# Define your virtual environment directory
VENV_DIR="venv"

# Define the path to your requirements file
REQUIREMENTS_FILE="requirements.txt"

# Check if virtual environment exists
if [ ! -d "$VENV_DIR" ]; then
  echo "Virtual environment not found. Creating one..."
  python3 -m venv $VENV_DIR
fi

# Activate the virtual environment
echo "Activating virtual environment..."
source $VENV_DIR/bin/activate

# Upgrade pip just in case
echo "Upgrading pip..."
pip install --upgrade pip

# Install required modules from requirements.txt
if [ -f "$REQUIREMENTS_FILE" ]; then
  echo "Installing dependencies from $REQUIREMENTS_FILE..."
  pip install -r $REQUIREMENTS_FILE
else
  echo "requirements.txt not found. Please create one with required packages."
fi

echo "Setup complete."
