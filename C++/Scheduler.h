#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "Task.h"
#include "Job.h"
#include "PCPU.h"
#include "Partition.h"
#include <string>
#include <unordered_map>
#include <list>
using std::string;
using std::unordered_map;
using std::list;
class Scheduler
{
private:
	int pcpu_num;
	//pcpus store the map from the id of pcpu to the pointer to the pcpu object.
	unordered_map<string, PCPU *> pcpus; 

	//partitions store the map from the id of partitions to the point to the partition object.
	unordered_map<string, Partition *> partitions;

	//time_partition_tables stores the table of each cpu. 
	//In the table, each time slice (each item) is mapped to a partition pointer.
	unordered_map<string, list<Partition *>> time_partition_tables;

	//partition_cpu_map records which pcpu a partition is mapped to.
	unordered_map<string, string> partition_cpu_map;

	//task_partition_map records which partition the task is executed last time
	unordered_map<string, string> task_partition_map;

	//initialize some parameters used to record the task execution 
	//to be added

public:
	//the constructor takes in the number of pcpus
	Scheduler(int p_num)
	{
		pcpu_num = p_num;
		string prefix = "PCPU#";
		for(int i=0;i<pcpu_num;i++)
		{
			PCPU *p_now = new PCPU(prefix+std::to_string(i));
			pcpu_list
		}
	}

	//run simulates the schedule and should return a schedule in the form of string.
	string run();


};
//TO DO:
//1. Get input: task_list and partition_list
//2. Run AAF/MulZ to allocate partitions
//3. Run PCPUs
//4. Run OS_Simulator
//5. Accomplish the dead loop that employs various scheduling policy (partitioned/global and etc.) 
#endif