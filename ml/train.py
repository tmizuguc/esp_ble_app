'''学習を行う関数（emg_model.ccの値を決定）'''

import argparse
import subprocess
from tensorflow.keras import layers
from tensorflow import keras
import numpy as np
import re
import pandas as pd
import os
import glob

import tensorflow as tf
tf.random.set_seed(1)

# パラメータ
parser = argparse.ArgumentParser()
parser.add_argument('--useTask', nargs='*')
parser.add_argument('--test', action='store_true')
args = parser.parse_args()
useTask = args.useTask

# ファイルパス
label_file_path = f"{os.path.dirname(__file__)}/../monitor/file/label/"
monitor_file_path = f"{os.path.dirname(__file__)}/../monitor/file/monitor/"
model_tflite_file = f"{os.path.dirname(__file__)}/models/model.tflite"
model_cc_file = f"{os.path.dirname(__file__)}/models/model.cc"
esp_model_file = f"{os.path.dirname(__file__)}/../src/emg_model.cc"
test_file = f'{os.path.dirname(__file__)}/test.json'

N_EPOCHS = 5
SEQ_LENGTH = 10
TARGET_DIM = 2


def categorize(arr):
    # カテゴリ変数に変換
    arr = arr//1
    return np.eye(10)[arr.astype(int)]


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


def load_data(task_name, task_num):
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

    # 信号処理後の筋電
    # TODO: tuning.pyと同時に行えるように修正
    try:
        with open(monitor_file_path + f"monitor_{task_name}_{task_num}.txt", "r") as f:
            lines = f.readlines()
    except:
        raise FileNotFoundError

    extensor = []
    flexor = []
    e_tmp = []
    f_tmp = []
    times = []
    read = False
    for line in lines:
        if "time:" in line:
            t = int(re.sub("[^.0-9]", "", line))
        if "sp: start" in line:
            read = True
        if read == True:
            if "e_sp:" in line:
                try:
                    # 書き出しの途中で終了した場合はErrorになる場合がある
                    e_tmp.append(float(re.sub("[^\.0-9]", "", line)))
                except:
                    pass
            if "f_sp:" in line:
                try:
                    # 書き出しの途中で終了した場合はErrorになる場合がある
                    f_tmp.append(float(re.sub("[^\.0-9]", "", line)))
                except:
                    pass
        if "sp: end" in line:
            if len(e_tmp) == 10 and len(f_tmp) == 10:
                extensor.append(e_tmp)
                flexor.append(f_tmp)
                times.append(t)
            e_tmp = []
            f_tmp = []

    # 筋電にラベルを付与
    df_emg = pd.DataFrame(
        {"time": times, "extensor": extensor, "flexor": flexor})
    df = pd.merge_asof(df_emg, df_label, on="time",
                       direction="backward").dropna()

    # 入力可能なように変換
    df_rock = df[df.label == "rock"]
    # rockでflexorがすべて0はおかしいので除外
    # df_rock = df_rock[df_rock.flexor.apply(lambda x: sum(x) != 0)]
    extensor_rock = np.array([_ for _ in df_rock.extensor.values])
    flexor_rock = np.array([_ for _ in df_rock.flexor.values])

    df_paper = df[df.label == "paper"]
    # df_paper = df_paper[df_paper.extensor.apply(
    #     lambda x: sum(x) != 0)]  # paperでextensorがすべて0はおかしいので除外
    extensor_paper = np.array([_ for _ in df_paper.extensor.values])
    flexor_paper = np.array([_ for _ in df_paper.flexor.values])

    extensor = np.concatenate([extensor_rock, extensor_paper])
    flexor = np.concatenate([flexor_rock, flexor_paper])
    label = np.array([0]*len(df_rock) + [1]*len(df_paper))

    # データ増幅
    extensor = np.concatenate(
        [extensor, extensor + 1, extensor + 2, extensor + 1, extensor + 2, extensor, extensor])
    extensor = np.where(extensor > 9, 9, extensor)
    flexor = np.concatenate(
        [flexor, flexor + 1, flexor + 2, flexor, flexor, flexor + 1, flexor + 2])
    flexor = np.where(flexor > 9, 9, flexor)
    label = np.concatenate([label, label, label, label, label, label, label])

    y_data = label
    # one-hotベクトルに
    y_data = np.eye(TARGET_DIM)[y_data.astype(int)]

    X_data = np.array([pd.DataFrame([e, f]).T.values for e, f in zip(
        extensor, flexor)])  # X_data.shape -> (195, 10, 2, 10)
    # X_data.shape -> (195, 10, 2, 10)
    X_data = np.apply_along_axis(categorize, axis=2, arr=X_data)
    X_data = X_data.transpose(0, 1, 3, 2)  # X_data.shape -> (195, 10, 10, 2)

    return X_data, y_data


def get_task_num(task_name):
    # 使用可能なtask_num一覧を取得する
    label_files = glob.glob(label_file_path + f"/*{task_name}*")
    label_num = [int(re.match(
        f".+label_{task_name}_(.+).txt", label_file)[1]) for label_file in label_files]

    monitor_files = glob.glob(monitor_file_path + f"/*{task_name}*")
    monitor_num = [int(re.match(
        f".+monitor_{task_name}_(.+).txt", monitor_file)[1]) for monitor_file in monitor_files]

    return list(set(label_num) & set(monitor_num))


# ========================
# 筋電データとラベルの読み込み
# ========================
X_data_list = []
y_data_list = []
for task_name in useTask:
    # print(f"task: {task_name}")
    task_num_list = get_task_num(task_name)
    for task_num in task_num_list:
        try:
            # X_data, y_data = load_data(task_name=task_name, task_num=task_num)
            # X_data_list.append(X_data)
            # y_data_list.append(y_data)
            print(f"use: {task_name} {task_num}")
        except (FileNotFoundError, AttributeError) as e:
            # print(f"cannot use task {task_name} {task_num}")
            print("Reason -> FileNotFoundError, AttributeError")

print("■■ TEST RESULTS ■■")
print(f"OK!!")
