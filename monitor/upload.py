import subprocess
import argparse

subprocess.call(
    f"pio run --target upload", shell=True)