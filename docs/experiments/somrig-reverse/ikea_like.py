
def main():
    observations = tuple(parse_observations(PLAN_RAW))
    for actions, expected_events in observations:
        actual_events_from_presses = translate_presses_to_zigbee_events(actions)
        actual_events_from_callbacks = translate_callbacks_to_zigbee_events(translate_presses_to_callbacks(actions))
        expected_events = ' '.join(expected_events)
        actual_events_from_presses = ' '.join(actual_events_from_presses)
        actual_events_from_callbacks = ' '.join(actual_events_from_callbacks)
        print(f"{actions:<21} {expected_events}")
        if expected_events != actual_events_from_presses:
            print(f"  mismatch   presses: {actual_events_from_presses}")
        if expected_events != actual_events_from_callbacks:
            print(f"  mismatch callbacks: {actual_events_from_callbacks}")


"""
State machine that makes more sense to me
"""
def translate_presses_to_zigbee_events(presses: str):
    for press in presses:
        if press not in "ls":
            raise Exception(f"invalid action {a}")

    events = []
    state_pending = False
    for press in presses:
        if state_pending and press == "s":
            events += ["dp"]
            state_pending = False
            continue
        if state_pending:
            events += ["sr"]
        events += ["ip"]
        if press == "l":
            events += ["lp", "lr"]
            state_pending = False
        else:
            state_pending = True
    if state_pending:
        events += ["sr"]

    return events


def translate_callbacks_to_zigbee_events(callbacks: list[str]):

    events = []
    state_pending = 0
    state_long = 0
    state_press_count = 0
    for cb in callbacks:
        if cb == "ep":
            if state_pending == 0:
                events += ["ip"]
            state_long = 0
            state_pending = (state_pending + 1) & 3
        elif cb == "er":
            if state_long == 1:
                events += ["lr"]
                state_pending = 0
            if state_pending == 2:
                events += ["dp"]
                state_pending = 0
        elif cb == "kp":
            if state_pending == 2:
                events += ["sr", "ip"]
                state_pending = 0
            events += ["lp"]
            state_long = 1
        elif cb == "kr":
            if state_pending == 1 and state_long == 0:
                events += ["sr"]
            state_pending = 0
            state_long = 0
        else:
            raise Exception(f"unknown callback {cb!r}")

    return events

def translate_presses_to_callbacks(presses):
    """
    p = edge pressed
    r = edge released
    k = keep last
    """
    if not presses:
        return []
    output = []
    for p in presses:
        if p == "s":
            output += ["ep", "er"]
        if p == "l":
            output += ["ep", "kp", "er"]
    output += ["kr"]
    return output


PLAN_RAW = """
s:         ip sr
sl:        ip sr ip lp lr
sls:       ip sr ip lp lr ip sr
slsl:      ip sr ip lp lr ip sr ip lp lr
slsls:     ip sr ip lp lr ip sr ip lp lr ip sr
slslsl:    ip sr ip lp lr ip sr ip lp lr ip sr ip lp lr
slslsls:   ip sr ip lp lr ip sr ip lp lr ip sr ip lp lr ip sr

s:         ip sr
ss:        ip dp
sss:       ip dp ip sr
ssss:      ip dp ip dp
l:         ip lp lr
ll:        ip lp lr ip lp lr
sl:        ip sr ip lp lr
ls:        ip lp lr ip sr
"""


def parse_observations(obs):
    for line in obs.splitlines():
        line = line.strip()
        if line:
            k, v = line.split(":")
            k = k.strip()
            v = tuple(v.strip().split(" "))
            yield k, v


if __name__ == "__main__":
    main()
