import heapq
class Partition:
	count_partition=0
	def __init__(self, af, reg):
		'''
		Args:
			af:				type: double; Avaialbility factor of the partition
			reg:			type: int; The supply regularity of the partition
			partition_id:	type: string; The id of the partition
			job_queue:		type: heapq; The list of jobs, sorted by deadline.
			capacity:		type: double; Capacity left.
		'''
		self.af = af
		self.reg = reg
		self.partition_id = str(Partition.count_partition)
		Partition.count_partition += 1
		self.job_queue = []
		self.capacity = -1

	
