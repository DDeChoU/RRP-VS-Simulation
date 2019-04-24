from Scheduler import Scheduler
from OS_Simulator import OS_Simulator
from PCPU import PCPU
from multiprocessing import Process, Pipe
import time
import signal
#main function
if __name__ == "__main__":
	#set up parameters
	pcpu_num = 8 #homogeneous pcpu for now
	load_ratio =2
	sum_af = 5
	simulation_time = 100 #in unit of seconds
	#initialize pcpus
	pcpus = []
	for i in range(pcpu_num):
		pcpu_now = PCPU()
		pcpus.append(pcpu_now)
	#initialize OS_Simulator
	load = sum_af*load_ratio
	simulator = OS_Simulator(load)

	#initialize Scheduler
	scheduler = Scheduler(sum_af, pcpus)
	terminate_pipe_read, terminate_pipe_send = Pipe()
	p = Process(target = scheduler.run_system, args = (simulator, 1, "best_fit", terminate_pipe_read))
	p.start()
	time.sleep(simulation_time)
	terminate_pipe_send.send("End!!")
	[failed_jobs, total_jobs] = terminate_pipe_send.recv()
	print("Jobs received: "+str(total_jobs)+" and job failed: "+str(failed_jobs))



