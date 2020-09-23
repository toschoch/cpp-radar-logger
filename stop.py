#!/usr/bin/python3
import subprocess
from pijuice import PiJuice

p = PiJuice(1, 0x14)

subprocess.run("tmux -S /tmp/radartmux kill-session -t radar",shell=True)
p.status.SetLedBlink('D2',0, [0,0,0], 100, [0,0,0], 100)
