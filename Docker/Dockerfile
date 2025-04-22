# Use a general lightweight base image with Python pre-installed
FROM python:3.9-slim

# Set the working directory
WORKDIR /app

RUN apt-get update && apt-get install ffmpeg libsm6 libxext6  -y

# Copy the requirements file first to leverage Docker cache
COPY requirements.txt /app/requirements.txt

# Install Python dependencies
RUN pip install --upgrade pip && pip install --no-cache-dir -r requirements.txt

# Copy the entire project to the working directory
COPY . /app

# Expose port for optional API usage
EXPOSE 8000

# Default command to execute the script in the `yolov9` folder
CMD ["python", "yolov9/detect.py"]
