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
		self.set_aaf(self.magic7(self.af, self.reg))
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

