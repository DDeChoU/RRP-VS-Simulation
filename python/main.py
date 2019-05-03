from Scheduler import Scheduler
from OS_Simulator import OS_Simulator
from PCPU import PCPU
from multiprocessing import Process, Pipe
import time
import signal
import sys
import os
from Generation import Generation
import copy
import logging

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



def run_test(partition_list, task_list, pcpu_num, simulation_time, policy_name):
	#set up parameters
	pcpu_num = 8 #homogeneous pcpu for now
	load_ratio =0.5
	sum_af = 5
	simulation_time = 30 #in unit of seconds
	#f = open("log/Initialization.log","w")
	#old = sys.stdout
	#sys.stdout = f
	cpu_affinity_list = []
	cpu_counter = 0
	#initialize pcpus
	try:
		pcpus = []
		for i in range(pcpu_num):
			pcpu_now = PCPU()
			pcpus.append(pcpu_now)

		#initialize Scheduler
		scheduler = Scheduler(partition_list, pcpus)


		#largest_aaf = scheduler.largest_aaf
		#initialize OS_Simulator
		load = sum_af*load_ratio
		simulator = OS_Simulator(task_list)


		#retrieve the cpu_affinity_list
		r = os.popen("taskset -c -p "+str(os.getpid()))
		cpu_affinity_list = analyze_command(r.read())
		# for running locally
		#cpu_affinity_list = [1,2]
		#print(cpu_affinity_list)

	except Exception as err:
		#print(err)
		#sys.stdout = old
		#f.close()
		#logging.exception("Error here:")
		return None, None


	terminate_pipe_read, terminate_pipe_send = Pipe()
	p = Process(target = scheduler.run_system, args = (simulator, 1, policy_name, terminate_pipe_read, cpu_counter, cpu_affinity_list))
	p.start()
	time.sleep(simulation_time)
	terminate_pipe_send.send("End!!")
	[failed_jobs, total_jobs] = terminate_pipe_send.recv()
	return failed_jobs, total_jobs

	'''result_file = open("log/result.log", "a")
	#print("Jobs received: "+str(total_jobs)+" and job failed: "+str(failed_jobs))
	result_file.write(str(failed_jobs)+', '+str(total_jobs)+'\n')
	result_file.flush()
	result_file.close()

	sys.stdout = old
	f.close()'''



#main function
if __name__ == "__main__":


	if(len(sys.argv)<5):
		exit(0)
	repeat_times = 200#fixed

	
	sum_af = int(sys.argv[1])
	pcpu_num = int(sys.argv[2])
	simulation_time = int(sys.argv[3])
	load_ratio = float(sys.argv[4])
	file_name = "log/main_"+str(sum_af)+"_"+str(pcpu_num)+"_"+str(simulation_time)+"_"+str(load_ratio)+".log"

	f = open(file_name, "w")
	old = sys.stdout
	#sys.stdout = f

	policies = ["best_fit", "first_fit", "worst_fit","almost_worst_fit"]# ,
	#policies = ["worst_fit"]
	schedulability = []
	total_jobs = []
	failed_jobs = []
	ratio = []
	g = Generation()
	#initialize counters
	for i in range(len(policies)):
		schedulability.append(0)
		total_jobs.append(0)
		failed_jobs.append(0)
		ratio.append(0)
	for i in range(repeat_times):
		partition_list = g.generate_partitions(sum_af)
		largest_aaf = max(partition_list, key = lambda x:x.af).af
		task_list = g.generate_tasks(load_ratio*sum_af, False, 0.5, 0, largest_aaf)
		for j in range(len(policies)):
			temp_partition_list = copy.deepcopy(partition_list)
			temp_task_list = copy.deepcopy(task_list)
			temp_fail, temp_total = run_test(temp_partition_list, temp_task_list, pcpu_num, simulation_time, policies[j])
			while temp_fail is None or temp_total is None:
				#print("not schedulable")
				partition_list = g.generate_partitions(sum_af)
				largest_aaf = max(partition_list, key = lambda x:x.af).af
				task_list = g.generate_tasks(load_ratio*sum_af, False, 0.5, 0, largest_aaf)
				temp_partition_list = copy.deepcopy(partition_list)
				temp_task_list = copy.deepcopy(task_list)
				temp_fail, temp_total = run_test(temp_partition_list, temp_task_list, pcpu_num, simulation_time, policies[j])
			if temp_fail == 0:
				schedulability[j] += 1
			total_jobs[j] += temp_total
			failed_jobs[j] += temp_fail
			ratio[j] += (temp_total - temp_fail)/temp_total
			print("This round: "+str(temp_total)+","+str(temp_fail))
			print(str(i)+", "+str(j)+": "+str(schedulability[j])+", "+str(ratio[j]))
	#change the file name here
	file_name = "result"+str(sum_af)+"_"+str(pcpu_num)+"_"+str(simulation_time)+"_"+str(load_ratio)+".txt"
	result_file = open(file_name, "w")
	for j in range(len(policies)):

		ratio[j]/= repeat_times
		schedulability[j]/=repeat_times
		result_file.write(policies[j]+": "+"("+str(load_ratio)+", "+str(schedulability[j])+");("\
			+str(load_ratio)+","+str(completion_ratio)+")\n")
	result_file.flush()
	result_file.close()
	sys.stdout = old
	f.close()



