import sys
import os
import subprocess

from log_entries import LogEntry
from log_entries import BroadcastSentEntry
from log_entries import BroadcastRecvEntry
from log_entries import BroadcastBaseRequestEntry
from log_entries import NodeJoinEntry

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

        recv_msg_ids = set([e.msg_id for e in recv_by_id[node_id]])

        for i in sorted(list(set(send_entries.keys()) | set(recv_entries.keys()))):
            # Look at interval i
            if not i in send_entries:
                continue
            if not i in recv_entries:
                continue

            # Calculate send entries / interval
            send_entries_per_interval = send_entries[i]

            # Calculate pdr
            min_msg_id = min([e.msg_id for e in recv_entries[i]])
            max_msg_id = min_msg_id + len(send_entries_per_interval)

            ctr = 0
            for msg_id in range(min_msg_id, max_msg_id):
                if msg_id in recv_msg_ids:
                    ctr += 1

            raport = ctr / (max_msg_id - min_msg_id)

            if raport < 1:
                print("Packet not received yet(or lost) at node {} in interval {}.".format(
                    node_id, i
                ))

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

def get_nodes(log_entries):
    node_entries = filter_entries(log_entries, NodeJoinEntry)
    return [e.node_id for e in node_entries][0:9]

def get_avg_pdr(log_entries):
    send_entries = filter_entries(log_entries, BroadcastBaseRequestEntry)
    recv_entries = filter_entries(log_entries, BroadcastRecvEntry)

    send_by_id = group_by(send_entries, lambda entry: entry.sent_id)
    recv_by_id = group_by(recv_entries, lambda entry: entry.from_id)

    pdrs = []
    for node_id in recv_by_id.keys():
        recv_msgs = [e.msg_id for e in recv_by_id[node_id]
            if e.timestamp > 60000 and e.timestamp < 300000]

        min_idx_ids = min(recv_msgs)
        max_idx_ids = max(recv_msgs)

        ctr = 0
        recv_set = set([e.msg_id for e in recv_by_id[node_id]])
        for i in range(min_idx_ids, max_idx_ids):
            if i in recv_set:
                ctr += 1
        pdrs.append(float(ctr) / float(max_idx_ids - min_idx_ids + 1))
    return float(sum(pdrs)) / float(len(pdrs))


if __name__ == "__main__":
    log_file = sys.argv[1]
    # run_cooja()

    log_entries  = get_log(log_file)
    node_ids     = get_nodes(log_entries)

    fig, axs = make_canvases()

    upd_interval = 30
    canvases = {}
    idx = 0
    for key in node_ids:
        canvases[key] = Canvas(fig, axs[idx], upd_interval, "Node {}".format(key))
        idx += 1

    def animate(current_index):
        log_entries = get_log(log_file)
        pdr_per_node = pdr(log_entries)

        print("Current total pdr: {}".format(get_avg_pdr(log_entries)))

        slide = max(0, int(log_entries[-1].timestamp / interval) - upd_interval)
        current_index = min(current_index, slide)

        for key, canvas in canvases.items():
            if key not in pdr_per_node:
                continue
            canvas.update_data(
                range(current_index, current_index + upd_interval),
                pdr_per_node[key][current_index : current_index + upd_interval])

    canvases[node_ids[0]].run(animate)
