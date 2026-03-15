
def main():
    observations = tuple(parse_observations(OBSERVATIONS_RAW))
    for actions, observed_events in observations:
        predicted_events = predict_fsm_medium(actions)
        if tuple(observed_events) == tuple(predicted_events):
            print(f"{actions:<20} ok       {' '.join(observed_events)}")
        else:
            print(f"{actions:<20} mismatch")
            print(f"{' ' * 15}   prediction: {' '.join(predicted_events)}")
            print(f"{' ' * 15}  observation: {' '.join(observed_events)}")


"""
State machine that intends to follow ikeas implementation as closely as possible.
It is getting complicated yet there are still cases that are not covered.
I came to the conclusion it is not worth mimicing their behavior fully because
it seems pretty random in some cases.
"""
def predict_fsm_medium(actions):
    events = []

    state_suppress_dp = False
    state_suppress_ip = False
    state_suppress_l = False
    state_schedule_sr = False
    state_initial = True
    for a in actions:
        next_state_initial = False
        if state_initial:
            if not state_suppress_ip:
                events += ['ip']
                state_suppress_ip = True
        if a == 's':
            state_suppress_l = True
            if state_initial:
                state_schedule_sr = True
            else:
                if not state_suppress_dp:
                    events += ['dp']
                    state_suppress_ip = False
                    state_suppress_l = False
                    state_suppress_dp = False
                    state_schedule_sr = False
                next_state_initial = True
        elif a == 'l':
            state_schedule_sr = False
            if state_initial:
                if not state_suppress_l:
                    events += ['lp', 'lr']
            else:
                state_suppress_dp = True
        else:
            raise Exception(f"unknown actor: {a!r}")
        state_initial = next_state_initial
    if state_schedule_sr:
        events += ['sr']

    return events

"""
actions reported by z2m when pressing ikea somrig button with the pattern in `key`

key: s is short press, l is long press
value:
  - ip: initial_press
  - sr: short_release
  - lp: long_press
  - lr_ long_release
  - dp: double_press
"""
OBSERVATIONS_RAW = """
s:         ip sr
ss:        ip dp
sss:       ip dp ip sr
ssss:      ip dp ip dp
sssss:     ip dp ip dp ip sr
l:         ip lp lr
ll:        ip lp lr
lll:       ip lp lr
llll:      ip lp lr
ls:        ip lp lr dp
lsl:       ip lp lr dp ip lp lr
ssl:       ip dp ip lp lr
sll:       ip
lss:       ip lp lr dp ip sr
lls:       ip lp lr
sssl:      ip dp ip
slll:      ip
llls:      ip lp lr 
lsss:      ip lp lr dp ip dp
ssssl:     ip dp ip dp ip lp lr
sslll:     ip dp ip lp lr
lllss:     ip lp lr sr
llsss:     ip lp lr sr
lssss:     ip lp lr dp ip dp ip sr
lslsl:     ip lp lr dp ip lp lr dp ip lp lr

sl:            ip
sls:           ip
slsl:          ip
slsls:         ip sr
slslsl:        ip
slslsls:       ip
slslslsl:      ip
slslslsls:     ip
slslslslsl:    ip
slslslslsls:   ip
slslslslslsl:  ip ip sr
slslslslslsls: ip ip dp

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
