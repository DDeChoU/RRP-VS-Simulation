import datetime
import threading
import time
from multiprocessing import Process
import signal
import os
import time

def receive_signal(signum, stack):
    print('Received:', signum)





def send_signals(pid):
	while True:
		os.kill(pid, signal.SIGUSR1)
		time.sleep(3)

signal.signal(signal.SIGUSR2, receive_signal)
signal.signal(signal.SIGUSR1, receive_signal)
p = Process(target=send_signals,args=(os.getpid(),))
p.start()
print(p.pid)
while True:
	print('sending...')
	#os.kill(p.pid, signal.SIGUSR2)
	time.sleep(3)
#signal test


'''
#Timing test
def get_time_stamp():
	return datetime.datetime.now()
def get_time_difference(a, b):
	c = b-a
	return c.total_seconds()*1000


def test_waiting(wait_time):
	wait_time = int(wait_time)
	execution = threading.Event()
	start = get_time_stamp()
	while True:
		end = get_time_stamp()
		internal = get_time_difference(start, end)
		if internal> wait_time:
			break


	
	print(get_time_difference(start, end))

for i in range(32):
	p = Process(target=test_waiting, args=(200,))
	p.start()
'''


