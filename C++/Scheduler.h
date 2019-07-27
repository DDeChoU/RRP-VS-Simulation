#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <string>
#include <unordered_map>
#include <list>
#include <math.h>
#include <queue>
#include <vector>
#include <set>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <signal.h>
#include <unordered_set>
#include <limits.h>

#include "Task.h"
#include "Job.h"
#include "PCPU.h"
#include "Partition.h"
#include "Generation.h"
#include "OS_Simulator.h"
using std::string;
using std::unordered_map;
using std::list;
using std::priority_queue;
using std::set;
using std::vector;
using std::unordered_set;
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
	//unordered_map<string, list<Partition *> > time_partition_tables;

	//partition_cpu_map records which pcpu a partition is mapped to.
	unordered_map<string, string> partition_cpu_map;

	//task_partition_map records which partition the task is executed last time
	unordered_map<string, string> task_partition_map;

	//domain_partition_map includes a map from domain to partition
	//for the simplified version simulation, only one key is stored.
	unordered_map<string, list<string> > domain_partition_map;

	unordered_map<string, int> hyper_periods;//all aafs are constrained to 3-digit-long after the decimal point.
	

	//initialize some parameters used to record the task execution 
	
	// the number of jobs that arrive
	long long total_job_num;

	//the number of jobs that miss the deadline
	long long total_miss_num;
public:
	//the constructor takes in the number of pcpus
	Scheduler(int p_num);

	bool set_partitions(vector<Partition> &partition_list);

	//run simulates the schedule and should return a schedule in the form of string.
	//pass the output redirection to run through ostream &out.
	void run(vector<Task> taskList, std::ostream &out, int schedule_mode, int simulation_time, int starting_port);

	int getJobNum(){return total_job_num;}
	int getMissNum(){return total_miss_num;}
private:
	bool MulZ(); //allocate partitions to pcpu, if not schedulable, return false.
	bool Magic7(string pcpu);//build the time slice table of a certain pcpu based on the allocation returned by MulZ
	vector<int> z_approx(double w, double n);//calculate the approximated availability factor

	//pass in a partition and returns the pcpu that partition should lie on.
	//this function needs to track the space left on a pcpu all the time.
	//check the simulation_time
	string mulZ_FFD_Alloc(Partition *p, unordered_map<string, double> &factors, unordered_map<string, double> &rests);

	struct cmp_par
	{
		bool operator()(const Partition *a, const Partition *b) const
		{
			return a->getAF()<b->getAF();
		}
	};

	//decides where a job should be allocated to, the schedule mode indicates which policy should be used.
	//1 for best fit, 2 for first fit, 3 for worst fit, 4 for almost worst fit.
	//the chosen partition's parameter will be modified directly in the function.
	string schedule_job(const Job &j, int schedule_mode);

	//partitioned scheduling policies.
	string best_fit(const Job &j, double density);
	string worst_fit(const Job &j, double density);
	string first_fit(const Job &j, double density);
	string almost_worst_fit(const Job &j, double density);
	//double approximateValue(double value);

	//initialize the time-partition table in the given pcpu;
	//void partition_single(string pcpu_id);

	//calculate the hyperperiod of corresponding pcpu.
	//void calculate_hp(string pcpu_id, vector<Partition *> partition_set);
	//calculate the lcm of a and b
	//int lcm(int a, int b);
};

Scheduler::Scheduler(int p_num)
{
	pcpu_num = p_num;
	string prefix = "PCPU#";
	for(int i=0;i<pcpu_num;i++)
	{
		string id_now = prefix+std::to_string(i);
		PCPU *p_now = new PCPU(id_now);
		pcpus[id_now] = p_now;
		hyper_periods[id_now] = 1;
	}
	total_job_num = 0;
	total_miss_num = 0;

}

