import subprocess
import os
import json
import time
from datetime import datetime

# Define paths
venv_python = "/home/mukeshbala/Desktop/scripts/venv/bin/python3"
input_dir = "/home/mukeshbala/Desktop/input"
output_dir = "/home/mukeshbala/Desktop/output"
encoded_output = "/home/mukeshbala/Desktop/comp_output/encoded_data.txt"

def run_main_logic():
    docker_command = [
        "sudo", "docker", "run", "--rm",
        "-v", f"{input_dir}:/input",
        "-v", f"{output_dir}:/output",
        "samendhra007/yolov11:latest",
        "python", "yolov9/detect.py",
        "--source", "/input",
        "--project", "/output"
    ]

    try:
        total_start = time.time()

        # 1. Run Docker
        print("Running Docker container...")
        docker_start = time.time()
        subprocess.run(docker_command, check=True)
        print(f"Docker completed in {time.time() - docker_start:.2f} seconds.")

        # 2. Find latest exp folder
        find_start = time.time()
        exp_folders = sorted(
            [f for f in os.listdir(output_dir) if f.startswith("exp")],
            key=lambda x: os.path.getmtime(os.path.join(output_dir, x)),
            reverse=True
        )
        if not exp_folders:
            print("No output folders found.")
            return

        latest_exp = os.path.join(output_dir, exp_folders[0])
        json_path = os.path.join(latest_exp, "face_counts.json")
        print(f"Scanning folders took {time.time() - find_start:.2f} seconds.")

        # 3. Encode JSON into a single line and append to file
        encode_start = time.time()
        with open(json_path, "r") as infile:
            # Copy JSON to comp_output with timestamped filename
            timestamp_filename = "slm_" + datetime.now().strftime('%Y%m%d%H%M%S') + ".json"
            json_backup_path = os.path.join("/home/mukeshbala/Desktop/comp_output", timestamp_filename)
            subprocess.run(["cp", json_path, json_backup_path], check=True)

            json_data = json.load(infile)
            encoded_entries = []

            for entry in json_data:
                pi_id = entry["pi_id"]
                num_faces = str(entry["num_faces"])
                timestamp_raw = entry["timestamp"]

                dt = datetime.fromtimestamp(timestamp_raw)
                timestamp_compact = dt.strftime('%Y%m%d%H%M%S') + f"{int(dt.microsecond / 1000):03}"

                encoded = f"{pi_id};{num_faces};{timestamp_compact}#"
                encoded_entries.append(encoded)

            full_line = ''.join(encoded_entries)
            with open(encoded_output, "a") as outfile:
                outfile.write(full_line + "\n")

            print("Encoded line:")
            print(full_line)

        print(f"Encoding took {time.time() - encode_start:.2f} seconds.")
        print(f"Appended encoded data to {encoded_output}")

        # 4. Delete all exp folders
        del_start = time.time()
        for folder in exp_folders:
            full_path = os.path.join(output_dir, folder)
            subprocess.run(["sudo", "rm", "-rf", full_path], check=True)
        print(f"Deleted folders in {time.time() - del_start:.2f} seconds.")

        print(f"\n? Total time taken: {time.time() - total_start:.2f} seconds.")

    except Exception as e:
        print(f"? Error: {e}")

if __name__ == "__main__":
    run_main_logic()
