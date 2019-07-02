#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "Task.h"
#include "Job.h"
#include "PCPU.h"
#include "Partition.h"
class Scheduler
{
private:
	int pcpu_num;
	vector<Task *> task_list;
	vector<Partition *> partition_list;
	vector<PCPU *> pcpu_list;
public:
	//the constructor takes in the number of pcpus
	Scheduler(int p_num)
	{
		pcpu_num = p_num;
		for(int i=0;i<pcpu_num;i++)
		{
			PCPU *p_now = new PCPU();
			pcpu_list.push_back(p_now);
		}
	}
	//takes in the task list to be executed.
	void setTasks(vector<Task *> t_list)
	{
		task_list = t_list;
	}
	void setPartitions(vector<Partition *> p_list)
	{
		partition_list = p_list;
	}
	//run simulates the schedule and should return a schedule in the form of string.
	string run();
};

#endif