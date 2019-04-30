import random

class Generation:
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
	def generate_tasks(self, target_load, has_sporadic = False, per_ratio = 0.5, hard_ratio = 0, highest_density = 1):
		'''
		 
		 Args:
			  - target_load: Total utilization to reach.
			  - has_sporadic: whether tasks has_sporadic or not
			  - per_ratio: the ratio of periodic tasks.
		'''
		task_set =[]
		#set the min and max accordingly
		utils = self.gen_kato_utilizations(target_load,0, highest_density)#generate utilizations based on the number of tasks generated
		num = len(utils)
		for i in range(num):
			util_now = utils[i]
			#wcet = random.randint(2, OS_Simulator.MAX_WCET)*OS_Simulator.TIME_SLICE_LEN
			#period = math.ceil(wcet/util_now*OS_Simulator.TIME_SLICE_LEN)
			period = random.randint(5*OS_Simulator.TIME_SLICE_LEN, 2000*OS_Simulator.TIME_SLICE_LEN)
			wcet = math.ceil(util_now*period)
			deadline = period
			#arrival = -math.log(1.0 - random.random())
			arrival = random.randint(0, 3000*OS_Simulator.TIME_SLICE_LEN)
			is_periodic= True
			is_hard_rt = False
			if hard_ratio>0:
				dice = random.random()
				if dice<hard_ratio:
					is_hard_rt = True
			elif has_sporadic:
				dice = random.random()
				if dice>= per_ratio:
					is_periodic = False
			task_now = Task(wcet, period, deadline, arrival, is_periodic, is_hard_rt)
			task_set.append(task_now)
			#print("Task #"+str(task_now.task_id)+" density is "+str(util_now))
		task_set = sorted(task_set, key=lambda x: x.arr_time)
		return task_set