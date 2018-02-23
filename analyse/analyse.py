import sys

from log_entries import LogEntry
from log_entries import BroadcastSentEntry
from log_entries import BroadcastRecvEntry

from log_processing import get_log
from log_processing import group_by
from log_processing import filter_entries

from collections import defaultdict

def pdr(log_entries):
    send_entries = filter_entries(log_entries, BroadcastSentEntry)
    recv_entries = filter_entries(log_entries, BroadcastRecvEntry)

    sent_by_id = group_by(send_entries, lambda entry: entry.id)
    recv_by_id = group_by(recv_entries, lambda entry: entry.from_id)

    for node_id in sent_by_id.keys():
        send_entries = sent_by_id[node_id]
        recv_entries = recv_by_id[node_id]

        raport = len(recv_entries) / len(send_entries)
        print(raport)

if __name__ == "__main__":
    log_file = sys.argv[1]

    log_entries = get_log(log_file)
    pdr(log_entries)
