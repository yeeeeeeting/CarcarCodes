from hm10_esp32 import HM10ESP32Bridge
import time
import sys
import threading
# Modified: import agent
from agent import SimpleAgent

PORT = 'COM5'
EXPECTED_NAME = 'HM10_team8'

# Original
"""
def background_listener(bridge):
    while True:
        msg = bridge.listen()
        if msg:
            print(f"\r[HM10]: {msg}")
            print("You: ", end="", flush=True)
        time.sleep(0.1)
"""
# Modified
def background_listener(bridge, agent):
    print("activation")
    bridge.send(agent.activation_msg())
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

def main():
    bridge = HM10ESP32Bridge(port=PORT)
    
    # 1. Configuration Check
    current_name = bridge.get_hm10_name()
    if current_name != EXPECTED_NAME:
        print(f"Target mismatch. Current: {current_name}, Expected: {EXPECTED_NAME}")
        # if current_name != None:
        return False

        print(f"Updating target name to {EXPECTED_NAME}...")
        
        if bridge.set_hm10_name(EXPECTED_NAME):
            print("✅ Name updated successfully. Resetting ESP32...")
            bridge.reset()
            # Re-init after reset
            bridge = HM10ESP32Bridge(port=PORT)
        else:
            print("❌ Failed to set name. Exiting.")
            # sys.exit(1)
            return False

    # 2. Connection Check
    status = bridge.get_status()
    if status != "CONNECTED":
        print(f"⚠️ ESP32 is {status}. Please ensure HM-10 is advertising. Exiting.")
        # sys.exit(0)
        return False

    print(f"✨ Ready! Connected to {EXPECTED_NAME}")

    # Modified: added agent object and as an argument
    agent = SimpleAgent()
    # threading.Thread(target=background_listener, args=(bridge, agent), daemon=True).start()
    background_listener(bridge, agent)

    try:
        while True:
            # Now the work is done by agent
            """
            user_msg = input("You: ")
            if user_msg.lower() in ['exit', 'quit']: break
            if user_msg: bridge.send(user_msg)
            """
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    print("\nChat closed.")
    return True

if __name__ == "__main__":
    while not main():
        input("input anything to retry")
