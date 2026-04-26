import csv
from collections import deque

# ── SETTINGS ──────────────────────────────────────────────────────────────
# ↓ Change these to match your setup
FILE_PATH   = r"./medium_maze.csv"
START_NODE  = 1    # FIX 1: removed trailing space — map IDs are always stripped
# INIT_FACING = "N"   # initial facing: 'N', 'S', 'E', or 'W'

# Treasure nodes: set to None to auto-detect dead-ends (degree-1 nodes),
# or list them manually, e.g. TREASURE_NODES = [4, 6, 9, 10]
# TREASURE_NODES = [7,9,10,12]


# ── DIRECTION TABLE ────────────────────────────────────────────────────────
# Converts a relative command (f/l/r/b) to an absolute direction (N/S/E/W)
# depending on the robot's current facing.
#
#         facing N    facing S    facing E    facing W
#   f  →    N           S           E           W
#   l  →    W           E           N           S
#   r  →    E           W           S           N
#   b  →    S           N           W           E
#
REL_TO_ABS = {
    'N': {'f': 'N', 'l': 'W', 'r': 'E', 'b': 'S'},
    'S': {'f': 'S', 'l': 'E', 'r': 'W', 'b': 'N'},
    'E': {'f': 'E', 'l': 'N', 'r': 'S', 'b': 'W'},
    'W': {'f': 'W', 'l': 'S', 'r': 'N', 'b': 'E'},
}


# ── DIRECTION SENSOR ──────────────────────────────────────────────────────
def scan_directions(node, facing, game_map, treasure_nodes):
    """
    At the current node, check all 4 relative directions (F/L/R/B).
    For each direction:
      - If the neighbour exists AND is a treasure node → label '1'
      - Otherwise (wall or non-treasure neighbour)     → label '0'

    Returns a string like 'F1  L0  R0  B1'
    """
    parts = []
    for rel, label in [('f', 'F'), ('l', 'L'), ('r', 'R'), ('b', 'B')]:
        abs_dir      = REL_TO_ABS[facing][rel]
        nb           = game_map[node][abs_dir]
        is_treasure  = (nb > 0) and (nb in treasure_nodes)
        parts.append(f"{label}{'1' if is_treasure else '0'}")
    return '  '.join(parts)


# ── LOAD MAP ───────────────────────────────────────────────────────────────
def load_map_from_csv(file_path):
    """
    Read CSV and build map structure.
    Each node stores absolute neighbours: N / S / W / E.
    Treasure nodes are detected automatically as dead-end nodes (degree == 1),
    or can be listed manually in TREASURE_NODES below.
    """
    game_map = {}

    def safe_int(value):
        if not value or value.strip() == "":
            return 0
        try:
            return int(float(value))
        except ValueError:
            return 0

    try:
        with open(file_path, mode='r', encoding='utf-8-sig') as f:
            reader = csv.DictReader(f)
            reader.fieldnames = [name.strip() for name in reader.fieldnames]
            for row in reader:
                room_id = row.get('index')
                if room_id is None:
                    print("錯誤：找不到 'index' 欄位，請檢查 CSV 標題")
                    return None
                room_id = int(float(str(room_id).strip()))   # always stripped and convert to int
                game_map[room_id] = {
                    'N': safe_int(row.get('North')),
                    'S': safe_int(row.get('South')),
                    'W': safe_int(row.get('West')),
                    'E': safe_int(row.get('East')),
                }
        return game_map
    except Exception as e:
        print(f"讀取錯誤: {e}")
        return None


# ── Find all absolute directions available from a node ─────────────────────
def find_valid_directions(node, game_map):
    result = []
    for abs_dir in ['N', 'S', 'W', 'E']:
        if game_map[node][abs_dir] > 0:
            result.append(abs_dir)
    return result


# ── BFS: find shortest facing-aware path from one node to another ──────────
def bfs_to_target(start_node, start_facing, target_node, game_map):
    """
    BFS over states (node, facing).
    Returns (step_count, arrival_facing, command_list).
    arrival_facing is the facing when the robot reaches target_node,
    which becomes the new init_facing for the next leg.
    """
    if start_node == target_node:
        return 0, start_facing, []

    q = deque()
    q.append((start_node, start_facing, []))
    visited = {(start_node, start_facing)}

    while q:
        node, facing, cmds = q.popleft()
        for rel in ['f', 'l', 'r', 'b']:
            abs_dir = REL_TO_ABS[facing][rel]
            nb = game_map[node][abs_dir]
            if nb <= 0:
                continue
            ns = (nb, abs_dir)
            if ns in visited:
                continue
            visited.add(ns)
            new_cmds = cmds + [rel]
            if nb == target_node:
                return len(new_cmds), abs_dir, new_cmds
            q.append((nb, abs_dir, new_cmds))

    return float('inf'), None, None   # unreachable


# ── Compute all scores at given treasure nodes, using Manhattan distance ──
def compute_all_scores(start_node, treasure_nodes, game_map):
    scores = {}
    # Used to record traversed nodes, without traversing the set, so there's no randomness issue
    traversed = set()
    # (node, x, y)
    q = deque()
    q.append((start_node, 0, 0))
    delta = {'N': (0, 1), 'S': (0, -1), 'W': (-1, 0), 'E': (1, 0)}
    while q:
        node, x, y = q.popleft()
        traversed.add(node)
        if node in treasure_nodes:
            scores[node] = abs(x) + abs(y)
        for abs_dir in ['N', 'S', 'W', 'E']:
            next = game_map[node][abs_dir]
            if next <= 0:
                continue
            if next in traversed:
                continue
            q.append((next, x + delta[abs_dir][0], y + delta[abs_dir][1]))
    return scores


