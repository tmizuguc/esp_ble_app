'''学習のためのデータセットを作成する関数'''

import argparse
import os
import re
import glob
import pandas as pd

# パラメータ
parser = argparse.ArgumentParser()
parser.add_argument('--useTask', nargs='*')
parser.add_argument('--test', action='store_true')
args = parser.parse_args()
useTask = args.useTask

# ファイルパス
label_file_path = f"{os.path.dirname(__file__)}/../monitor/file/label/"
monitor_file_path = f"{os.path.dirname(__file__)}/../monitor/file/monitor/"
dataset_file_path = f"{os.path.dirname(__file__)}/dataset/"

def fix_times(times):
    # label.txtが故障している場合の修復
    # https://github.com/tmizuguc/esp_app/issues/8
    fixed_times = times.copy()
    for t_i, t in enumerate(times):
        if t_i == 0:
            continue
        if t < times[t_i - 1]:
            fixed_times[t_i] = times[t_i - 1] + 3000
    return fixed_times

#===
def load_label_data(task_name, task_num):
    # input
    try:
        with open(label_file_path + f"label_{task_name}_{task_num}.txt", "r") as f:
            times = []
            labels = []

            for line in f.readlines():
                gr = re.search("^([0-9]+): ([a-z]+)\n", line)
                if gr:
                    times.append(int(gr.group(1)))
                    labels.append(gr.group(2))
    except:
        raise FileNotFoundError

    # label.txtが故障している場合の修復
    # https://github.com/tmizuguc/esp_app/issues/8
    times = fix_times(times)

    df_label = pd.DataFrame({"time": times, "label": labels})
    
    return df_label

#===
def load_emg_raw_data(task_name, task_num):
    # 生筋電
    try:
        with open(monitor_file_path + f"monitor_{task_name}_{task_num}.txt", "r") as f:
            lines = f.readlines()
    except:
        raise FileNotFoundError
        
    _time = False
    _extensor_raw = False
    _flexor_raw = False
    times = []
    extensor_raws = []
    flexor_raws = []

    for line in lines:
        if "time:" in line:
            _time = int(re.sub("[^.0-9]", "", line))

        mc = re.match("^([ef]): ([+-]?\\d+(?:\\.\\d+)?)\n", line)
        if mc:
            if mc[1] == "e":
                _extensor_raw = int(float(mc[2]))
            else:
                _flexor_raw = int(float(mc[2]))

        if _time & _extensor_raw & _flexor_raw:
            times.append(_time)
            extensor_raws.append(_extensor_raw)
            flexor_raws.append(_flexor_raw)

            _time = False
            _extensor_raw = False
            _flexor_raw = False

    df_emg_raw = pd.DataFrame({"time": times, "extensor_raw": extensor_raws, "flexor_raw": flexor_raws})
        
    return df_emg_raw

#===
def load_emg_sp_data(task_name, task_num):
    # 信号処理後筋電
    try:
        with open(monitor_file_path + f"monitor_{task_name}_{task_num}.txt", "r") as f:
            lines = f.readlines()
    except:
        raise FileNotFoundError
        
    _time = False
    _extensor_sp = False
    _flexor_sp = False
    times = []
    extensor_sps = []
    flexor_sps = []

    for line in lines:
        if "time:" in line:
            _time = int(re.sub("[^.0-9]", "", line))

        mc = re.match("^(e_sp): ([+-]?\\d+(?:\\.\\d+)?)\n", line)
        if mc:
            if mc[1] == "e_sp":
                _extensor_sp = float(mc[2])
        mc = re.match("^(f_sp): ([+-]?\\d+(?:\\.\\d+)?)\n", line)
        if mc:
            if mc[1] == "f_sp":
                _flexor_sp = float(mc[2])

        if (_time != False) & (_extensor_sp != False) & (_flexor_sp != False):
            times.append(_time)
            extensor_sps.append(_extensor_sp)
            flexor_sps.append(_flexor_sp)

            _time = False
            _extensor_sp = False
            _flexor_sp = False

    df_emg_sp = pd.DataFrame({"time": times, "extensor_raw": extensor_sps, "flexor_raw": flexor_sps})
        
    return df_emg_sp

def load_raw_dataset(task_name, task_num):
    df_label = load_label_data(task_name, task_num)
    df_emg_raw = load_emg_raw_data(task_name, task_num)

    _df = pd.merge_asof(df_emg_raw, df_label, on="time",
                           direction="backward").dropna()
    
    _df["task_name"] = task_name
    _df["task_num"] = task_num
    
    # 整理
    _df = _df[["task_name", "task_num", "label", "extensor_raw", "flexor_raw", "time"]]
    return _df

def load_sp_dataset(task_name, task_num):
    df_label = load_label_data(task_name, task_num)
    df_emg_sp = load_emg_sp_data(task_name, task_num)

    _df = pd.merge_asof(df_emg_sp, df_label, on="time",
                           direction="backward").dropna()
    
    _df["task_name"] = task_name
    _df["task_num"] = task_num
    
    # 整理
    _df = _df[["task_name", "task_num", "label", "extensor_raw", "flexor_raw", "time"]]
    return _df


def get_task_num(task_name):
    # 使用可能なtask_num一覧を取得する
    label_files = glob.glob(label_file_path + f"/*{task_name}*")
    label_num = [int(re.match(
        f".+label_{task_name}_(.+).txt", label_file)[1]) for label_file in label_files]

    monitor_files = glob.glob(monitor_file_path + f"/*{task_name}*")
    monitor_num = [int(re.match(
        f".+monitor_{task_name}_(.+).txt", monitor_file)[1]) for monitor_file in monitor_files]

    return list(set(label_num) & set(monitor_num))


_df_raw_list = []
_df_sp_list = []
for task_name in useTask:
    
    task_num_list = get_task_num(task_name)
    for task_num in task_num_list:
        _df_raw = load_raw_dataset(task_name, task_num)
        _df_raw_list.append(_df_raw)
        _df_sp = load_sp_dataset(task_name, task_num)
        _df_sp_list.append(_df_sp)
        print(f"use: {task_name} {task_num}")
        
df_raw = pd.concat(_df_raw_list)
df_sp = pd.concat(_df_sp_list)

# 保存
df_raw.to_json(dataset_file_path + "raw.json", orient='records')
df_sp.to_json(dataset_file_path + "sp.json", orient='records')