vector<int> Scheduler::z_approx(double w, double n)
{
	int i = 0, j=0, m = 2;
	double largest = 1;
	vector<int> result;
	while(true)
	{
		if((n-i)/n>= w && (n-i)!=1)
		{
			largest = (n-i)/n;
			result.clear();
			result.push_back(n-i);
			result.push_back(n);
			i+=1;
		}
		else
		{
			double denom = n*pow(m, j);
			if(1/denom >= w)
			{
				largest = 1/denom;
				result.clear();
				result.push_back(1);
				result.push_back(denom);
				j+= 1;
			}
			else
				return result;
		}
	}
	return result;

}
bool Scheduler::MulZ()
{
	//initialize assistance maps
	unordered_map<string, double> factors;
	unordered_map<string, double> rests;
	for(auto it = pcpus.begin();it!=pcpus.end();it++)
	{
		string cpu_id = it->first;
		factors[cpu_id] = 0;
		rests[cpu_id] = 1;
	}
	//calculate which resource each partition should be allocated to,
	//do the calculation in non-decreasing order for partitions' af

	priority_queue<Partition *, std::vector<Partition *>, cmp_par> pq;
	for(auto it = partitions.begin();it!=partitions.end();it++)
	{
		Partition *p_now = it->second;
		pq.push(p_now);
		/*
		string res = mulZ_FFD_Alloc(p_now, factors, rests);
		if(res=="")
		{
			return false;
		}
		else
		{
			partition_cpu_map[it->first] = res;
		}*/
	}
	//std::cout<<"Priority queue loop starts"<<std::endl;
	while(!pq.empty())
	{
		Partition *p_temp = pq.top();
		pq.pop();
		//std::cout<<"Allocating "<<p_temp->getID()<<std::endl;
		string res = mulZ_FFD_Alloc(p_temp, factors, rests);
		//std::cout<<res<<std::endl;
		if(res=="")
		{
			return false;
		}
		else
		{
			std::cout<<p_temp->getID()<<"("<<p_temp->getAAFUp()<<"/"<<p_temp->getAAFDown() <<") mapped to "<<res<<std::endl;
			partition_cpu_map[p_temp->getID()] = res;
		}
	}
	//std::cout<<"Priority queue loop ends."<<std::endl;
	//run aaf for each pcpu to set up the time-partition tables.
	//the aaf of each partition is already calculated, no need to recalculate, invoke partition_single in pcpu here
	/* Test this after the fraction part is tested. */
	for(auto it = pcpus.begin();it!=pcpus.end();it++)
	{
		list<Partition *> partitions_now;
		for(auto it2 = partition_cpu_map.begin();it2!=partition_cpu_map.end();it2++)
		{
			if(it2->second== ( (it->second)) ->getID())
			{
				partitions_now.push_back(partitions[it2->first]);
			}
		}
		(it->second)->partition_single(partitions_now);
		//std::cout<<(it->second)->getID()<<std::endl;
		//std::cout<<(it->second)->showTTable();

	}
	
	return true;

}

string Scheduler::mulZ_FFD_Alloc(Partition *p, unordered_map<string, double> &factors, unordered_map<string, double> &rests)
{
	int fixed_list[4] = {3,4,5,7};
	int x_index = 5;
	double smallest = INT_MAX;
	int smallest_up = 0, smallest_down = INT_MAX;
	for(int i=0;i<4;i++)
	{
		vector<int> num_frac = z_approx(p-> getAF(), fixed_list[i]);
		double num = (double)num_frac[0]/num_frac[1];
		if(num<smallest)
		{
			smallest = num;
			smallest_up = num_frac[0];
			smallest_down = num_frac[1];
			x_index = fixed_list[i];
		}
	}

	double r = smallest;
	for(auto it = factors.begin(); it!=factors.end();it++)
	{
		string pcpu_id_now = it->first;
		//std::cout<<"After "<<pcpu_id_now<<std::endl;
		if(factors[pcpu_id_now]==0)
		{
			factors[pcpu_id_now] = x_index;
			rests[pcpu_id_now] = 1 - r;
			p->setAAF(r);
			p->setAAFFrac(smallest_up, smallest_down);
			//std::cout<<"Final factor is: "<<r<<std::endl;
			return pcpu_id_now;
		}
		else if(rests[it->first]>=r)
		{
			vector<int> rfrac= z_approx(p->getAF(), factors[pcpu_id_now]);
			r = (double)rfrac[0]/rfrac[1];
			//std::cout<<"Final factor is: "<<r<<std::endl;
			rests[pcpu_id_now] -= r;
			p->setAAF(r);
			p->setAAFFrac(rfrac[0], rfrac[1]);
			return pcpu_id_now;
		}
	}
	return "";
}

