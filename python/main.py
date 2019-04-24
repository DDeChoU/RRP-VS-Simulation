from Scheduler import Scheduler
from OS_Simulator import OS_Simulator
from PCPU import PCPU
from multiprocessing import Process, Pipe
import time
import signal
import sys
#main function
if __name__ == "__main__":
	#set up parameters
	pcpu_num = 8 #homogeneous pcpu for now
	load_ratio =0.5
	sum_af = 5
	simulation_time = 100 #in unit of seconds
	f = open("Initialization.log","w")
	old = sys.stdout
	sys.stdout = f
	#initialize pcpus
	try:
		pcpus = []
		for i in range(pcpu_num):
			pcpu_now = PCPU()
			pcpus.append(pcpu_now)
		#initialize OS_Simulator
		load = sum_af*load_ratio
		simulator = OS_Simulator(load)

		#initialize Scheduler
		scheduler = Scheduler(sum_af, pcpus)
	except Exception as err:
		print(err)
		sys.stdout = old
		f.close()
	else:
		sys.stdout = old
		f.close()
		terminate_pipe_read, terminate_pipe_send = Pipe()
		p = Process(target = scheduler.run_system, args = (simulator, 1, "best_fit", terminate_pipe_read))
		p.start()
		time.sleep(simulation_time)
		terminate_pipe_send.send("End!!")
		[failed_jobs, total_jobs] = terminate_pipe_send.recv()
		result_file = open("result.log", "a")
		#print("Jobs received: "+str(total_jobs)+" and job failed: "+str(failed_jobs))
		result_file.write(str(failed_jobs)+', '+str(total_jobs)+'\n')
		result_file.flush()
		result_file.close()



