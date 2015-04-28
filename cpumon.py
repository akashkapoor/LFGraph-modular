import time
import string
import sys
import commands
import psutil
import os

def get_cpumem(pid):
    d = [i for i in commands.getoutput("ps aux").split("\n")
        if i.split()[1] == str(pid)]
    return (float(d[0].split()[2]), float(d[0].split()[3])) if d else None

def get_util(pid):
    print("%CPU\t%MEM")
    cpu = []
    mem = []
    try:
        while True:
            x = get_cpumem(pid)
            if not x:
                print("no such process")
                exit(1)
            #print("%.2f\t%.2f" % x)
            if x[0] > 0:
            	cpu.append(x[0])
            if x[1] > 1:
            	mem.append(x[1])
            time.sleep(0.5)
    except KeyboardInterrupt:
        print "CPU: max-" + str(max(cpu)) + " , avg-" + str(sum(cpu)/len(cpu))
        print "MEM: " + str(max(mem))
        exit(0)

efficient_args=['main', 'default.cfg']

pid = os.fork()
if pid:
	#parent
	get_util(pid)
else:
	#child
	os.execl("main", *efficient_args)
