from Scheduler import Scheduler
from OS_Simulator import OS_Simulator
from PCPU import PCPU
from multiprocessing import Process

#main function
if __name__ == "__main__":
	#set up parameters
	pcpu_num = 4 #homogeneous pcpu for now
	load_ratio = 0.5
	sum_af = 2.5
	#initialize pcpus
	pcpus = []
	for i in range(pcpu_num):
		pcpu_now = PCPU()
		pcpus.append(pcpu_now)
	#initialize OS_Simulator
	load = sum_af*load_ratio
	simulator = OS_Simulator(load)

	#initialize Scheduler
	scheduler = Scheduler(sum_af, pcpus)
	print("Entering dead loop!")
	p = Process(target = scheduler.run_system, args = (simulator, 1, "best_fit"))
	p.start()
	p.join(20)
	print("Jobs received: "+str(scheduler.total_jobs)+" and job failed: "+str(scheduler.failed_jobs))


