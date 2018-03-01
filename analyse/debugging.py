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

if __name__ == "__main__":
    log_file = sys.argv[1]
    log_entries = [e for e in get_log(log_file)
        if  e.timestamp > 60000]
    send_entries = filter_entries(log_entries, BroadcastBaseRequestEntry)
    recv_entries = filter_entries(log_entries, BroadcastRecvEntry)

    send_by_id = group_by(send_entries, lambda entry: entry.sent_id)
    recv_by_id = group_by(recv_entries, lambda entry: entry.from_id)

    print([e.msg_id for e in recv_by_id[3]])

    for j in range(2, 11):
        max_msg_id = max([e.msg_id for e in recv_by_id[j]])
        min_msg_id = min([e.msg_id for e in recv_by_id[j]])
        lst = [int(e.msg_id) for e in recv_by_id[j]]
        for i in range(min_msg_id, max_msg_id):
            if i not in lst:
                print("Node {}: missing {}".format(j, i))
        print(j, len(lst), min_msg_id, max_msg_id)
