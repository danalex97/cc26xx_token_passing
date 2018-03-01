import sys
import os
import subprocess

from log_entries import LogEntry
from log_entries import BroadcastSentEntry
from log_entries import BroadcastRecvEntry
from log_entries import BroadcastBaseRequestEntry

from log_processing import get_log
from log_processing import group_by
from log_processing import filter_entries

from collections import defaultdict

from plot import make_canvases
from plot import Canvas
import time
import signal

global interval
interval = 1000

def pdr(log_entries):
    # Map from node_id -> pdr list for each interval
    pdr_per_node = {}

    send_entries = filter_entries(log_entries, BroadcastBaseRequestEntry)
    recv_entries = filter_entries(log_entries, BroadcastRecvEntry)

    send_by_id = group_by(send_entries, lambda entry: entry.sent_id)
    recv_by_id = group_by(recv_entries, lambda entry: entry.from_id)

    pdr_per_node = {}
    for node_id in recv_by_id.keys():
        pdr_per_node[node_id] = []

        # Group entries by time interval
        send_entries = group_by(send_by_id[node_id], lambda entry: int(entry.timestamp / interval))
        recv_entries = group_by(recv_by_id[node_id], lambda entry: int(entry.timestamp / interval))

        for i in sorted(list(set(send_entries.keys()) | set(recv_entries.keys()))):
            # Look at interval i
            if not i in send_entries:
                continue

            # Calculate send entries / interval
            send_entries_per_interval = send_entries[i]

            recv_entries_per_interval = []
            if i in recv_entries:
                recv_entries_per_interval += recv_entries[i]
            if i + 1 in recv_entries:
                recv_entries_per_interval += recv_entries[i + 1]

            # Calculate pdr
            recv_msg_ids = set([e.msg_id for e in recv_entries_per_interval])
            if len(recv_msg_ids) == 0:
                continue

            min_msg_id = min(recv_msg_ids)
            max_msg_id = min_msg_id + len(send_entries_per_interval)

            ctr = 0
            for msg_id in range(min_msg_id, max_msg_id):
                if msg_id in recv_msg_ids:
                    ctr += 1

            raport = ctr / (max_msg_id - min_msg_id)
            pdr_per_node[node_id].append(raport)
    return pdr_per_node

    # for j in range(2, 11):
    #     max_msg_id = max([e.msg_id for e in recv_by_id[j]])
    #     min_msg_id = max([e.msg_id for e in recv_by_id[j]])
    #     lst = [int(e.msg_id) for e in recv_by_id[j]]
    #     for i in range(min_msg_id, max_msg_id):
    #         if i not in lst:
    #             print("Node {}: missing {}".format(j, i))
    #     print(len(lst))

def run_cooja():
    os.system("rm COOJA.log")
    os.system("rm COOJA.testlog")
    pid = subprocess.Popen(['/bin/sh', './run.sh'])
    time.sleep(10)
    def sigint_handler(signal, frame):
        os.system("pkill -9 java")
        sys.exit(0)
    signal.signal(signal.SIGINT, sigint_handler)

def draw_textbox():
    textbox.set_text("INFO")
    textbox.append_separator()

    # Filter old log entries
    useful_log_entries = [e for e in log_entries
        if  e.timestamp > 60000
        and e.timestamp < (current_index + upd_interval) * 1000]
    pdr_per_node = pdr(useful_log_entries)

    for key, node_pdr in pdr_per_node.items():
        avg_pdr = sum(node_pdr) / len(node_pdr)
        textbox.append_text("Node {} pdr: {}".format(key, avg_pdr))
    textbox.append_separator()
    textbox.render()

if __name__ == "__main__":
    # # DEBUG...
    # log_file = sys.argv[1]
    # log_entries = [e for e in get_log(log_file)
    #         if  e.timestamp > 60000]
    # print(pdr(log_entries)[3])

    log_file = sys.argv[1]
    run_cooja()

    fig, axs, textbox = make_canvases()

    upd_interval = 30
    canvases = {}
    idx = 0
    for key in range(2, 11):
        canvases[key] = Canvas(fig, axs[idx], upd_interval, "Node {}".format(key))
        idx += 1

    log_entries = []
    current_index = 0
    while True:
        log_entries = get_log(log_file)
        pdr_per_node = pdr(log_entries)

        slide = max(0, int(log_entries[-1].timestamp / interval) - upd_interval)
        current_index = min(current_index, slide)

        for key, canvas in canvases.items():
            if key not in pdr_per_node:
                continue
            canvas.update_data(
                range(current_index, current_index + upd_interval),
                pdr_per_node[key][current_index : current_index + upd_interval])
        for key, canvas in canvases.items():
            canvas.draw()

        # draw_textbox()

        if slide > current_index:
            current_index += 1
