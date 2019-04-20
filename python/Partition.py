import heapq
import math
import datetime
from Task import Task, Job
class Partition:
	count_partition=1
	def __init__(self, af, reg):
		'''
		Args:
			af:				type: double; Avaialbility factor of the partition
			reg:			type: int; The supply regularity of the partition
			partition_id:	type: string; The id of the partition
			job_queue:		type: heapq; The list of jobs, sorted by deadline.
			capacity:		type: double; Capacity left.
			aaf:			type: double; approximate availability factor, calculated by magic7
		'''
		self.af = af
		self.reg = reg
		self.partition_id = str(Partition.count_partition)
		Partition.count_partition += 1
		self.job_queue = []
		heapq.heapify(self.job_queue)
		self.capacity = -1
		self.aaf = self.af
	def set_aaf(self, aaf):
		self.aaf = math.ceil(aaf*10000)/10000 #ceil it to the third decimal digit so that hyper-period can be 1000 all the time

    #insert jobs into 
	def insert_job(self, Job):
		heapq.heappush(self.job_queue, Job)

	def schedule(self):
		try:
			job_now = heapq.heappop(self.job_queue)
			#print(len(self.job_queue))
			return job_now
		except IndexError:
			return None 


#test 
'''
p = Partition(0.5, 1)
now = datetime.datetime.now()
interval = datetime.timedelta(seconds = 5)

j_list = []
j_list.append(Job(2,now+interval,1))
interval = datetime.timedelta(seconds = 10)
j_list.append(Job(2,now+interval,2))
interval = datetime.timedelta(seconds = 4)
j_list.append(Job(1,now+interval,1))
#for i in range(3):
#	p.insert_job(j_list[i])

for i in range(3):
	print(p.schedule())'''

