from OS_Simulator import OS_Simulator
from multiprocessing import Process, Pipe, Queue
import random 
import datetime
from Partition import Partition
import sys

class Scheduler:

	def __init__(self, sum_af, pcpus):
		'''
		Args:

			sum_af: 					type: float; The total availability factor of partitions.
			pcpus:						type: PCPU dictionary; each item is a PCPU instance, 
											  while the key is the pcpu_id
			partitions:					type: dictionary; each item is a partition instance, 
											  while the key is the partition instance
			partition_pcpu_mapping:		type: dictionary; each item is a pcpu_id, 
											  the key is the partition_id
			partition_task:				type: dictionary; each item is a set with multiple task_id's, 
											  the key is the partition_id, used for partitioned scheduling
			partition_task_period:		type: dictionary; each item is a number indicating the smallest period of tasks,
											  the key is the partition_id, used for partitioned scheduling
			partition_task_density:		type: dictionary; each item is a number indicating the total densities of tasks
											  deployed on the partition, the key is the partition_id, used for partitionged scheduling.

		'''
		self.sum_af = sum_af
		self.pcpus = {}
		self.partitions = {}
		self.partition_pcpu_mapping = {}
		self.partition_task = {}
		self.partition_task_period = {}
		self.partition_task_density = {}
		self.process_list = []
		for i in range(len(pcpus)):
			self.pcpus[pcpus[i].pcpu_id] = pcpus[i]

		#counters used for statistical analysis
		self.total_jobs = 0
		self.failed_jobs = 0

		#invoke generate partitions here
		partition_list = self.generate_partitions(sum_af)
		for i in range(len(partition_list)):
			self.partitions[partition_list[i].partition_id] = partition_list[i]
			self.partition_task[partition_list[i].partition_id] = set()
			self.partition_task_period[partition_list[i].partition_id] = 1000
			self.partition_task_density[partition_list[i].partition_id] = 0
		#invoke mulZ_FFD here
		self.mulZ_FFD(partition_list)

	def mulZ_FFD(self, partition_list):
		pcpus_partitions = {}
		for (pcpu_id, _) in self.pcpus.items():
			pcpus_partitions[pcpu_id] = []
		for i in range(len(partition_list)):
			self.partitions[partition_list[i].partition_id] = partition_list[i]
		partition_list = sorted(partition_list, key=lambda x: x.af, reverse=True)
		for x in range(len(partition_list)):
			f = self.mulZ_FFD_Alloc(partition_list[x])
			if f is None:
				raise Exception("Error! Partitions not schedulable!!! Aborting!")
				return
			pcpus_partitions[f].append(partition_list[x])
			self.partition_pcpu_mapping[partition_list[x].partition_id] = f
		for (pcpu_id, pcpu_now) in self.pcpus.items():
			pcpu_now.set_partitions(pcpus_partitions[pcpu_id])
		return True

	def mulZ_FFD_Alloc(self, par):
		fixed_list=[3,4,5,7]
		approx_weights = [0 for x in range(4)]
		smallest = 5
		f=-1
		for x in range(4):
			num=self.z_approx(par.af, fixed_list[x])
			if num<smallest:
				f = fixed_list[x]
				smallest = num
			approx_weights[x] = smallest

		approx_weights = sorted(approx_weights)
		r = approx_weights[0]
		for (pcpu_id, pcpu_now) in self.pcpus.items():
			if pcpu_now.factor==0:
				pcpu_now.factor=f
				pcpu_now.rest = 1-r
				return pcpu_id
			elif pcpu_now.rest>=r:
				num=self.z_approx(par.af, pcpu_now.factor)
				pcpu_now.rest -= num
				return pcpu_id
		return None

	def z_approx(self, w, n):
		i=1
		j=0
		m=2
		largest = 1
		while True:
			if n-i/n >= w and (n-i != 1):
				largest = n-i/n
				i+= 1
			else:
				denom=n*m**j
				if 1/denom >= w:
					largest = 1/denom
					j+= 1
				else:
					return largest
		return -1
	def generate_partitions(self, target_af):
		'''
		 
		 Args:
			  - `target_af`: Total af of all partitions to reach.
		'''
		partition_set = []
		afs = self.gen_kato_utilizations(target_af,0.1, 1)#generate utilizations based on the number of tasks generated
		num = len(afs)
		for i in range(num):
			reg = random.randint(1,2)
			partition_now = Partition(afs[i], reg)#only generates regular partitions
			#print afs[i]
			partition_set.append(partition_now)
		return partition_set
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

	def run_system(self, simulator, mode, policy_name, terminate_pipe):
		'''
			Inputs:
			simulator:			type: OS_Simulator instance; The OS_Simulator initiated
			mode:				type: int; 1 for partitioned, 2 for global, 3 for mixed strategy
		'''
		#invoked when everything is set up

		#setup pipes: job receiver (from OS_Simulator), job sender(To pcpus), info receiver (from PCPUs), maintain a form for each pcpu
		if not callable(getattr(self, policy_name)):
			print("Invalid policy name given!")
		job_send_pipes = {}
		job_receive_pipe = Queue()
		info_pipes = {}

		self.process_list = []
		core_count = 0 #for now, we have to require all 28 cores in a node and core_count can work there.

		#run pcpus first
		for (pcpu_id, pcpu_now) in self.pcpus.items():
			job_pipe_now = Queue()
			job_send_pipes[pcpu_id] = job_pipe_now
			info_pipe_now = Queue()
			info_pipes[pcpu_id] = info_pipe_now
			tempP = Process(target = pcpu_now.run_pcpu, args = (info_pipe_now, job_pipe_now, core_count))
			tempP.start()
			self.process_list.append(tempP)
			core_count += 1

		#run OS_Simulator
		start_time = datetime.datetime.now()
		#print(job_receive_pipe)
		tempP = Process(target = simulator.generate_jobs, args = (start_time, job_receive_pipe, core_count))
		tempP.start()
		self.process_list.append(tempP)
		core_count += 1


		#run the scheduler, bind it to a seperate core 
		f = open("scheduler.log","w")
		old = sys.stdout
		sys.stdout = f
		#os.system("taskset -p -c " +str(core_count% os.cpu_count())+" "+str(os.getpid()))
		while True:

			#receive jobs from the simulator first, run scheduling policies on it and send it to pcpus
			while not job_receive_pipe.empty():
				self.total_jobs += 1
				#print("Total_jobs: "+str(self.total_jobs))
				job_now = job_receive_pipe.get()
				print(job_now.job_info())
				par_id = getattr(self, policy_name)(job_now)
				if par_id is None:
					#not schedulable job!
					self.failed_jobs += 1
					continue
				job_now.par_id = par_id
				pcpu_id = self.partition_pcpu_mapping[par_id]
				job_send_pipes[pcpu_id].put(job_now)
			#receive execution info:
			for (pcpu_id, _) in self.pcpus.items():
				while not info_pipes[pcpu_id].empty():
					
					jr = info_pipes[pcpu_id].get()
					if not jr.on_time:
						self.failed_jobs += 1
						print("Failed jobs: "+str(self.failed_jobs))
					#can report to the user here by logging
			if terminate_pipe.poll():
				msg = terminate_pipe.recv()
				print(msg)
				for tempP in self.process_list:
					tempP.terminate()
					tempP.join()
				#print(str(self.failed_jobs)+', '+str(self.total_jobs))
				terminate_pipe.send([self.failed_jobs, self.total_jobs])
				sys.out = old
				break
			#print("One loop ends")

	def best_fit(self, job_now):
		#this version does not take mode into consideration yet
		#for mode 1(partitioned scheduling), first judge whether the job has been allocated (check task id)
		#calculate density based on time now?
		#print("In the best fit")		
		time_now = datetime.datetime.now()
		real_period = (job_now.arb_ddl - time_now).total_seconds()*1000
		density_now = job_now.WCET/float(real_period)
		closest_gap = 1000
		smallest_id = None
		#print("Number of partitions: "+str(len(self.partitions)))
		for (par_id, partition_now) in self.partitions.items():
			#print("testing: "+str(job_now.job_id in self.partition_task[par_id]))
			#if so, directly, return the partition id
			if job_now.job_id in self.partition_task[par_id]:
				return par_id
			#or else, find the Best Fit based on the capacity left of each partition
			temp_density = self.partition_task_density[par_id] + density_now
			task_period = min(real_period, self.partition_task_period[par_id])
			capacity = partition_now.af - (partition_now.reg - 1)/task_period
			#print("Density and capcity: "+str(temp_density)+", "+str(capacity))
			if temp_density > capacity:
				continue
			else:
				#print("Find one fit")
				if capacity - temp_density < closest_gap:
					#print("Update the id")
					closest_gap = capacity - temp_density
					smallest_id = par_id
		if smallest_id is not None:
			self.partition_task_density[smallest_id] += density_now
			self.partition_task_period[smallest_id] = min(self.partition_task_period[smallest_id], real_period)
			self.partition_task[smallest_id].add(job_now.task_id)
			#print("Updating")
		return smallest_id

	def first_fit(self, job_now):
		time_now = datetime.datetime.now()
		real_period = (job_now.arb_ddl - time_now).total_seconds()*1000
		density_now = job_now.WCET/float(real_period)
		for (par_id, partition_now) in self.partitions.items():
			if job_now.job_id in self.partition_task:
				return par_id
			temp_density = self.partition_task_density[par_id] + density_now
			task_period = min(real_period, self.partition_task_period[par_id])
			capacity = partition_now.af - (partition_now.reg - 1)/task_period
			if temp_density > capacity:
				continue
			else:
				#if the partition can schedule the task, immediately return after updates
				self.partition_task_density[par_id] += density_now
				self.partition_task_period[par_id] = min(self.partition_task_period[par_id], real_period)
				self.partition_task[par_id].add(job_now.task_id)
				return par_id
		return None

	def worst_fit(self, job_now):

		time_now = datetime.datetime.now()
		real_period = (job_now.arb_ddl - time_now).total_seconds()*1000
		density_now = job_now.WCET/float(real_period)
		largest  = -1
		largest_id = None
		for (par_id, partition_now) in self.partitions.items():
			#if so, directly, return the partition id
			if job_now.job_id in self.partition_task:
				return par_id
			#or else, find the Best Fit based on the capacity left of each partition
			temp_density = self.partition_task_density[par_id] + density_now
			task_period = min(real_period, self.partition_task_period[par_id])
			capacity = partition_now.af - (partition_now.reg - 1)/task_period
			if temp_density > capacity:
				continue
			else:
				if capacity > largest:
					largest = capacity
					largest_id = par_id
		if largest_id is not None:
			self.partition_task_density[largest_id] += density_now
			self.partition_task_period[largest_id] = min(self.partition_task_period[largest_id], real_period)
			self.partition_task[largest_id].add(job_now.task_id)
		return largest_id