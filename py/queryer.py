import subprocess
import threading
import queue

class Queryer:
    def __init__(self, specs_path, params_path, maze_path, strategy_path):
        self.stop_event = threading.Event()
        self.proc = subprocess.Popen(
            ["./analyzer/query_resolver.exe", specs_path, params_path, maze_path, strategy_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )
        self.inputQueue = queue.Queue()
        self.outputQueue = queue.Queue()
        self.errorQueue = queue.Queue()
        self.__stdout_listener = threading.Thread(target=self.__stdout_listener_loop)
        self.__stderr_listener = threading.Thread(target=self.__stderr_listener_loop)
        self.__stdout_listener.start()
        self.__stderr_listener.start()
    
    def __stdout_listener_loop(self):
        while not self.stop_event.is_set() and self.proc.poll() is None:
            line = self.proc.stdout.readline()
            if line:
                self.outputQueue.put(line)
            else:
                break
    
    def __stderr_listener_loop(self):
        while not self.stop_event.is_set() and self.proc.poll() is None:
            line = self.proc.stderr.readline()
            if line:
                self.errorQueue.put(line)
            else:
                break

    def close(self):
        if self.proc.poll() is None:
            self.proc.terminate()
            self.proc.wait()
        self.stop_event.set()
        self.__stdout_listener.join()
        self.__stderr_listener.join()
    
    def input(self):
        if not self.inputQueue.empty():
            try:
                self.proc.stdin.write(str(self.inputQueue.get()) + '\n')
                self.proc.stdin.flush()
            except OSError:
                pass  # Process might be dead


def queryerLoop(agent: Queryer):
    agent.stop_event.clear()
    while not agent.stop_event.is_set():
        while not agent.inputQueue.empty():
            agent.input()
        if not agent.errorQueue.empty():
            err = agent.errorQueue.get().strip()
            print("Queryer error:", err)
            print("queryerLoop quit")
            agent.inputQueue.put('quit')
            agent.input()
            agent.stop_event.set()
            return None
