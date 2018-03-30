import sys
import os
import subprocess

from log_entries import LogEntry
from log_entries import PrioritySentEntry
from log_entries import PriorityRecvEntry
from log_entries import NodeJoinEntry

from log_processing import get_log
from log_processing import group_by
from log_processing import filter_entries

from collections import defaultdict

from plot import make_canvases
from plot import Canvas

from pdr import run_cooja
from pdr import get_nodes

global interval
interval = 1000

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

    fig, axs = make_canvases()

    log_entries  = get_log(log_file)
    node_ids     = get_nodes(log_entries)

    upd_interval = 30
    canvases = {}
    idx = 0
    for key in node_ids:
        canvases[key] = Canvas(fig, axs[idx], upd_interval, "Node {}".format(key), y_label="rtt(ms)", scatter=True)
        idx += 1

    def animate(_unused):
        log_entries  = get_log(log_file)
        rtt_per_node = rtts(log_entries)

        for key, canvas in canvases.items():
            if key not in rtt_per_node:
                continue
            to_display = [(float(ts)/float(interval), rtt)
                for (ts, rtt) in rtt_per_node[key]]

            xs = [x for x, y in to_display]
            ys = [y for x, y in to_display]

            canvas.update_scatter_data(xs, ys)

    canvases[node_ids[0]].run(animate)
