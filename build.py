import os
import platform
import subprocess

# Build configurations
BUILD_DIR = "build"
BUILD_TYPE = "Debug"  # Can be Debug or Release


def run_command(command, cwd=None):
    """Run a shell command."""
    print(f"Running command: {' '.join(command)}")  # Debug output
    result = subprocess.run(command, cwd=cwd)
    if result.returncode != 0:
        raise RuntimeError(f"Command failed: {' '.join(command)}")


def is_ninja_available():
    """Check if Ninja is available on the system."""
    try:
        subprocess.run(["ninja", "--version"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
        return True
    except Exception as e:
        print(f"Ninja check failed: {e}")  # Debug output
        return False


def configure_and_build():
    """Configure and build the project."""
    # Detect platform
    system = platform.system()
    generator = None

    if system == "Windows":
        generator = "Visual Studio 17 2022"  # Adjust to match your Visual Studio version
    elif is_ninja_available():
        generator = "Ninja"
    else:
        generator = "Unix Makefiles"  # Default for Linux/Mac

    if not generator:
        raise RuntimeError("No compatible build system found. Please install Ninja or Visual Studio.")

    # Ensure the build directory exists
    os.makedirs(BUILD_DIR, exist_ok=True)

    # Configure the project with CMake
    cmake_command = [
        "cmake",
        "-S", os.getcwd(),  # Source directory
        "-B", os.path.join(os.getcwd(), BUILD_DIR),  # Build directory
        "-G", generator,
        f"-DCMAKE_BUILD_TYPE={BUILD_TYPE}",
    ]
    run_command(cmake_command)

    # Build the project
    build_command = [
        "cmake",
        "--build", os.path.join(os.getcwd(), BUILD_DIR),
        "--config", BUILD_TYPE,
    ]
    run_command(build_command)


def run_tests():
    """Run unit tests."""
    if not os.path.exists(BUILD_DIR):
        raise RuntimeError(f"Build directory '{BUILD_DIR}' does not exist. Cannot run tests.")

    test_command = ["ctest", "--output-on-failure", "-C", BUILD_TYPE]
    run_command(test_command, cwd=BUILD_DIR)


if __name__ == "__main__":
    try:
        configure_and_build()
        run_tests()
    except RuntimeError as e:
        print(f"Error: {e}")
