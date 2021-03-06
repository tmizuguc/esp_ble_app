import os
import argparse

# do_monitor, do_task, view_monitor, view_stateで共通
parser = argparse.ArgumentParser()
parser.add_argument('--taskNumber', type=int, default=0)
parser.add_argument('--taskType', type=str)
args = parser.parse_args()

monitor_file = f"monitor/file/monitor/monitor_{args.taskType}_{args.taskNumber}.txt"
label_file = f"monitor/file/label/label_{args.taskType}_{args.taskNumber}.txt"

try:
    os.remove(monitor_file) 
    print(f"deleted monitor: {task_type} {task_num}")
except:
    pass

try:
    os.remove(label_file) 
    print(f"deleted label: {task_type} {task_num}") 
except:
    pass