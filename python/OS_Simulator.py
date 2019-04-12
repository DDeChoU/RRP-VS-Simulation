from Task import Task
from Task import Job
import datetime
import random
from multiprocessing import Process, Pipe
import signal
import os
import time
import math
class OS_Simulator:
	TIME_SLICE_LEN = 10
	MAX_WCET=1000
	def __init__(self, load, com_pid, core_rank,has_sporadic = False, per_ratio = 0.5):
		'''

		Args:
			load:			type: double; The sum of densities of all tasks
			task_list:		type: Task[]; The array of tasks running
			start_time: 	type: datetime; The booting time of the OS_simulator
			total_tasks:	type: int; The total number of tasks
			total_jobs: 	type: int; The total number of jobs
			success_jobs:	type: int; The number of jobs successfully accomplished in time
			failed_tasks:	type: int; The number of tasks failed.
			job_pipe_send:	type: Pipe; A pipe that sends the info of the OS to others.
			job_pipe_read:	type: Pipe; A pipe that receives the info of others in OS
			info_pipe_read: type: Pipe; A pipe that receives the info from the scheduler
			info_pipe_send:	type: Pipe; A pipe that sends the info of the OS to others.
			target_pid:		type: pid;	The pid of the target process that receives message.
			core_rank:		type: int; Set up which core the process should bind to
		'''

		#generate the task set here based on the load
		self.load = load
		self.target_pid = com_pid
		self.task_list = self.generate_tasks(load, has_sporadic, per_ratio)
		self.total_tasks = len(self.task_list)
		self.total_jobs = 0
		self.success_jobs = 0
		#for t in self.task_list:
		#	print(t.task_info())
		#set the start_time here
		self.start_time = datetime.datetime.now()
		#construct a pipe here and save the pipe ends in the job_pipe
		self.job_pipe_read, self.job_pipe_send = Pipe(duplex=False)
		self.info_pipe_read, self.info_pipe_send = Pipe(duplex=False)
		self.core_rank = core_rank
		#start the non-stop process that generate jobs

		p = Process(target=self.generate_jobs, args=(self.start_time,))
		p.start()
		self.gen_pid = p.pid	


	def gen_kato_utilizations(self, target_val, min_val, max_val):
		'''
			This function is modified from the function gen_kato_utilizations in class simso.generator.task_generator.
		'''
		vals = []
		total_val = 0
		# Classic UUniFast algorithm:
		while total_val < target_val:
			val = random.uniform(min_val, max_val)
			if val + total_val> target_val:
 				val = target_val - total_val
			total_val += val
			vals.append(val)
		return vals

	def generate_tasks(self, target_load, has_sporadic = False, per_ratio = 0.5):
		'''
		 
		 Args:
			  - target_load: Total utilization to reach.
			  - has_sporadic: whether tasks has_sporadic or not
			  - per_ratio: the ratio of periodic tasks.
		'''
		task_set =[]
		#set the min and max accordingly
		utils = self.gen_kato_utilizations(target_load,0, 1)#generate utilizations based on the number of tasks generated
		num = len(utils)
		for i in range(num):
			util_now = utils[i]
			wcet = random.randint(2, OS_Simulator.MAX_WCET)*OS_Simulator.TIME_SLICE_LEN
			period = math.ceil(wcet/util_now*OS_Simulator.TIME_SLICE_LEN)
			deadline = period
			#arrival = -math.log(1.0 - random.random())
			arrival = random.randint(0, 3000*OS_Simulator.TIME_SLICE_LEN)
			is_periodic= True
			if has_sporadic:
				dice = random.random()
				if dice>= per_ratio:
					is_periodic = False
			task_now = Task(wcet, period, deadline, arrival, is_periodic)
			task_set.append(task_now)
		task_set = sorted(task_set, key=lambda x: x.arr_time)
		return task_set

	def generate_jobs(self, start_time):
		os.system("taskset -p -c " +str(self.core_rank% os.cpu_count())+" "+str(os.getpid()))
		count+=1
		arrived_task_list = []
		phases = []#use the phases array to record which job of the task is to come next
		counter = 0
		all_arrived = False
		while True:
			time_now = datetime.datetime.now()
			interval = (time_now - start_time).total_seconds()*1000
			#if interval>=50000:
			#	break
			#add newly arrived tasks

			#print(self.task_list[counter].arr_time/1000.0)
			if not all_arrived:
				arr_time_now = self.start_time+datetime.timedelta(milliseconds = self.task_list[counter].arr_time)
				#print(arr_time_now)
				#print(self.start_time)
				while arr_time_now<=time_now:#(time_now - self.task_list[counter].arr_time).total_second()>=0
					print("A task has arrived")
					print(counter)
					arrived_task_list.append(self.task_list[counter])
					counter += 1
					phases.append(0)
					if counter>= len(self.task_list):
						all_arrived = True
						print("All tasks' first instances have arrived.")
						break
					arr_time_now = self.start_time+datetime.timedelta(self.task_list[counter].arr_time)


			#maintain tasks that already arrived
			for i in range(len(arrived_task_list)):
				if interval>= arrived_task_list[i].arr_time + phases[i]*arrived_task_list[i].period:
					if arrived_task_list[i].is_periodic == False:
						dice = random.random.ranind(0,5)#only 20% of sporadic arrives at the first minimal interval
						if dice > 0:
							break
					arb_ddl = time_now+datetime.timedelta(milliseconds=arrived_task_list[i].ddl)
					phases[i]+= 1
					j = Job(arrived_task_list[i].WCET, arb_ddl, arrived_task_list[i].task_id)
					#send it through the pipe
					self.job_pipe_send.send(j)
					#os.system("ps -o pid,psr,comm -p "+str(os.getpid()))
			#receive info from the pipe 

	def get_job_pipe_read(self):
		return self.job_pipe_read
	def get_info_pipe_send(self):
		return self.info_pipe_send


		#analyze the info and decides whether a job is accomplished on time or not
	def get_pid(self):
		return self.gen_pid



#code for the test of OS_Simulator
core_count = 0
a = OS_Simulator(10, os.getpid(), core_count)
core_count += 1
pid = a.get_pid()


job_receiver= a.get_job_pipe_read()
while True:
	if job_receiver.poll():
		job_now = job_receiver.recv()
		print(job_now.job_info())

