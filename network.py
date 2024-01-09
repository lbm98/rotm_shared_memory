import os
import sys
import time
import signal
import subprocess

from pathlib import Path

THIS_SCRIPT_DIR = Path(__file__).resolve().parent

GNB_COMMAND = (
    THIS_SCRIPT_DIR / 'network/srsRAN_Project/build/apps/gnb/gnb',
    '-c',
    THIS_SCRIPT_DIR / './configs/gnb_zmq.yaml'
)

UE_COMMAND = (
    THIS_SCRIPT_DIR / 'network/rotm_srsRAN_4G_fork/cmake-build-debug/srsue/src/srsue',
    THIS_SCRIPT_DIR / './configs/ue_zmq.conf'
)


def start_gnb():
    return subprocess.Popen(GNB_COMMAND)


def start_ue():
    return subprocess.Popen(UE_COMMAND)


def main(debug_ue):
    gnb = start_gnb()

    if debug_ue:
        while True:
            time.sleep(1)

    ue = start_ue()

    while True:
        if ue.poll() is not None:
            break
        time.sleep(0.01)

    # The GNB should never crash
    # If it did crash, we signal it with a return-code of 1
    if gnb.poll() is not None:
        sys.exit(1)

    ue.send_signal(signal.SIGALRM)
    gnb.send_signal(signal.SIGALRM)

    while ue.poll() is None and gnb.poll is None:
        time.sleep(0.01)


if __name__ == '__main__':
    if os.geteuid() != 0:
        print("Please run this script using sudo")
        sys.exit(1)

    debug_ue = False
    if len(sys.argv) > 1 and sys.argv[1] == '--debug_ue':
        debug_ue = True

    main(debug_ue)
    sys.exit(0)
