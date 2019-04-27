from Partition import Partition
from Partition import Partition
from Task import Task, Job
import math
from OS_Simulator import OS_Simulator
from Info_Message import Job_Report
from multiprocessing import Process, Queue
import copy
import datetime
import sys
import os
class PCPU:
	count_pcpu=0
	def __init__(self):
		'''
		Args:
			pcpu_id: 		type: string; The id of the pcpu
			hyperperiod:	type: int;	The length of the hyperperiod, in the unit of time slice
			time_par_table:	type: hash_map; The partition of each time slice
			par_list:		type: Partition[]; The list of partition running on the pcpu.
			factor:			type: int; Used in the MulZ
			rest:			type: double; Used in the MulZ
		'''
		self.pcpu_id = str(PCPU.count_pcpu)
		PCPU.count_pcpu += 1
		self.hyperperiod = 10000
		self.par_dict = {}
		self.time_par_table = [-1 for i in range(self.hyperperiod)]
		self.factor = 0
		self.rest = 1
		self.time_now = 0
		#run the multiprocessing here and set the signals and pipes. Task inserted are put in here. based on their ids.

	def set_partitions(self, par_list):
		total_aaf = 0
		smallest_aaf = 1
		#first use magic7 to calculate the approximate availability factor
		for i in range(len(par_list)):
			self.par_dict[par_list[i].partition_id] = par_list[i]
			aaf_now = self.magic7(par_list[i].af, par_list[i].reg) #using magic7 now
			#aaf_now = self.AAF(par_list[i].af, par_list[i].reg)
			self.par_dict[par_list[i].partition_id].set_aaf(aaf_now)
			#print(aaf_now)
			total_aaf += aaf_now
			if aaf_now<smallest_aaf:
				smallest_aaf = aaf_now
		if total_aaf>1:
			print("Partitions on PCPU #"+str(self.pcpu_id)+" is not schedulable")

		#use partition single to set up partitions.
		self.partition_single()
		return smallest_aaf

	def AAF(self, af, reg):
		if af==0:
			return 0
		if af>0 and reg==1:
			n = math.floor(math.log(af)/math.log(0.5))
			return 1/(2**n)
		else:
			n = math.ceil(math.log(af)/math.log(0.5))
			result = 1/(2**n)
			return self.AAF(af - result, reg-1) + result

	def magic7(self, af, reg):
		if af==0:
			return 0
		if reg==1:
			if af>0 and af<1.0/7:
				n = math.floor(self.approximateValue(math.log(7*af)/math.log(0.5)))

				result = 1/(7*(2**n))
				#print("First n"+str(n)+" and result is: "+str(result))				
				return result
			if af>=1.0/7 and af<=6.0/7:
				#print((math.ceil(self.approximateValue(7*af)))/7)
				result = (math.ceil(self.approximateValue(7*af)))/7
				#print("Middle result: "+str(result))
				return result
			if af>6.0/7 and af<1:
				n = math.ceil(self.approximateValue(math.log(7*(1-af))/math.log(0.5)))
				result = 1-(1/(7*(2**n)))
				#print("Last n: "+str(n)+" and result is: "+str(result))
				return result
		else:
			#print(af)
			#print("recursion needed L: "+str(self.L(af)))
			return (self.L(af)+self.magic7(af-self.L(af), reg - 1))
		return 0

	#private function, needed by magic7, used to round up the given value
	def approximateValue(self, value):
		result = math.floor(value)
		if value - result >0.99999:
			return result+1
		if value - result > 0.49999 and value - result < 0.5:
			return result + 0.5
		if value - result > 0 and value - result < 0.00001:
			return result
		return value

	#private function, needed by magic7
	def L(self, alpha):
		if alpha>0 and alpha<1.0/7:
			n = math.ceil(self.approximateValue(1*math.log(7*alpha)/math.log(0.5)))
			return 1.0/(7*(2**n))
		elif alpha>=1.0/7 and alpha<=6.0/7:
			return math.floor(self.approximateValue(7.0*alpha))/7
		elif alpha>6.0/7 and alpha<1:
			n = math.floor(self.approximateValue(math.log(7.0*(1-alpha))/math.log(0.5)))
			return 1-(1.0/(7*2**n))
		elif alpha==1:
			return 1
		return 0
	def partition_single(self):
		'''
			variables needed: self.partition_list(AAF calculated), self.hyperperiod
			store the final result to self.time_par_table
		'''

		timeslices = [[] for x in range(len(self.par_dict))]
		avail_time_slices = set()
		aaf_left = []
		partition_list = []
		for j in range(self.hyperperiod):
			avail_time_slices.add(j)
		for (_, par_now) in self.par_dict.items():
			aaf_left.append(par_now.aaf)
			partition_list.append(par_now)


		# how is distance decided???? how many time slices level l include? divides them on average. 

		counter = len(partition_list)
		level = 0
		firstAvailableTimeSlice = 0
		while counter>0:
			w = 1/(2**level)
			num = math.floor(w*self.hyperperiod) 
			interval = 2**level
			for i in range(len(partition_list)):
				aaf_now = aaf_left[i]
				if aaf_now>=w or (self.approximateValue(abs(aaf_now-w))==0 and aaf_now>0):
					for j in range(num):
						time_slice_now = firstAvailableTimeSlice + j*interval
						if time_slice_now>=self.hyperperiod or self.time_par_table[time_slice_now]!= -1:
							print("Allocation conflicts!")
							continue
						self.time_par_table[time_slice_now] = partition_list[i].partition_id
						#timeslices[i].append(time_slice_now)
						avail_time_slices.discard(time_slice_now)
						#print(avail_time_slices)
					if len(avail_time_slices)==0:
						counter = 0
						break
					firstAvailableTimeSlice = min(avail_time_slices)
					aaf_left[i] -= w
					if aaf_left[i]<=0.0001:
						counter -= 1
			level += 1

		#sort and insert timeslices into time_par_table
	def calculateW(self, n):
		if n<=3:
			return 1.0-(1.0/(112/(2**n)))
		if n>3 and n<=0:
			return (10.0-n)/7
		else:
			return (1.0/(7*2**(n-10+1)))
		return 0

	def printSchedule(self):
		for i in range(20):
			print(str(i)+': '+str(self.time_par_table[i])+'; ')


	def execute(self,execution_time):
		time_start = datetime.datetime.now()
		while True:
			end = datetime.datetime.now()
			internal = (end-time_start).total_seconds()*1000
			if internal>=execution_time:
				return

	def run_pcpu(self, info_pipe, job_receiver, core_rank):
		f = open("log/PCPU"+str(self.pcpu_id)+".log", "w")
		old = sys.stdout
		sys.stdout = f

		#print("In the dead loop of pcpu now.")
		r=os.popen("taskset -p -c " +str(core_rank)+" "+str(os.getpid()))
		print(r.read()) 
		while True:

			# find the job to be running here: 
			next_domain = self.time_par_table[self.time_now]
			self.time_now += 1
			self.time_now %= self.hyperperiod
			if next_domain == -1:
				continue
			partition_now = self.par_dict[next_domain]
			#print ("Running partition #"+partition_now.partition_id)
			job_now = partition_now.schedule()
			#execute it for 10 milliseconds by default:
			execution_time = OS_Simulator.TIME_SLICE_LEN
			if job_now is None:
				self.execute(OS_Simulator.TIME_SLICE_LEN)
			else:
				if job_now.WCET<OS_Simulator.TIME_SLICE_LEN:
					execution_time = job_now.WCET

				self.execute(OS_Simulator.TIME_SLICE_LEN)
			#report execution with the info_pipe
				if job_now.WCET > OS_Simulator.TIME_SLICE_LEN:
					job_now.WCET -= OS_Simulator.TIME_SLICE_LEN
					partition_now.insert_job(job_now)
				else:
					accomplished_time = datetime.datetime.now() #not fair to report not real-time if it is smaller
					jr = Job_Report(job_now, accomplished_time, partition_now.partition_id)
					info_pipe.put(jr)
			#receive jobs here, allocate based on the partition_id
			while not job_receiver.empty():
				job_now = job_receiver.get()
				#insert jobs to corresponding partitions
				par_now_id = job_now.par_id
				self.par_dict[par_now_id].insert_job(job_now)
			f.flush()
#test code for partition: test passed
'''
p = PCPU()
par1 = Partition(0.375, 2)
par2 = Partition(0.3125, 2)
par3 = Partition(0.3125, 2)
partition_list = [par1, par2, par3]
p.set_partitions(partition_list)
info_pipe = Queue()
job_pipe = Queue()

now = datetime.datetime.now()
interval = datetime.timedelta(seconds = 4)
j_list = []
j_list.append(Job(2000,now+interval,1))
interval = datetime.timedelta(seconds = 10)
j_list.append(Job(2,now+interval,2))
interval = datetime.timedelta(seconds = 0.004)
j_list.append(Job(1,now+interval,1))

#use for loop to set up the jobs in partitions
for (_, par) in p.par_dict.items():
	for j in range(3):
		j_temp = copy.deepcopy(j_list[j])
		par.insert_job(j_temp)

pro = Process(target = p.run_pcpu, args=(info_pipe, job_pipe))
pro.start()
while True:
	if not info_pipe.empty():
		jr = info_pipe.get()
		print(jr.report())
'''
