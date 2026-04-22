import time

class Agent:
    CMD_END = "ED"
    def __init__(self, queryFunc, uploadFunc):
        self.state = 0
        self.counter = 0
        self.actions = ['L1', 'R0', 'E0']
        self.repeat = ''
        self.last_msg_time = time.time()
        self.isReady = False
        self.isActivated = False
        self.queryFunc = queryFunc
        self.uploadFunc = uploadFunc
        self.auto_listen = False

    @classmethod
    def __activation_msg(cls):
        return 'a' + cls.CMD_END;

    def get_activation_msg(self):
        if self.isReady:
            self.repeat = self.__activation_msg()
            self.auto_listen = True
            return self.repeat
        else:
            return None

    def update_msg_time(self):
        self.last_msg_time = time.time()

    def check_no_response(self):
        # print("check no response")
        if not self.isActivated and self.last_msg_time - self.last_msg_time > 2.0:
            return self.repeat
        else:
            return None

    def on_message(self, msg: str):
        msg = msg.strip()

        if msg[0] == 'v':
            if self.repeat == self.__activation_msg():
                self.isActivated = True
            self.update_msg_time()
            print("Success")
            return None
        
        if msg[0] == 'k':
            # print("received k")
            self.isReady = True
            # print("agent.isReady = ", self.isReady)
            if self.repeat == self.__activation_msg():
                return self.repeat
            else:
                return None

        if msg[0] == 'A':
            self.update_msg_time()
            if msg.endswith("AT+RESET"):
                self.counter = 0
                time.sleep(3.5)
                self.repeat = self.__activation_msg()
                return self.repeat
            else:
                return None

        if msg[0] == 'q':
            # self.update_msg_time()
            result = self.queryFunc()
            self.repeat = result + self.CMD_END
            return self.repeat

        if msg[0] == 'r':
            # self.update_msg_time()
            # vacuous query
            self.queryFunc()
            code = self.parse(msg[1:5])
            if self.upload(code):
                return 'v' + self.CMD_END
            else:
                self.repeat = 'x' + self.CMD_END
                return self.repeat

        else:
            # self.update_msg_time()
            self.repeat = 'x' + self.CMD_END
            return self.repeat

    @staticmethod
    def hexChar(value: int) -> (chr | None):
        if value < 0:
            return None
        elif value < 10:
            return chr(ord('0') + value)
        elif value < 16:
            return chr(ord('A') + (value - 10))
        else:
            return None

    @staticmethod
    def parse(msg: str):
        code = str()
        for c in msg:
            c = ord(c)
            code += Agent.hexChar((c & 0xf0) >> 4)
            code += Agent.hexChar((c & 0x0f))
        return code

    def upload(self, code: str):
        print("uploading:", code)
        self.uploadFunc(code)
        return True



if __name__ == "__main__":
    msg = ""
    def encodeByteArray(msg: list):
        array = []
        byte = 0
        bits = 0
        for ch in msg:
            if ch != 0 and ch != '0':
                byte = (byte << 1) | 1
            else:
                byte = byte << 1
            bits += 1
            if bits >= 8:
                array.append(byte)
                byte = 0
                bits = 0
        if bits != 0:
            byte = byte << (8 - bits)
            array.append(byte)
        return array

    def encodeByteString(arr: list):
        res = ""
        for byte in arr:
            res += chr(byte)
        return res

    while True:
        bits = input("type a byte string: ")
        if (bits == "exit") or (bits == "quit"):
            break
        bytes = encodeByteArray(bits)
        s = encodeByteString(bytes)
        print("$ bytes =", bytes)
        print("$ s =", s)
        print("$ result =", Agent.parse(s))
