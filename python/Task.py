class Task:
	count_task=0
	def __init__(self, c, p, d, r, is_periodic, is_hard_rt):
		'''

		Args:
			WCET: 		type: int; Worst Case Execution Time
			period:		type: int; The period of periodic tasks or the minimum arrivial interval of sporadic tasks
			ddl:		type: int; Relative deadline of each instance
			arr_time:	type: int; Relative arrival time
			is_periodic:type: bool; Sign that marks whether the task is periodic
			par_id:		type: string; The id of the partition the task is now allocated to, -1 means not allocated yet
			task_id:	type: string; The id of the task
			is_hard_rt: type: bool; Sign that marks whether the task is hard real-time
		'''
		self.WCET = c
		self.period=p
		self.ddl=d
		self.arr_time=r
		self.is_periodic=is_periodic
		self.par_id = -1
		self.task_id = str(Task.count_task)
		self.is_hard_rt = is_hard_rt
		Task.count_task += 1
	def task_info(self):
		info = 'Task '+str(self.task_id)+': Computation time: '+str(self.WCET)+'; Period: '+str(self.period)+'; DDL: '+str(self.ddl) \
				+'; Arrival Time: '+str(self.arr_time)+'; is periodic? '+str(self.is_periodic)
		return info

class Job:
	count_job = 0
	def __init__(self, c, D, task_id, is_hard_rt = False):
		'''
			WCET: 		type: int; Worst Case Execution Time of the job
			arb_ddl:	type: datetime; Arbitrary deadline of the job
			job_id:		type: string;  The id of the job
			par_id:		type: string;  The id of the partition the job is now allocated to, -1 means not allocated yet
			task_id:	type: string:  The id of the task the job belongs to.
			is_hard_rt:	type: bool; Decides whether it is hard real time or not
		'''
		self.WCET = c
		self.arb_ddl = D
		self.job_id = str(Job.count_job)
		Job.count_job += 1
		self.par_id = -1
		self.task_id = task_id
		self.is_hard_rt = is_hard_rt
	def job_info(self):
		info = 'Job '+str(self.job_id)+': WCET is '+str(self.WCET)+'; Deadline is: '+str(self.arb_ddl)+'; Task id is: '+str(self.task_id)
		return info
	def __gt__(self, j2):
		return self.arb_ddl>j2.arb_ddl
'''
Test code
a = Job(1,2)
b = Job(2,3)
print(a.job_id)
print(Job.count_job)
'''