from Scheduler import Scheduler
from OS_Simulator import OS_Simulator
from PCPU import PCPU
from multiprocessing import Process, Pipe
import time
import signal
import sys
import os



def analyze_command(a):
	targets = []
	a+=","
	index = a.find(":")
	a = a[index+1:]
	a.replace(" ","")
	#print(a)
	index_now = a.find(",")
	while index_now!=-1:
		#split strings based on comma
		string_now = a[0:index_now]
		#print(string_now)
		hyphen = string_now.find("-")
		if hyphen!= -1:
			#print(hyphen)
			first = string_now[:hyphen]
			end = string_now[hyphen+1:]
			#print(first)
			#print(end)
			first = int(first)
			end = int(end)
			for i in range(first, end+1, 1):
				targets.append(i)
		else:
			targets.append(int(string_now))
		a = a[index_now+1:]
		index_now = a.find(",")
		#print(index_now)
	return targets





#main function
if __name__ == "__main__":
	#set up parameters
	pcpu_num = 8 #homogeneous pcpu for now
	load_ratio =0.5
	sum_af = 5
	simulation_time = 30 #in unit of seconds
	f = open("log/Initialization.log","w")
	old = sys.stdout
	sys.stdout = f
	cpu_affinity_list = []
	cpu_counter = 0
	#initialize pcpus
	try:
		pcpus = []
		for i in range(pcpu_num):
			pcpu_now = PCPU()
			pcpus.append(pcpu_now)

		#initialize Scheduler
		scheduler = Scheduler(sum_af, pcpus)

		largest_aaf = scheduler.largest_aaf
		#initialize OS_Simulator
		load = sum_af*load_ratio
		simulator = OS_Simulator(load, largest_aaf)


		#retrieve the cpu_affinity_list
		r = os.popen("taskset -c -p "+str(os.getpid()))
		cpu_affinity_list = analyze_command(r.read())
		#print(cpu_affinity_list)

	except Exception as err:
		print(err)
		sys.stdout = old
		f.close()
	else:

		print("Initialization done.")


		terminate_pipe_read, terminate_pipe_send = Pipe()
		p = Process(target = scheduler.run_system, args = (simulator, 1, "best_fit", terminate_pipe_read, cpu_counter, cpu_affinity_list))
		p.start()
		print("Game starts")
		time.sleep(simulation_time)
		terminate_pipe_send.send("End!!")
		[failed_jobs, total_jobs] = terminate_pipe_send.recv()
		result_file = open("log/result.log", "a")
		#print("Jobs received: "+str(total_jobs)+" and job failed: "+str(failed_jobs))
		result_file.write(str(failed_jobs)+', '+str(total_jobs)+'\n')
		result_file.flush()
		result_file.close()

		sys.stdout = old
		f.close()

