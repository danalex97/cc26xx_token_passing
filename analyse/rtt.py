import sys
import os
import subprocess

from log_entries import LogEntry
from log_entries import PrioritySentEntry
from log_entries import PriorityRecvEntry

from log_processing import get_log
from log_processing import group_by
from log_processing import filter_entries

from collections import defaultdict

from plot import make_canvases
from plot import Canvas
import time
import signal

def rtts(log_entries):
    sent_entries = filter_entries(log_entries, PrioritySentEntry)
    recv_entries = filter_entries(log_entries, PriorityRecvEntry)

    # group entries by node
    send_by_id = group_by(sent_entries, lambda entry: entry.node_id)
    recv_by_id = group_by(recv_entries, lambda entry: entry.node_id)

    rtt_per_node = {}
    for node_id in recv_by_id.keys():
        sent_entries = send_by_id[node_id]
        recv_entries = recv_by_id[node_id]

        rtts = []
        for sent_entry in sent_entries:
            t1 = sent_entry.timestamp
            next_recv_entry = [e for e in recv_entries if e.timestamp > t1][0]
            t2 = next_recv_entry.timestamp

            rtt = t2 - t1
            rtts.append((t1, rtt))
        rtt_per_node[node_id] = rtts
    return rtt_per_node


if __name__ == "__main__":
    log_file = sys.argv[1]
    # run_cooja()

    # fig, axs = make_canvases()

    # upd_interval = 30
    # canvases = {}
    # idx = 0
    # for key in range(2, 11):
    #     canvases[key] = Canvas(fig, axs[idx], upd_interval, "Node {}".format(key))
    #     idx += 1

    log_entries  = get_log(log_file)
    rtt_per_node = rtts(log_entries)
    print(rtt_per_node)

    # def animate(current_index):
    #     log_entries = get_log(log_file)
    #     pdr_per_node = pdr(log_entries)
    #
    #     slide = max(0, int(log_entries[-1].timestamp / interval) - upd_interval)
    #     current_index = min(current_index, slide)
    #
    #     for key, canvas in canvases.items():
    #         if key not in pdr_per_node:
    #             continue
    #         canvas.update_data(
    #             range(current_index, current_index + upd_interval),
    #             pdr_per_node[key][current_index : current_index + upd_interval])
    #
    # canvases[2].run(animate)
