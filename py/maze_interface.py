import queryer
import time
import threading


class StrategyFiles:
    def __init__(self, specs, params, maze, strategy):
        self.specs_path = specs
        self.params_path = params
        self.maze_path = maze
        self.strategy_path = strategy

class MazeInterface:
    def __init__(self, strategy: StrategyFiles):
        self.end_time = 0
        self.__queryer = queryer.Queryer(strategy.specs_path, strategy.params_path, strategy.maze_path, strategy.strategy_path)
        self.thread = threading.Thread(target=queryer.queryerLoop, args=(self.__queryer,))
        self.thread.start()
        self.isReady = False
        self.activated = False

    def checkReady(self):
        if self.isReady:
            return True
        if self.__queryer.outputQueue.empty():
            return False
        msg = self.__queryer.outputQueue.get()
        if msg.strip() == 'ok':
            self.isReady = True
            return True
        return False
    
    def activate(self, end_time: float):
        if self.isReady and not self.activated:
            self.activated = True
            self.end_time = end_time
            return True
        return False
    
    def close(self):
        self.__queryer.inputQueue.put('quit')
        self.__queryer.close()
        self.thread.join()

    def query(self):
        self.__queryer.inputQueue.put(self.end_time - time.time())

    def getResponse(self):
        if self.__queryer.outputQueue.empty():
            return None
        else:
            return self.__queryer.outputQueue.get().strip()

    def queryForResponse(self):
        self.query()
        response = self.getResponse()
        while response == None:
            response = self.getResponse()
        return response


"""
if __name__ == '__main__':
    mazeInterface = MazeInterface()
    while not mazeInterface.isReady:
        mazeInterface.checkReady()
    input('Ready! Press Enter to Continue...')

    INIT_TIME = 65
    mazeInterface.activate(time.time() + INIT_TIME)

    while True:
        if mazeInterface.waitForNode() <= 0:
            break
        mazeInterface.query()
        response = mazeInterface.getResponse()
        while response == None:
            response = mazeInterface.getResponse()
        print("Current remaining time:", mazeInterface.end_time - time.time())
        print("Responded command:", response)
        print("")
    
    mazeInterface.close()
    print("End")
    
"""
