class LogEntry():
    def __init__(self, raw_entry):
        raw_fields = raw_entry.replace('\t', ' ').split(" ", 2)

        try:
            self.timestamp = float(raw_fields[0])
        except:
            mins = float(raw_fields[0].split(":")[0])
            secs = float(raw_fields[0].split(":")[1])
            self.timestamp = (mins * 60 + secs) * 1000

        self.id = int(raw_fields[1].split(":")[1])
        self.msg = raw_fields[2]

    def __repr__(self):
        return "<{} ID:{}>".format(self.timestamp, self.id)

class BroadcastSentEntry(LogEntry):
    def __init__(self, raw_entry):
        super(BroadcastSentEntry, self).__init__(raw_entry)
        assert("broadcast message sent" in self.msg)

        values = list(map(int, filter(lambda s: s.isnumeric(), self.msg.split(" "))))
        self.msg_id = values[0]

    def __repr__(self):
        return "<{} ID:{} BROADCAST-SENT(msg: {})>".format(self.timestamp, self.id, self.msg_id)

class BroadcastRecvEntry(LogEntry):
    def __init__(self, raw_entry):
        super(BroadcastRecvEntry, self).__init__(raw_entry)
        assert("[Periodic] received from" in self.msg)

        def is_float(element):
            try:
                float(element)
                return True
            except ValueError:
                return False

        self.msg = self.msg.replace('\'', ' ')
        self.msg = self.msg.replace(':', ' ')
        values = list(map(float, filter(lambda s: is_float(s), self.msg.split(" "))))
        values = list(map(int, values))

        self.from_id = values[0]
        self.msg_id  = values[1]

    def __repr__(self):
        return "<{} ID:{} BROADCAST-RECV(from: {}, msg: {})>".format(
            self.timestamp, self.id, self.from_id, self.msg_id
        )
        # return "<{} ID:{} BROADCAST-RECV({}: {} => {})>".format(
        #     self.timestamp, self.id, self.msg_id, self.from_id, self.to_id)