# ── GREEDY MULTI-TREASURE PLANNER ─────────────────────────────────────────
def greedy_collect_all(start_node, start_facing, treasure_nodes, scores, game_map):
    """
    At each decision point choose the unvisited treasure with the highest
    score / distance_to_reach ratio.

      score    = fixed value computed from start (Manhattan distance proxy)
      distance = BFS steps from current position with current facing

    At every node the robot visits, prints a sensor line showing whether
    each relative direction (F/L/R/B) leads to a treasure (1) or not (0).
    e.g.  Node 2 (facing N) | F0  L0  R0  B0  | [F]

    Returns
    -------
    visit_order  : list of treasure node ids in visit order
    full_cmds    : flat list of relative commands for the entire journey
    total_score  : sum of scores collected
    """
    treasure_set   = dict(zip(treasure_nodes, [True for _ in range(len(treasure_nodes))]))   # fast lookup
    current_node   = start_node
    current_facing = start_facing
    remaining      = dict(zip(treasure_nodes, [True for _ in range(len(treasure_nodes))]))
    full_cmds      = []
    visit_order    = []
    total_score    = 0
    step_total     = 0

    print("─" * 60)
    print(f"{'Step':<6} {'Node':<6} {'Facing':<8} {'Sensor (F / L / R / B)':<26} {'Cmd'}")
    print("─" * 60)

    # Print sensor at starting node
    sensor = scan_directions(current_node, current_facing, game_map, treasure_set)
    print(f"{'START':<6} {current_node:<6} {current_facing:<8} {sensor:<26}")

    while remaining:
        # ── Pick best next treasure ────────────────────────────────────────
        best_target  = None
        best_ratio   = -1
        best_cmds    = None
        best_facing  = None

        for t in remaining:
            dist, arr_facing, cmds = bfs_to_target(
                current_node, current_facing, t, game_map
            )
            ratio = scores[t] / dist if dist > 0 else float('inf')
            if ratio > best_ratio:
                best_ratio   = ratio
                best_target  = t
                best_cmds    = cmds
                best_facing  = arr_facing

        print(f"\n  → Heading to node {best_target} "
              f"(score={scores[best_target]}, ratio={best_ratio:.3f})")

        # ── Walk step by step, printing sensor at each new node ────────────
        for cmd in best_cmds:
            abs_dir        = REL_TO_ABS[current_facing][cmd]
            next_node      = game_map[current_node][abs_dir]
            step_total    += 1
            current_node   = next_node
            current_facing = abs_dir

            is_treasure    = current_node in treasure_set
            cmd_token      = f"{cmd.upper()}{'1' if is_treasure else '0'}"
            full_cmds.append(cmd_token)

            sensor = scan_directions(current_node, current_facing, game_map, treasure_set)
            marker = " ★ TREASURE" if is_treasure else ""
            print(f"{step_total:<6} {current_node:<6} {current_facing:<8} {sensor:<26} {cmd_token}{marker}")

        # ── Collect treasure ───────────────────────────────────────────────
        total_score   += scores[best_target]
        visit_order.append(best_target)
        del remaining[best_target]
        # current_node / current_facing already updated in the walk loop

    print("─" * 60)
    return visit_order, full_cmds, total_score

# ── MAIN ───────────────────────────────────────────────────────────────────
def getActions():
    game_map = load_map_from_csv(FILE_PATH)
    if not game_map:
        return

    init_facing = find_valid_directions(START_NODE, game_map)
    if len(init_facing) != 1:
        print("The start node is not a dead-end")
        return
    init_facing = init_facing[0]

    # FIX 2: strip START_NODE before comparison so dead-end detection is correct
    start_node = START_NODE

    # Detect treasure nodes
    treasure_nodes = [
        n for n, conns in game_map.items()
        if sum(1 for v in conns.values() if v != 0) == 1
        and n != start_node   # use stripped start_node for comparison
    ]
    print(f"Auto-detected treasure nodes: {sorted(treasure_nodes, key=int)}")

    if not treasure_nodes:
        print("No treasure nodes found.")
        return

    # Compute fixed scores (distance from original start node)
    scores = compute_all_scores(start_node, treasure_nodes, game_map)
    print("\nScores (BFS steps from start node):")
    for n, s in sorted(scores.items(), key=lambda x: -x[1]):
        if s > 0:
            print(f"  Node {n}: {s}")

    # Run greedy planner
    print(f"\nPlanning route from node {start_node} (facing {init_facing})...\n")
    visit_order, full_cmds, total_score = greedy_collect_all(
        start_node, init_facing, treasure_nodes, scores, game_map
    )

    # Append end marker
    full_cmds.append('E0')

    print(f"\nVisit order : {visit_order}")
    print(f"Total score   : {total_score}")
    print(f"Commands      : {full_cmds}")

    filtered_cmds = []
    last_cmd = "F0"
    for cmd in full_cmds:
        if not (last_cmd[1] == '1' and cmd[0] == 'B'):
            filtered_cmds.append(cmd)
        last_cmd = cmd

    print(f"Filtered Cmds : {filtered_cmds}")

    # FIX 3: queryFunc now cycles through individual commands, not the whole list
    def getQueryFunc():
        index = 1   # use list so inner function can mutate it
        def queryFunc():
            nonlocal index
            cmd = filtered_cmds[min(index, len(filtered_cmds)-1)]
            index += 1
            return 't' + cmd
        return queryFunc
    return getQueryFunc()

if __name__ == "__main__":
    getActions()