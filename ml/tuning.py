'''チューニングのみ行う関数'''

import numpy as np
import re
import os

# チューニング用のmonitorとlabelは0番目固定
monitor_file = f"monitor/file/monitor/monitor_0.txt"
label_file = f"monitor/file/label/label_0.txt"
param_file = "./src/constants_param.h"

# =============================
# constants_param.hの学習
# =============================
# 信号処理後データ
with open(monitor_file, "r") as f:
    extensor = np.array([float(re.sub("[^\\.0-9]", "", line))
                         for line in f.readlines() if re.match("^e: ", line) and len(line) >= 4])
with open(monitor_file, "r") as f:
    flexor = np.array([float(re.sub("[^\\.0-9]", "", line))
                       for line in f.readlines() if re.match("^f: ", line) and len(line) >= 4])

if len(extensor) > 30000:
    extensor = extensor[15000:]
    flexor = flexor[15000:]
else:
    extensor = extensor[-12000:]
    flexor = flexor[-12000:]

extensor_ave = extensor.mean()
flexor_ave = flexor.mean()

filter_width = 1  # 移動平均の個数
fileter = np.ones(filter_width)/filter_width

extensor_rolling = np.convolve(
    abs(extensor - extensor_ave), fileter, mode='same')
flexor_rolling = np.convolve(abs(flexor - flexor_ave), fileter, mode='same')

extensor_max = min(4000, np.sort(extensor_rolling)[
                   int(len(extensor_rolling)*0.95)]*1.2)  # 移動平均のmax
flexor_max = min(4000, np.sort(flexor_rolling)[
                 int(len(flexor_rolling)*0.95)]*1.2)  # 移動平均のmax
extensor_min = max(0, np.sort(extensor_rolling)[
                   int(len(extensor_rolling)*0.2)]*1.2)  # 移動平均のmin
flexor_min = max(0, np.sort(flexor_rolling)[
                 int(len(flexor_rolling)*0.2)]*1.2)  # 移動平均のmin

# 更新
with open(param_file, "w") as f:
    f.write(f"const float extensor_max = {int(extensor_max)};\n")
    f.write(f"const float flexor_max = {int(flexor_max)};\n")
    # f.write(f"const float extensor_min = {int(extensor_min)};\n")
    # f.write(f"const float flexor_min = {int(flexor_min)};\n")
    f.write(f"const float extensor_ave = {int(extensor_ave)};\n")
    f.write(f"const float flexor_ave = {int(flexor_ave)};\n")
    print(f"const float extensor_ave = {int(extensor_ave)};\n")
    print(f"const float flexor_ave = {int(flexor_ave)};\n")
