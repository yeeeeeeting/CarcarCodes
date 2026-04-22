from bt_interface import BTInterface
import time

PORT = 'COM5'

if __name__ == '__main__':
    def getQueryFunc():
        index = 0
        actions = ['R0','B0','F0','B0','L0','B0','F0','B0']
        n = len(actions)
        def queryFunc():
            nonlocal index, actions, n
            response = actions[index]
            index = (index + 1) % n
            return response
        return queryFunc

    interface = BTInterface(PORT, 'HM10_TM8', getQueryFunc())
    interface.connect()
    while not interface.isReady():
        print("Waiting for ready signal...")
        time.sleep(0.2)
        msg = interface.bridge.listen()
        if len(msg) > 0:
            interface.agent.on_message(msg)
    input("Press Enter to Continue...")
    interface.activate()