bool Scheduler::set_partitions(vector<Partition> &partition_list)
{
	//insert the partitions into variable 'partitions'
	for(int i=0;i<partition_list.size();i++)
	{
		partitions[partition_list[i].getID()] = &partition_list[i];
		//mind that if partition_list gets eliminated, the pointers in "partitions" will be invalid as well.
	}
	// map partitions to pcpu and maintain the time slice table here
	//std::cout<<"MulZ starts."<<std::endl;
	if(!MulZ())
	{
		std::cout<<"The partitions are not schedulable"<<std::endl;
		partitions.clear();
		return false;
		//put a log here saying something is wrong.
	}
	return true;
}

void Scheduler::run(vector<Task> taskList, std::ostream &out, int schedule_mode, int simulation_time, int starting_port)
{
	//initialize os simulator with task_list generated by Generation.
	OS_Simulator os(taskList);

	//build the pipes and start running PCPU and jobs
	//int starting_port = 50;
	vector<int> ports;//store all ports in this array, the first port is for the generator
	for(int i=0;i<=pcpu_num;i++)
		ports.push_back(starting_port+i*2);
	//set two connections
	unordered_map<string, Socket_Conn *> receive_connections;
	unordered_map<string, Socket_Conn *> send_connections;
	vector<int> pids;
	auto it = pcpus.begin();
	unordered_set<string> missed_jobs;
	for(int i=0;i<=pcpu_num;i++)
	{
		int pid = fork();

		//fork here get stuck! Needs to set up the server at the same time
		if(pid==0)
		{
			//initialize the deadloop in each pcpu
			//sleep to let the server set up first.
			sleep(1);
			if(i==pcpu_num)
			{
				os.generate_jobs( ports[i]);
			}
			else
			{
				it->second->run_pcpu(ports[i]);
			}
			out<<"Child Process ends."<<std::endl;
			exit(0);
		}
		else
		{
			out<<"New process: "<<pid<<std::endl;
			pids.push_back(pid);
			Socket_Conn *temp = new Socket_Conn(ports[i], true);
			if(i==pcpu_num)
			{	
				//the last one is OS's socket
				receive_connections["OS"] = temp;
				out<<"Building OS sending pipe"<<std::endl;
			}
			else
			{
				out<<"Building cpu sending pipe for"<<it->first<<std::endl;
				receive_connections[it->first] = temp;
			}
			//let the other socket get prepared
			sleep(1);
			Socket_Conn *temp2 = new Socket_Conn(ports[i]+1, false);
			if(i==pcpu_num)
			{
				send_connections["OS"] = temp2;
				out<<"Building OS receiving pipe"<<std::endl;
			}
			else
			{
				out<<"Building cpu receiving pipe for"<<it->first<<std::endl;
				send_connections[it->first] = temp2;
				it++;
			}

		}
		
	}
	//error occurs before the fork ends
	//out<<"Fork ends"<<std::endl;
	//go on to the scheduler's dead loop
	system_clock::time_point start_time = system_clock::now();
	while(true)
	{
		//receive jobs first from OS_Simulator
		//segmentation fault here????
		vector<string> arriving_jobs = receive_connections["OS"]->receiveInfo();
		//out<<arriving_jobs.size()<<" jobs received."<<std::endl;
		for(int i=0;i<arriving_jobs.size();i++)
		{
			Job j_now(arriving_jobs.at(i));
			//get the time left now and calculate the density
			auto dur = j_now.getDDL() - system_clock::now();
			double ms_ddl = dur.count()/(double)1000;
			double density = j_now.getComputationTime()/ms_ddl;
			string par_selected = "";
			total_job_num ++;
			if(task_partition_map.count(j_now.getTaskId())!=0)
			{
				par_selected = task_partition_map[j_now.getTaskId()];
				// no need to modify the aaf_left here because partitioned scheduling does not release the resource even when the job leaves.
			}
			else
			{
				switch(schedule_mode)
				{
				case 1:
					//best fit
					par_selected = best_fit(j_now, density);
					break;
				case 2:
					//worst fit
					par_selected = worst_fit(j_now, density);
					break;
				case 3:
					//first fit
					par_selected = first_fit(j_now, density);
					break;
				case 4:
					//almost worst fit
					par_selected = almost_worst_fit(j_now, density);
					break;
				}

				if(par_selected=="")
				{
					//handle not schedulable here
					out<<j_now.getJobId()<<" is not schedulable! \n";
					total_miss_num ++;
					continue;
				}
				//modify the parameter partition selected here (aaf_left)
				partitions[par_selected]->setAAFLeft(partitions[par_selected]->getAAFLeft() - density);
				//maintain the job-partition map here
				task_partition_map[j_now.getTaskId()] = par_selected;
			}
			//out<<j_now.getJobId()<<" is scheduled to "<<par_selected<<std::endl;
			//modify the job's partition tag and allocate the task to the pcpu that partition is on.
			j_now.setPartitionId(par_selected);
			string pcpu_now = partition_cpu_map[par_selected];
			send_connections[pcpu_now]->sendInfo(j_now.wrap_info());
		}
		//Receive info from each pcpu.
		for(auto it = receive_connections.begin();it!=receive_connections.end();it++)
		{
			if(it->first=="OS")
				continue;
			vector<string> taskslices = it->second->receiveInfo();
			//where does the task slice come from?
			//out<<taskslices.size()<<" task slices done."<<std::endl;
			for(int i=0;i<taskslices.size();i++)
			{
				TaskSlice ts(taskslices.at(i));
				if(!ts.isOnTime()&&missed_jobs.count(ts.getJobID())==0)
				{
					missed_jobs.insert(ts.getJobID());
					total_miss_num ++;
				}
				out<<"Task slice report:"<<ts.wrap_info()<<std::endl;
				//out<<taskslices.at(i)<<std::endl;
			}
		}

		auto dur = system_clock::now() - start_time;
		double ms = dur.count()/(double)1000;
		if(ms>=simulation_time)
		{
			
			//send poweroff to all children processes.
			string shutdownSignal = "Poweroff\n";
			for(auto it=send_connections.begin();it!=send_connections.end();it++)
			{
				out<<"Shutting "<<it->first<<std::endl;
				it->second->sendInfo(shutdownSignal);
				sleep(1);
			}

			/*
			*/
			for(auto it = send_connections.begin();it!=send_connections.end();it++)
			{
				it->second->shutDown();
				delete it->second;
			}
			for(auto it=receive_connections.begin();it!=receive_connections.end();it++)
			{
				it->second->shutDown();
				delete it->second;
			}
			for(int i=0;i<pids.size();i++)
			{
				out<<"Killing "<<pids.at(i)<<std::endl;
				kill(pids.at(i), 9);

			}
			break;

		}


	}

	cout<<"Shutting down scheduler.\n";
}


