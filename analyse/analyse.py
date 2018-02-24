import sys
import os
import subprocess

from log_entries import LogEntry
from log_entries import BroadcastSentEntry
from log_entries import BroadcastRecvEntry

from log_processing import get_log
from log_processing import pool_log
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
    send_entries = filter_entries(log_entries, BroadcastSentEntry)
    recv_entries = filter_entries(log_entries, BroadcastRecvEntry)

    sent_by_id = group_by(send_entries, lambda entry: entry.id)
    recv_by_id = group_by(recv_entries, lambda entry: entry.from_id)

    pdr_per_node = {}
    for node_id in sent_by_id.keys():
        pdr_per_node[node_id] = []

        if node_id not in sent_by_id: sent_by_id[node_id] = []
        if node_id not in recv_by_id: recv_by_id[node_id] = []

        send_entries = group_by(sent_by_id[node_id], lambda entry: int(entry.timestamp / interval))
        recv_entries = group_by(recv_by_id[node_id], lambda entry: int(entry.timestamp / interval))

        for i in sorted(list(set(send_entries.keys()) | set(recv_entries.keys()))):
            if not i in send_entries:
                continue

            send_entries_per_interval = send_entries[i]

            recv_entries_per_interval = []
            if i in recv_entries:
                recv_entries_per_interval += recv_entries[i]
            if i + 1 in recv_entries:
                recv_entries_per_interval += recv_entries[i + 1]

            send_entries_map = group_by(send_entries_per_interval, lambda entry: entry.msg_id)
            recv_entries_map = group_by(recv_entries_per_interval, lambda entry: entry.msg_id)

            count_recv_msg = 0
            # count recv_msg by message id
            for msg_id in send_entries_map.keys():
                if msg_id in recv_entries_map:
                    count_recv_msg += 1

            raport = count_recv_msg / len(send_entries_per_interval)
            pdr_per_node[node_id].append(raport)
    return pdr_per_node

def run_cooja():
    os.system("rm COOJA.log")
    os.system("rm COOJA.testlog")
    pid = subprocess.Popen(['/bin/sh', './run.sh'])
    time.sleep(10)
    def sigint_handler(signal, frame):
        os.system("pkill -9 java")
        sys.exit(0)
    signal.signal(signal.SIGINT, sigint_handler)

if __name__ == "__main__":
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
        new_log_entries = pool_log(log_file)

        log_entries += new_log_entries
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

        textbox.set_text("INFO")
        textbox.append_separator()
        for key, node_pdr in pdr_per_node.items():
            avg_pdr = sum(node_pdr) / len(node_pdr)
            textbox.append_text("Node {} pdr: {}".format(key, avg_pdr))
        textbox.append_separator()
        textbox.render()

        if slide > current_index:
            current_index += 1
