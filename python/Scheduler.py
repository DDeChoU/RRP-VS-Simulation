from OS_Simulator import OS_Simulator
from Partition import Partition
from Task import Task
from PCPU import PCPU
#1. What is resource_units: pcpu
#2. What is the factor in the resource_units: a parameter used by MulZ only
#3. How to return the schedule: MulZ simply chooses which partition goes to which unit, the detailed should be done by aaf
class Scheduler:

	def __init__(self, sum_af, pcpus):
		'''
		Args:

			sum_af: 		type: float; The total availability factor of partitions.
			pcpus:			type: PCPU[]; The list of PCPU, now merely a number!! 

		'''
		self.sum_af = sum_af
		self.pcpus = pcpus
		#invoke generate partitions here

		#pass the partitions to the pcpus after aaf

	def mulZ_FFD(self, partition_list):
		partition_list = sorted(partition_list, key=lambda x: x.af, reverse=True)
		for x in range(len(partition_list)):
			f = self.mulZ_FFD_Alloc(partition_list[x], self.pcpus)
			print(str(partition_list[x].af)+' is allocated to: '+str(f))
			if f==-1:
				return False
		return True

	def mulZ_FFD_Alloc(self, par, pcpus):
		fixed_list=[3,4,5,7]
		approx_weights = [0 for x in range(4)]
		smallest = 1
		f=-1
		for x in range(4):
			num=self.z_approx(par.af, fixed_list[x])
			#print("Result of Z_(", fixed_list[x], ",2) with weight", par.af,"is ", num, "\n");
			if num<smallest:
				f = fixed_list[x]
				smallest = num
			approx_weights[x] = smallest

		approx_weights = sorted(approx_weights)
		r = approx_weights[0]
		for x in range(len(self.pcpus)):
			if pcpus[x].factor==0:
				pcpus[x].factor=f
				pcpus[x].rest = 1-r
				return x
			elif pcpus[x].rest>=r:
				num=self.z_approx(par.af, pcpus[x].factor)
				pcpus[x].rest -= num
				return x
		return -1

	def z_approx(self, w, n):
		i=1
		j=0
		m=2
		largest = 1
		while True:
			if (n-i)/n >= w and (n-i != 1):
				largest = (n-i)/n
				i+= 1
			else:
				denom=n*m**j
				if 1/denom >= w:
					largest = 1/denom
					j+= 1
				else:
					return largest
		return -1

#test code
partition_list = []
pcpu_list = []
afs = [0.6, 0.65,0.3, 0.35, 0.5, 0.55, 0.25, 0.25, 0.25]
for af in afs:
	tempP = Partition(af,1)
	partition_list.append(tempP)
for i in range(4):
	p = PCPU()
	pcpu_list.append(p)
s = Scheduler(4,pcpu_list)
s.mulZ_FFD(partition_list)

