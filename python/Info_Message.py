class Job_Report:

	def __init__(self, job, accomplished_time):
		#retrieve info from job and judge whether it is real-time or not
		'''
			Args:
				job_id:  		type: string, the id of the job reported
				task_id:		type: string, the id of the task this job belongs to
				on_time:		type: bool, sign that marks whether the job is accomplished in time
		'''
		self.job_id = job.job_id
		self.task_id = job.task_id
		#print("DDL: "+str(job.arb_ddl))
		#print("Now: "+str(accomplished_time))
		if job.arb_ddl >= accomplished_time:
			self.on_time = True
		else:
			self.on_time = False

	def report(self):
		message = "Job #"+str(self.job_id)+" that belongs to Task #"+str(self.task_id)+"'s real-time nature: "+str(self.on_time)
		return message


