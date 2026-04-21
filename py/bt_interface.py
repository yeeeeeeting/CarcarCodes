from hm10_esp32 import HM10ESP32Bridge
import time
import sys
import threading
import queue
from agent import SimpleAgent

class BTInterface:
    def __init__(self, port: str):
        self.port = port
        self.rfids = queue.Queue()
    EXPECTED_NAME = 'HM10_TM8'

    @staticmethod
    def background_listener(bridge, agent):
        input("type anything with enter to activate")
        print("activation")
        agent.repeat = agent.activation_msg()
        bridge.send(agent.repeat)
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
        bridge = HM10ESP32Bridge(port=self.port)
        connected = False

        print(f"Searching for {self.EXPECTED_NAME}")

        while not connected:
            try:
                status = bridge.get_status()
                name = bridge.get_hm10_name()

                if status == 'DISCONNECTED' and name == None:
                    print('Target is disconnected with no name, try updating name...')
                    if bridge.set_hm10_name(self.EXPECTED_NAME):
                        print("✅ Name updated successfully. Resetting ESP32...")
                        bridge.reset()
                        # Re-init after reset
                        bridge = HM10ESP32Bridge(port=self.port)
                        continue
                    else:
                        print("❌ Failed to set name. Retry searching...")
                        continue

                if status != "CONNECTED":
                    print("Still searching for HM-10 signal...")
                    time.sleep(2)
                    continue

                current_name = bridge.get_hm10_name()

                if current_name == self.EXPECTED_NAME:
                    print(f"✅ Correct device found: {self.EXPECTED_NAME}")
                    connected = True
                else:
                    print(f"Target mismatch. Current: {current_name}, Expected: {self.EXPECTED_NAME}. Still looking...")
                    time.sleep(2)

            except Exception as e:
                print(f"Connection error: {e}. Retrying...")
                time.sleep(2)

        agent = SimpleAgent(self.pushRFID)
        threading.Thread(target=self.background_listener, args=(bridge, agent), daemon=False).start()

        print("\nChat closed.")
        return True

    def pushRFID(self, rfid):
        self.rfids.put(rfid)

    def popRFID(self):
        if self.rfids.empty():
            return None
        else:
            return self.rfids.get()

"""
if __name__ == "__main__":
    while not connect():
        input("input anything to retry")
"""