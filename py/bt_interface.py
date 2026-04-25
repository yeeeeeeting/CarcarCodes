from hm10_esp32 import HM10ESP32Bridge
import time
import sys
import threading
import queue
from agent import Agent

class BTInterface:
    def __init__(self, port: str, hm10_name: str, queryFunc):
        self.port = port
        self.rfids = queue.Queue()
        self.agent = Agent(queryFunc, self.putRFID)
        self.bridge = None
        self.expected_name = hm10_name

    @staticmethod
    def background_listener(bridge, agent):
        while not agent.auto_listen:
            None
        print('Background: start')
        while True:
            msg = bridge.listen()
            if msg:
                print(f"\r[HM10]: {msg}")
                reply = agent.on_message(msg)
                if reply:
                    bridge.send(reply)
                print("reply = ", reply)
            else:
                # print("no msg")
                reply = agent.check_no_response()
                if reply:
                    bridge.send(reply)
                    print("reply = ", reply)
            time.sleep(0.1)

    def connect(self):
        self.bridge = HM10ESP32Bridge(port=self.port)
        connected = False

        print(f"Searching for {self.expected_name}")

        while not connected:
            try:
                status = self.bridge.get_status()
                name = self.bridge.get_hm10_name()

                if status == 'DISCONNECTED' and name == None:
                    print('Target is disconnected with no name, try updating name...')
                    if self.bridge.set_hm10_name(self.expected_name):
                        print("✅ Name updated successfully. Resetting ESP32...")
                        self.bridge.reset()
                        # Re-init after reset
                        self.bridge = HM10ESP32Bridge(port=self.port)
                        continue
                    else:
                        print("❌ Failed to set name. Retry searching...")
                        continue

                if status != "CONNECTED":
                    print("Still searching for HM-10 signal...")
                    time.sleep(2)
                    continue

                current_name = self.bridge.get_hm10_name()

                if current_name == self.expected_name:
                    print(f"✅ Correct device found: {self.expected_name}")
                    connected = True
                else:
                    print(f"Target mismatch. Current: {current_name}, Expected: {self.expected_name}. Still looking...")
                    time.sleep(2)

            except Exception as e:
                print(f"Connection error: {e}. Retrying...")
                time.sleep(2)

        threading.Thread(target=self.background_listener, args=(self.bridge, self.agent), daemon=False).start()

        print("\nChat closed.")
        return True

    def isReady(self):
        return self.agent.isReady

    def activate(self):
        print("try activate")
        if self.isReady():
            print("activate")
            self.bridge.send(self.agent.get_activation_msg())

    def putRFID(self, rfid):
        self.rfids.put(rfid)

    def getRFID(self):
        if self.rfids.empty():
            return None
        else:
            return self.rfids.get()

"""
if __name__ == "__main__":
    while not connect():
        input("input anything to retry")
"""