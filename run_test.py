import subprocess

EXECUTABLE = "./circle_buffer_test"
LOG_FILE = "log.txt"

def run(producer_iters, consumer_iters, producer_speed, consumer_speed, buffer_size, timeout=None):
    params = [str(producer_iters), str(consumer_iters), str(producer_speed), str(consumer_speed), str(buffer_size)]

    try:
        result = subprocess.run([EXECUTABLE] + params, capture_output=True, text=True, timeout=timeout)
        output = result.stdout
    except subprocess.TimeoutExpired as e:
        output = "Timed out"

    with open(LOG_FILE, "w") as f:
        f.write(output)

    with open(LOG_FILE, "r") as f:
        return f.read()

def parse_values(output):
    produced = []
    consumed = []
    for line in output.splitlines():
        if line.startswith("Produced "):
            produced.append(int(line.split()[1]))
        elif line.startswith("Consumed "):
            consumed.append(int(line.split()[1]))
    return produced, consumed

def duplicate_produced(output):
    produced, _ = parse_values(output)
    return len(produced) != len(set(produced))

def duplicate_consumed(output):
    _, consumed = parse_values(output)
    return len(consumed) != len(set(consumed))

def find_max_produced(output):
    produced, _ = parse_values(output)
    return max(produced) if produced else None

def find_max_consumed(output):
    _, consumed = parse_values(output)
    return max(consumed) if consumed else None

def find_min_produced(output):
    produced, _ = parse_values(output)
    return min(produced) if produced else None

def find_min_consumed(output):
    _, consumed = parse_values(output)
    return min(consumed) if consumed else None

def run_tests(output, expected_max):
    assert duplicate_produced(output) == False
    assert duplicate_consumed(output) == False
    assert find_max_produced(output) == expected_max
    assert find_max_consumed(output) == expected_max
    assert find_min_produced(output) == 0
    assert find_min_consumed(output) == 0

if __name__ == "__main__":
    # Test basic case
    output = run(10, 10, 1000, 1000, 5, timeout=2)
    run_tests(output, 10)

    # Test no delay
    output = run(10, 10, 0, 0, 5, timeout=2)
    run_tests(output, 10)

    output = run(10, 10, 0, 1000, 5, timeout=2)
    run_tests(output, 10)

    output = run(10, 10, 1000, 0, 5, timeout=2)
    run_tests(output, 10)

    # Test producer/consumer delay mismatch
    output = run(15, 15, 10000, 1000, 5, timeout=2)
    run_tests(output, 15)

    output = run(3, 3, 10000, 1000, 5, timeout=2)
    run_tests(output, 3)

    output = run(15, 15, 1000, 10000, 5, timeout=2)
    run_tests(output, 15)

    output = run(3, 3, 1000, 10000, 5, timeout=2)
    run_tests(output, 3)

    #Test varying buffer size
    output = run(15, 15, 1000, 10000, 6, timeout=2)
    run_tests(output, 15)

    output = run(15, 15, 1000, 10000, 18, timeout=2)
    run_tests(output, 15)

    # Test bad inputs
    output = run(-15, -15, 1000, 1000, 2)
    assert output == "Parameters must be positive\n"

    output = run(15, 15, -1000, 1000, 2)
    assert output == "Parameters must be positive\n"

    output = run(15, 15, -1000, -1000, 2)
    assert output == "Parameters must be positive\n"

    output = run(15, 15, 1000, 1000, 1)
    assert output == "Buffer size must be at least 2\n"

    output = run(15, 15, 1000, 1000, 0)
    assert output == "Buffer size must be at least 2\n"

    # Test producer/consumer iteration mismatch 
    output = run(100, 10, 1000, 1000, 5, timeout=5)
    assert output == "Timed out"

    output = run(100, 10, 1000, 10000, 5, timeout=5)
    assert output == "Timed out"

    output = run(100, 10, 10000, 1000, 5, timeout=5)
    assert output == "Timed out"

    output = run(10, 100, 1000, 1000, 5, timeout=5)
    assert output == "Timed out"

    output = run(10, 100, 1000, 10000, 5, timeout=5)
    assert output == "Timed out"

