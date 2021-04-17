import os
import re
import argparse
import time
import pygame.mixer
import json

# do_monitor, do_task, view_monitor, view_stateで共通
parser = argparse.ArgumentParser()
parser.add_argument('--taskNumber', type=int, default=0)
args = parser.parse_args()

monitor_file = f"monitor/file/monitor/monitor_{args.taskNumber}.txt"
label_file = f"monitor/file/label/label_{args.taskNumber}.txt"
# ここまで

# パラメータ
with open("param.json") as f:
    param = json.load(f)


def sound(source):
    '''音を鳴らす'''
    pygame.mixer.music.load(f"sound/{source}")
    pygame.mixer.music.play(1)


def instruct(instruction, keep_time):
    '''1つの指示（グー/パー/レスト）を教師ラベルとしてlabel.txtに追記する'''

    # monitor読み込み
    # 10ms以下で完了できる

    # with open(monitor_file, "r") as f:
    #     for line in f.readlines()[::-1]:
    #         if "time: " in line:
    #             t = int(re.sub("[^0-9]", "", line))
    #             break

    # 教師として書き込み
    with open(label_file, "a") as f:
        f.write(f"0: {instruction}\n")

    # コマンドラインへ表示
    print(f"指示-> {instruction}をしてください。\n")

    # 音を鳴らす
    is_first = True
    for _ in range(keep_time):
        if is_first:
            sound(source="pi.mp3")
            is_first = False
        else:
            sound(source="po.mp3")

        # 正確にはinstruct全体で{keep_time}秒になる必要があるが、多少長くなっても問題ないのでこのまま
        time.sleep(1)

    return True


# =======
# タスク
# チューニング用の場合はpost_taskとpre_taskを使用しない
# =======
paper_keep_time = param["monitor"]["do_task"]["paper_keep_time"]
rock_keep_time = param["monitor"]["do_task"]["rock_keep_time"]
rest_keep_time = param["monitor"]["do_task"]["rest_keep_time"]

pre_task = [
    ["rest", rest_keep_time]
]*param["monitor"]["do_task"]["pre_task_repeat_num"]

task = [
    ["paper", paper_keep_time],
    ["rest", rest_keep_time],
    ["rock", rock_keep_time],
    ["rest", rest_keep_time],
]*param["monitor"]["do_task"]["task_repeat_num"]

post_task = [
    ["rest", rest_keep_time]
]*param["monitor"]["do_task"]["post_task_repeat_num"]


pygame.mixer.init()

# monitorがセットされているか確認
# 5秒待ってもセットされなければエラーを出す
count = 0
while True:
    t = -99
    with open(monitor_file, "r") as f:
        # for line in f.readlines()[::-1]:
        # if "time: " in line:
        #     t = int(re.sub("[^0-9]", "", line))
        break
    if t == -99:
        print("input_external.py: waiting for setup...")
        time.sleep(1)

        count += 1
        if count > 5:
            raise EOFError("monitor.txtに不備があります。")
    else:
        break

# 初期化
with open(label_file, "w") as f:
    f.write("\n")

# 10秒待機
waiting_time_seconds = 10
print(f"### {waiting_time_seconds}秒後に開始します ###")
for i in range(waiting_time_seconds, 0, -1):
    time.sleep(1)

# チューニング用 -> taskのみ
# タスク用 -> pre-task + task + post-task
time.sleep(3)
if int(args.taskNumber) == 0:
    all_task = pre_task + task
else:
    all_task = pre_task + task + post_task

for task_ in all_task:

    instruction = task_[0]
    keep_time = task_[1]

    instruct(instruction=instruction, keep_time=keep_time)

pygame.mixer.music.stop()

print("課題は終了しました。Ctrl+Cを押してください。")
