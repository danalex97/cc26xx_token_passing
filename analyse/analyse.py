import sys

from log_entries import LogEntry
from log_entries import BroadcastSentEntry
from log_entries import BroadcastRecvEntry

from log_processing import get_log
from log_processing import group_by
from log_processing import filter_entries

from collections import defaultdict

from plot import make_canvases
from plot import Canvas
import time

def pdr(log_entries):
    send_entries = filter_entries(log_entries, BroadcastSentEntry)
    recv_entries = filter_entries(log_entries, BroadcastRecvEntry)

    sent_by_id = group_by(send_entries, lambda entry: entry.id)
    recv_by_id = group_by(recv_entries, lambda entry: entry.from_id)

    pdr_per_node = {}
    for node_id in sent_by_id.keys():
        pdr_per_node[node_id] = []
        interval = 1000

        send_entries = group_by(sent_by_id[node_id], lambda entry: int(entry.timestamp / interval))
        recv_entries = group_by(recv_by_id[node_id], lambda entry: int(entry.timestamp / interval))

        for i in sorted(list(set(send_entries.keys()) | set(recv_entries.keys()))):
            if not i in send_entries or not i in recv_entries:
                continue

            send_entries_per_interval = send_entries[i]
            recv_entries_per_interval = recv_entries[i]
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

if __name__ == "__main__":
    log_file = sys.argv[1]

    log_entries = get_log(log_file)
    pdr_per_node = pdr(log_entries)

    fig, axs = make_canvases()

    upd_interval = 30
    canvases = {}
    idx = 0
    for key in pdr_per_node.keys():
        canvases[key] = Canvas(fig, axs[idx], upd_interval, "Node {}".format(key))
        idx += 1

    current_index = 0
    while True:
        for key, canvas in canvases.items():
            canvas.update_data(
                range(current_index, current_index + upd_interval),
                pdr_per_node[key][current_index : current_index + upd_interval])
        for key, canvas in canvases.items():
            canvas.draw()
        time.sleep(0.1)
        current_index += 1
