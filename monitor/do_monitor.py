import subprocess
import argparse

# do_monitor, do_task, view_monitor, view_stateで共通
parser = argparse.ArgumentParser()
parser.add_argument('--taskNumber', type=int, default=0)
args = parser.parse_args()

monitor_file = f"monitor/file/monitor/monitor_{args.taskNumber}.txt"
label_file = f"monitor/file/label/label_{args.taskNumber}.txt"
# ここまで


subprocess.call(
    f"pio device monitor > {monitor_file}", shell=True)