string Scheduler::best_fit(const Job &j, double density)
{
	string p = "";
	double smallest_gap = 2;//the gap cannot be larger than 2

	for(auto it = partitions.begin();it!=partitions.end();it++)
	{
		double al = it->second->getAAFLeft();
		if(al>=density && al - density<smallest_gap)
		{
			smallest_gap = al - density;
			p = it->first;
		}

	}

	return p;

}
string Scheduler::worst_fit(const Job &j, double density)
{
	string p = "";
	double largest_gap = -1;
	for(auto it = partitions.begin();it!=partitions.end();it++)
	{
		double al = it->second->getAAFLeft();
		if(al>=density && al-density>largest_gap)
		{
			largest_gap = al-density;
			p = it->first;
		}
	}
	return p;
}
string Scheduler::first_fit(const Job &j, double density)
{
	for(auto it = partitions.begin();it!=partitions.end();it++)
	{
		double al = it->second->getAAFLeft();
		if(al>=density)
			return it->first;
	}
	return "";
}
string Scheduler::almost_worst_fit(const Job &j, double density)
{
	string largest_id = "", second_largest_id = "";
	double largest_cap = -1, second_largest_cap = -1;
	for(auto it = partitions.begin();it!=partitions.end();it++)
	{
		double al = it->second->getAAFLeft();
		if(al>=density)
		{
			if(al>largest_cap)
			{
				second_largest_cap = largest_cap;
				second_largest_id = largest_id;
				largest_cap = al;
				largest_id = it->first;
			}
			else if(al>second_largest_cap)
			{
				second_largest_cap = al;
				second_largest_id = it->first;
			}
		}
	}

	if(second_largest_id == "")
	{
		second_largest_id = largest_id;
	}
	return second_largest_id;

}

#endif