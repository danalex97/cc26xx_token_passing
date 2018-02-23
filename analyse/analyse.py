import sys

from log_entries import LogEntry
from log_entries import BroadcastSentEntry
from log_entries import BroadcastRecvEntry

from log_processing import get_log
from log_processing import group_by
from log_processing import filter_entries

from collections import defaultdict

# def average_loss_rate(log_entries, node_log):
#     loss_rates = []
#
#     send_entries = filter_entries(log_entries, BroadcastSentEntry)
#     recv_entries = filter_entries(log_entries, BroadcastRecvEntry)
#
#     sent_by_id = group_by(send_entries, lambda entry: entry.msg_id)
#     recv_by_id = group_by(recv_entries, lambda entry: entry.msg_id)
#
#     total_nodes = len(node_log.keys())
#     for msg_id in sent_by_id.keys():
#         try:
#             received_node_ids = [e.id for e in recv_by_id[msg_id]]
#             received_nodes = len(set(received_node_ids))
#
#             loss_rates.append(received_nodes/total_nodes)
#         except:
#             pass
#     return sum(loss_rates) / len(loss_rates)

if __name__ == "__main__":
    log_file = sys.argv[1]

    log_entries = get_log(log_file)
    for entry in log_entries:
        print(entry)
    # node_log = group_by(log_entries, lambda entry: entry.id)

    # loss_rate          = average_loss_rate(log_entries, node_log)

    # print("Loss rate: {}".format(loss_rate))
