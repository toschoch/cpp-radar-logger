#!/usr/bin/python3
from pijuice import PiJuice
import subprocess
import time

p = PiJuice(1, 0x14)

subprocess.run("tmux -S /tmp/radartmux new -d -s radar '/home/pi/radar/RadarReader'", shell = True)

print('started tmux! wait...')
p.status.SetLedBlink('D2', 255, [0,50,0], 400, [0, 0, 0], 100)
time.sleep(3)
try:
    output = subprocess.check_output("tmux -S /tmp/radartmux list-sessions",
                                     stderr=subprocess.STDOUT,
                                     shell = True)

    print(output.decode('utf-8').strip())

except subprocess.CalledProcessError as err:
    output = err.output.decode('utf-8').strip()
    print(output)
    if not output.startswith('radar: '):
        p.status.SetLedBlink('D2', 1, [100,0,0], 400, [0, 0, 0], 100)
        raise Exception("could not start radar acquisition! ({})".format(output))


print('successfully started!')
p.status.SetLedBlink('D2', 255, [0,0,100], 1000, [0, 0, 0], 300)

