import argparse
import logging
import os
import sys
import time
import BFS

import numpy as np
import pandas
from bt_interface import BTInterface
# from maze import Action, Maze
from score import ScoreboardServer, ScoreboardFake

logging.basicConfig(
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s", level=logging.INFO
)

log = logging.getLogger(__name__)

TEAM_NAME = "Please_Enter_Your_Name"
HM10_NAME = "HM10_t8"
SERVER_URL = "http://carcar.ntuee.org/scoreboard"
# MAZE_FILE = "data/small_maze.csv"
BT_PORT = "COM5"
HM10_NAME = "HM10_TM8"

TIME_CONSTRAINT = 65.0

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", help="0: treasure-hunting, 1: self-testing", type=str)
    # parser.add_argument("--maze-file", default=MAZE_FILE, help="Maze file", type=str)
    parser.add_argument("--bt-port", default=BT_PORT, help="Bluetooth port", type=str)
    parser.add_argument(
        "--team-name", default=TEAM_NAME, help="Your team name", type=str
    )
    parser.add_argument("--server-url", default=SERVER_URL, help="Server URL", type=str)
    return parser.parse_args()


def main(mode: int, bt_port: str, team_name: str, server_url: str):
    # maze = Maze(maze_file)

    ### Bluetooth connection haven't been implemented yet, we will update ASAP ###
    bt_interface = BTInterface(port=bt_port, hm10_name=HM10_NAME, queryFunc=BFS.getActions())
    if mode == "0":
        bt_interface.connect()
        while not bt_interface.isReady():
            print("Waiting for ready signal...")
            time.sleep(0.5)
            msg = bt_interface.bridge.listen()
            if len(msg) > 0:
                bt_interface.agent.on_message(msg)

    _ = input("System is ready. Press Enter to continue...")

    point = None
    if mode == "0":
        point = ScoreboardServer(team_name, server_url)
    else:
        point = ScoreboardFake(team_name, "data/fakeUID.csv") # for local testing

    print("Start Game")
    if mode == "0":
        bt_interface.activate()

    if mode == "0":
        log.info("Mode 0: For treasure-hunting")
        while True:
            rfid = bt_interface.getRFID()
            if rfid != None:
                point.add_UID(rfid)
                print("Current score:", point.get_current_score())

    elif mode == "1":
        log.info("Mode 1: Self-testing mode.")
        def getRFIDgetter():
            rfids = ['C3AD6B29', 'D071E11B']
            index = 0
            def getRFID():
                nonlocal rfids, index
                if index < len(rfids):
                    response = rfids[index]
                    index += 1
                    return response
                else:
                    return None
            return getRFID

        getRFID = getRFIDgetter()
        while True:
            time.sleep(3)
            rfid = getRFID()
            if rfid != None:
                point.add_UID(rfid)
                print("Current score:", point.get_current_score())

    else:
        log.error("Invalid mode:"+mode)
        sys.exit(1)


if __name__ == "__main__":
    args = parse_args()
    main(**vars(args))
