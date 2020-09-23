#!/usr/bin/python3
from pijuice import PiJuice
import subprocess

p = PiJuice(1, 0x14)

subprocess.run("tmux -S /tmp/radartmux new -d -s radar '/home/pi/radar/RadarReader'",shell=True)

p.status.SetLedBlink('D2', 255, [0,50,0], 1000, [100, 0, 0], 500)

