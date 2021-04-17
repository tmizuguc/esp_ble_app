import subprocess
import asyncio
import time
import argparse
import glob

parser = argparse.ArgumentParser()
parser.add_argument('--taskNumber', type=int, default=1)
parser.add_argument('--onlyMonitor', action='store_true')
parser.add_argument('--onlyTuning', action='store_true')
parser.add_argument('--flipImage', action='store_true')
args = parser.parse_args()


async def view_task(task_number, flipImage):
    '''課題のステート画面を起動する'''
    print("start view_task")
    if flipImage:
        subprocess.Popen(
            f"python monitor/view_task.py --taskNumber {task_number} --flipImage", shell=True)
    else:
        subprocess.Popen(
            f"python monitor/view_task.py --taskNumber {task_number}", shell=True)
    print("end view_task")


async def view_monitor(task_number):
    '''筋電をモニターするための画面を起動する'''
    print("start view_monitor")
    subprocess.Popen(
        f"python monitor/view_monitor.py --taskNumber {task_number}", shell=True)
    print("end view_monitor")


async def do_task(task_number):
    '''
    課題を実行する。
    '''
    print("start do_task")
    while True:
        try:
            subprocess.Popen(
                f"python monitor/do_task.py --taskNumber {task_number}", shell=True)
            break
        except:
            print("wait for setting up...")
            time.sleep(1)

    print("end do_task")


async def do_monitor(task_number):
    '''
    ESPから信号処理後の筋電を継続的に取得して、monitor.txtへ書き込み続ける。
    '''
    print("start do_monitor")
    subprocess.call(
        f"python monitor/do_monitor.py --taskNumber {task_number}", shell=True)
    print("end do_monitor")


def main():
    loop = asyncio.get_event_loop()

    if args.onlyMonitor:
        task_number = args.taskNumber
        loop.run_until_complete(asyncio.gather(
            *[view_monitor(999), do_monitor(999)]))
    else:

        # チューニングは固定で0番目
        if args.onlyTuning:
            task_number = 0
        else:
            task_number = args.taskNumber
            # すでにファイルが存在していたらエラー
            exist_label_file = len(
                glob.glob(f"monitor/file/label/label_{task_number}.txt")) > 0
            exist_monitor_file = len(
                glob.glob(f"monitor/file/monitor/monitor_{task_number}.txt")) > 0
            if exist_label_file | exist_monitor_file:
                raise ValueError(
                    f"label_{task_number}.txtか、monitor_{task_number}.txtが存在しています。")

        loop.run_until_complete(asyncio.gather(
            *[view_task(task_number, args.flipImage), view_monitor(task_number), do_task(task_number), do_monitor(task_number)]))


if __name__ == '__main__':
    main()