import subprocess
import time
import statistics

def run_program(program, args):
    """
    Simulates C's fork, execvp, and wait using Python's subprocess module.

    Args:
        pcMatrix (str): The path to the executable program.
        args (list): A list of strings representing the arguments to pass to the program.

    Returns:
        tuple: A tuple containing the return code, standard output, and standard error.
    """
    try:
        # Simulate fork and execvp using subprocess.run
        result = subprocess.run(
            [program] + args,  # Combine program path and arguments
            capture_output=True,  # Capture stdout and stderr
            text=True,  # Decode stdout and stderr as text
            check=True  # Raise an exception for non-zero return codes
        )

        # Simulate wait by waiting for subprocess.run to complete

        return result.returncode, result.stdout, result.stderr

    except subprocess.CalledProcessError as e:
        # Handle non-zero return codes
        return e.returncode, e.stdout, e.stderr

    except FileNotFoundError:
        # Handle case where the executable is not found
        return -1, "", "Executable not found"

if __name__ == '__main__':
    
    pcMatrix = "./pcMatrix"
        
    # Run with default arguments
    default_times = []
    for i in range(100):
        print(f"DEFAULT TEST #{i + 1}")
        start_time = time.time()
        return_code, stdout, stderr = run_program(pcMatrix, [])
        end_time = time.time()
        execution_time = end_time - start_time
        default_times.append(execution_time)
        
        print(f"Execution time: {execution_time:.4f} seconds")
        if return_code == 0: print(f"Stdout:\n{stdout}")
        elif return_code == -1: 
            print(f"Stderr:\n{stderr}")
            break
        
    # Run with custom arguments
    custom_times = []
    for i in range(200):
        arguments = [str(i), str(200 - (i * 2) % 200), "1000", "0"]
        if int(arguments[1]) <= 0: arguments[1] = "1"
        print(f"NUMWORK = {arguments[0]} BUFFER SIZE = {arguments[1]} LOOPS = {arguments[2]} MODE = {arguments[3]} TEST #{i + 1}")
        
        start_time = time.time()
        return_code, stdout, stderr = run_program(pcMatrix, arguments)
        end_time = time.time()
        execution_time = end_time - start_time
        custom_times.append(execution_time)
        
        print(f"Execution time: {execution_time:.4f} seconds")
        if return_code == 0: 
            print(f"Stdout:\n{stdout}")
        else:
            print(f"Stderr:\n{stderr}")
            break
    
    # Print default test statistics
    if default_times:
        print("\nDEFAULT TEST STATISTICS:")
        print(f"Average time: {statistics.mean(default_times):.4f} seconds")
        print(f"Min time: {min(default_times):.4f} seconds")
        print(f"Max time: {max(default_times):.4f} seconds")
        print(f"Standard deviation: {statistics.stdev(default_times):.4f} seconds") if len(default_times) > 1 else None
    
    # Print custom test statistics
    if custom_times:
        print("\nCUSTOM TEST STATISTICS:")
        print(f"Average time: {statistics.mean(custom_times):.4f} seconds")
        print(f"Min time: {min(custom_times):.4f} seconds")
        print(f"Max time: {max(custom_times):.4f} seconds")
        print(f"Standard deviation: {statistics.stdev(custom_times):.4f} seconds") if len(custom_times) > 1 else None
