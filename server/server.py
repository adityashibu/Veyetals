import subprocess
import os

def parse_output_line(line, data):
    if line.startswith("Day:"):
        data['day'] = line.split(": ")[1]
    elif line.startswith("Date and Time:"):
        data['datetime'] = line.split(": ")[1]
    elif line.startswith("Memory Usage:"):
        data['memory_usage'] = line.split(": ")[1].replace("%", "")
    elif line.startswith("Total Physical Memory:"):
        data['total_memory'] = line.split(": ")[1].replace(" MB", "")
    elif line.startswith("Available Physical Memory:"):
        data['available_memory'] = line.split(": ")[1].replace(" MB", "")
    elif line.startswith("CPU Usage:"):
        data['cpu_usage'] = line.split(": ")[1].replace("%", "")
    elif line.startswith("GPU Name:"):
        data['gpu_name'] = line.split(": ")[1]
    elif line.startswith("GPU Usage:"):
        data['gpu_usage'] = line.split(": ")[1].replace("%", "")
    elif line.startswith("GPU Memory Usage:"):
        memory_usage = line.split(": ")[1]
        used, total = memory_usage.split(" / ")
        data['gpu_memory_used'] = used.replace(" MB", "")
        data['gpu_memory_total'] = total.replace(" MB", "")
    elif line.startswith("GPU Temperature:"):
        data['gpu_temperature'] = line.split(": ")[1].replace(" C", "")
    
    return data

def run_executable():
    data = {}
    exe_path = os.path.join(os.path.dirname(__file__), '..', 'data', 'main.exe')

    try:
        with subprocess.Popen(
            [exe_path], 
            stdout=subprocess.PIPE, 
            stderr=subprocess.PIPE, 
            bufsize=1, 
            universal_newlines=True, 
            text=True, 
            shell=True
        ) as proc:
            while True:
                output_line = proc.stdout.readline()
                
                if output_line:
                    data = parse_output_line(output_line.strip(), data)
                    print("Current Data: ", data)  # Real-time output handling

                if proc.poll() is not None:
                    break

    except Exception as e:
        print("Exception occurred while running the executable: ", e)

if __name__ == "__main__":
    run_executable()
