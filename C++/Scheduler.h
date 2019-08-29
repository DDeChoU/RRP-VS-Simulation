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
	//for the simplified version simulation, only one key is stored. Not in use now
	//unordered_map<string, list<string> > domain_partition_map;

	//hyper_periods is not in use now.
	//unordered_map<string, int> hyper_periods;//all aafs are constrained to 3-digit-long after the decimal point.
	


	//unordered_map<string, string> partition_job_map;//global and semi-partitioned used only, shows which job works on which partition
	unordered_map<string, list<string> > partition_job_map;//semi-partitioned used only, shows which jobs are working on which partition, when migrated, use assign function.

	unordered_map<string, string> partition_soft_job_map;//semi-partitioned used only, used to save the soft job currently running on the partition.

	unordered_map<string, Job> job_full_map;//global and semi-partitioned used only, find the instance of a job

	unordered_map<string, double> job_densities;//semi-partitioned used only, gives the density of a job.

	list<string> sorted_partitions;//the id of partitions in sorted order, global and semi_partitioned used only.

	list<string> waiting_jobs;//the list that keeps track of soft real-time jobs

	//initialize some parameters used to record the task execution 
	// the number of jobs that arrive
	long long total_job_num;

	//the number of jobs that miss the deadline
	long long total_miss_num;

	long long total_hard_job_num;

	long long total_hard_miss_num;

	unordered_map<string, Socket_Conn *> global_send_connections;

public:
	//the constructor takes in the number of pcpus
	Scheduler(int p_num);

	bool set_partitions(vector<Partition> &partition_list);

	//run simulates the schedule and should return a schedule in the form of string.
	//pass the output redirection to run through ostream &out.
	//schedule_mode and corresponding algorithms:
	//  1: Best_Fit (partitioned)
	//  2: Worst_Fit(partitioned)
	//	3: First_Fit(partitioned)
	//	4: Almost_Worst_Fit(partitioned)
	//	5: global EDF (global)
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
	//1 for best fit, 2 for first fit, 3 for worst fit, 4 for almost worst fit, 5 for global EDF, 6 for semi-partitioned scheduling
	//the chosen partition's parameter will be modified directly in the function (aaf_left and task_partition_map).
	//not in use now
	//string schedule_job(const Job &j, int schedule_mode);

	//partitioned scheduling policies.
	string best_fit(const Job &j, double density);

	//worst_fit, first_fit and almost worst fit not in use now
	/*
	string worst_fit(const Job &j, double density);
	string first_fit(const Job &j, double density);
	string almost_worst_fit(const Job &j, double density);
	*/

	//global serie of functions not in use
	//string global_add_job(Job &j, ostream &out);
	//void global_remove_job(string j_name, ostream &out);
	//double approximateValue(double value);

	//initialize the time-partition table in the given pcpu;
	//void partition_single(string pcpu_id);

	//calculate the hyperperiod of corresponding pcpu.
	//void calculate_hp(string pcpu_id, vector<Partition *> partition_set);
	//calculate the lcm of a and b
	//int lcm(int a, int b);

	//add a job into the list
	void semi_add_job(Job &j);

	//remove the given job from the list
	void semi_remove_job(string job_name);

	//add the job into the partition, does not contain the sending of updated job info
	void add_job_to_partition(string par, Job &j);

	//update the partition with EDF inside, send info to the PCPU to update it as well.
	void update_par_edf(string par);

	void insert_soft_job(Job &j);

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
		//hyper_periods[id_now] = 1;
	}
	total_job_num = 0;
	total_miss_num = 0;
	total_hard_miss_num = 0;
	total_hard_job_num = 0;

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
			//std::cout<<p_temp->getID()<<"("<<p_temp->getAAFUp()<<"/"<<p_temp->getAAFDown() <<") mapped to "<<res<<std::endl;
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
		//std::cout<<"The partitions are not schedulable"<<std::endl;
		partitions.clear();
		return false;
		//put a log here saying something is wrong.
	}

	//sort the partitions based on their AAFs
	for(int i=0;i<partition_list.size();i++)
	{
		Partition *p_now = &partition_list[i];
		auto it = sorted_partitions.begin();
		for( ;it!=sorted_partitions.end();it++)
		{
			if(partitions[*it]->getAAF() <= p_now->getAAF())
			{
				sorted_partitions.insert(it, p_now->getID());
				break;
			}
		}
		if(it==sorted_partitions.end())
			sorted_partitions.push_back(p_now->getID());
		list<string> temp;
		partition_job_map[p_now->getID()] = temp;
	}
	return true;
}
/*
//not in use now
string Scheduler::schedule_job(const Job &j_now, int schedule_mode)
{
	auto dur = duration_cast<microseconds>(j_now.getDDL() - system_clock::now());
	double ms_ddl = dur.count()/(double)1000;
	double density = j_now.getComputationTime()/ms_ddl;
	//out<<"Job received "<<j_now.print_info()<<"\n";
	string par_selected = "";
	total_job_num ++;
	if(schedule_mode<5 && task_partition_map.count(j_now.getTaskId())!=0)
	{
		par_selected = task_partition_map[j_now.getTaskId()];
		// no need to modify the aaf_left here because partitioned scheduling does not release the resource even when the job leaves.
	}
	else if(schedule_mode < 5)//execute partitioned scheduling
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
			//out<<j_now.getJobId()<<" is not schedulable! \n";
			return par_selected;
		}
	

		//modify the parameter partition selected here (aaf_left)
		partitions[par_selected]->setAAFLeft(partitions[par_selected]->getAAFLeft() - density);
		//maintain the job-partition map here
		task_partition_map[j_now.getTaskId()] = par_selected;

	}

	return par_selected;
}
*/

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
	global_send_connections = send_connections;
	//error occurs before the fork ends
	//out<<"Fork ends"<<std::endl;
	//go on to the scheduler's dead loop
	system_clock::time_point start_time = system_clock::now();
	while(true)
	{
		//receive jobs first from OS_Simulator
		vector<string> arriving_jobs = receive_connections["OS"]->receiveInfo();
		//out<<arriving_jobs.size()<<" jobs received."<<std::endl;
		for(int i=0;i<arriving_jobs.size();i++)
		{
			Job j_now(arriving_jobs.at(i));
			//get the time left now and calculate the density
			/*
			//global codes
			if(schedule_mode==5)
			{
				//std::cout<<j_now.print_info()<<std::endl;
				total_job_num ++;
				global_add_job(j_now, out);

			}*/
			total_job_num ++;
			if(j_now.isHardRT())
				total_hard_job_num ++;
			//semi-partitioned codes
			if(schedule_mode==6)
			{
				if(j_now.isHardRT() && task_partition_map.count(j_now.getTaskId())!=0)
				{
					//assign j_now to the given partition, no need to modify the parameters (hard rt jobs leave without modifying)
					//std::cout << j_now.print_info() << std::endl;
					string par = task_partition_map[j_now.getTaskId()];
					add_job_to_partition(par, j_now);
					update_par_edf(par);
					//std::cout<<j_now.getJobId()<<" is assigned to "<<par<<" as before."<<std::endl;

				}
				//invoke semi_add_job here!! Both soft real-time and hard real-time can invoke semi_add_job here
				else
				{
					//std::cout<< std::endl << j_now.print_info() ;
					semi_add_job(j_now);
				}
			}
			/*
			string par_selected = schedule_job(j_now, schedule_mode);
			out<<j_now.getJobId()<<" is scheduled to "<<par_selected<<"."<<std::endl;
			if(par_selected=="")
			{
				total_miss_num++;
				continue;
			}
			*/
			//modify the job's partition tag and allocate the task to the pcpu that partition is on.

		}
		//Receive info from each pcpu.
		for(auto it = receive_connections.begin();it!=receive_connections.end();it++)
		{
			if(it->first=="OS")
				continue;
			vector<string> taskslices = it->second->receiveInfo();
			//where does the task slice come from? ----> it->first
			//out<<taskslices.size()<<" task slices done."<<std::endl;
			//string partition_now = it->first;
			//string pcpu_now = partition_cpu_map[partition_now];
			for(int i=0;i<taskslices.size();i++)
			{
				TaskSlice ts(taskslices.at(i));
				if(it->first!=job_full_map[ts.getJobID()].getPartitionId())
					continue;
				out<<"Task slice report from "<<it->first<<" : "<<ts.wrap_info()<<"\n";
				if(!ts.isOnTime()&&missed_jobs.count(ts.getJobID())==0)
				{
					//std::cout<<ts.getJobID()<<" misses the deadline! "<<std::endl;
					//std::cout<< ts.wrap_info();
					missed_jobs.insert(ts.getJobID());
					total_miss_num ++;
					if(ts.isHardRT())
						total_hard_miss_num ++;
				}
				//if the job is done, remove the job and maintain the list
				if(schedule_mode==6)
				{
					if(ts.getTimeLeft()<0.001 && ts.getJobID()!=""&&job_full_map.count(ts.getJobID())!=0)
					{
						cout<<ts.getJobID()<<" is being removed. \n";
						// TODO: invoke semi_remove_job here!!
						semi_remove_job(ts.getJobID());
					}
					//need to update the info in the global map
					//still works for semi-partitioned schedule
					else if(job_full_map.count(ts.getJobID())!=0 && ts.getPartitionId() == job_full_map[ts.getJobID()].getPartitionId())
					{
						job_full_map[ts.getJobID()].setComputationTime(ts.getTimeLeft());
					}
					/* no need to send back the job
					string j_now_name = partition_job_map[partition_now];
					if(j_now_name=="")
						continue;
					Job j_now = job_full_map[j_now_name];

					j_now.setPartitionId(partition_now);
					send_connections[pcpu_now]->sendInfo(j_now.wrap_info());
					*/
				}
				
				//out<<taskslices.at(i)<<std::endl;
			}
		}

		auto dur = duration_cast<microseconds> (system_clock::now() - start_time);
		double ms = dur.count()/(double)1000;
		if(ms>=simulation_time)
		{
			//update the miss number first
			auto time_now = system_clock::now();
			//print time now
			stringstream ss;
			time_t et = system_clock::to_time_t(time_now);
			const auto nowMS = duration_cast<milliseconds>(time_now.time_since_epoch())%1000;
			ss<<std::put_time(localtime(&et), "%F %T")<<"."<<std::setfill('0')<<std::setw(3)<<nowMS.count();
			//std::cout<<ss.str()<<std::endl;
			
			//check how many jobs in each partition has missed the deadline
			for(auto it_par = partition_job_map.begin(); it_par != partition_job_map.end();it_par++)
			{
				list<string> &job_list_now = it_par->second;
				for(auto it_job = job_list_now.begin(); it_job != job_list_now.end();it_job++)
				{
					if(job_full_map[*it_job].getDDL() < time_now && missed_jobs.count(*it_job)==0)
					{
						missed_jobs.insert(*it_job);
						//std::cout << *it_job << " misses the deadline at the end "<<std::endl;
						//std::cout<< job_full_map[*it_job].print_info()<<std::endl;
						total_miss_num ++;
						total_hard_miss_num++;
					}
				}

			}


			//check how many jobs in thw waiting_already exceeds the deadline
			//std::cout<<"There are "<<waiting_jobs.size()<<" jobs waiting "<<std::endl; 
			for(auto it = waiting_jobs.begin();it!=waiting_jobs.end();it++)
			{
				Job j_now = job_full_map[*it];

				if(j_now.getDDL()<time_now && missed_jobs.count(*it)==0)
				{
					//std::cout<<j_now.print_info();
					missed_jobs.insert(*it);
					total_miss_num++;
				}
			}


			//std::cout<<total_miss_num<<std::endl;
			
			//send poweroff to all children processes.
			string shutdownSignal = "Poweroff\n";
			for(auto it=send_connections.begin();it!=send_connections.end();it++)
			{
				out<<"Shutting "<<it->first<<std::endl;
				it->second->sendInfo(shutdownSignal);
				sleep(1);
			}

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
			//check the result of jobs
			
			break;

		}
		out.flush();

	}

	out<<"Shutting down scheduler.\n";
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

//notice: this function does not modify the parameters of the partitions
void Scheduler::add_job_to_partition(string par, Job &j)
{
	j.setPartitionId(par);
	job_full_map[j.getJobId()] = j;
	list<string> &job_list_now = partition_job_map[par];
	if(job_list_now.empty() && partition_soft_job_map.count(par)!=0)
	{
		//if there are soft real-time jobs executing, remove the soft real-time job immediately.
		insert_soft_job(job_full_map[partition_soft_job_map[par]]);
		partition_cpu_map.erase(par);
	}
	auto it = job_list_now.begin();
	for(;it!=job_list_now.end();it++)
	{
		if(j.getDDL()<job_full_map[*it].getDDL())
		{
			//insert job j into the list here
			job_list_now.insert(it, j.getJobId());
			break;
		}
	}	
	if(it==job_list_now.end())
		job_list_now.insert(it, j.getJobId());
}

void Scheduler::update_par_edf(string par)
{
	string pcpu_now = partition_cpu_map[par];
	if(!partition_job_map[par].empty())
	{
		Job &j = job_full_map[partition_job_map[par].front()];
		global_send_connections[pcpu_now]->sendInfo(j.wrap_info());
	}
}

//notice: semi_add_job will update the value of partitions
void Scheduler::semi_add_job(Job &j)
{
	//solves the soft real-time job insertion first
	if(!j.isHardRT())
	{
		if(waiting_jobs.empty())
		{
			//if no soft real-time jobs are in the waiting_jobs list
			//check all partitions to see whether they are idle
			auto it_par = sorted_partitions.begin();
			for(;it_par!=sorted_partitions.end();it_par++)
			{
				if(partition_job_map[*it_par].empty() && partition_soft_job_map.count(*it_par)==0)
				{
					partition_soft_job_map[*it_par] = j.getJobId();
					j.setPartitionId(*it_par);
					job_full_map[j.getJobId()] = j;
					global_send_connections[partition_cpu_map[*it_par]]->sendInfo(j.wrap_info());
					return;
				}
			}
		}
		insert_soft_job(j);

		return;
	}

	//this is for hard real-time jobs only
	//find a partition that suits based on best fit
	auto dur = duration_cast<microseconds> (j.getDDL() - system_clock::now());
	double ms_ddl = dur.count()/(double)1000;
	double density = j.getComputationTime()/ms_ddl;
	job_densities[j.getJobId()] = density;
	string par_bf_id = best_fit(j, density);
	if(par_bf_id!="")
	{

		//this applies to any job (hard or not hard real-time) that can be fit with BF.
		//check whether swap can be helpful
		//check 1. job j swap with the whole job set in the partition
		//2. job j swap with one single job
		// in order to make the aaf_left in the partition smaller than epsilon
		double epsilon = 0.00001;
		bool swapped = false;
		for(auto it = partitions.begin();it!=partitions.end();it++)
		{
			if(it->first == par_bf_id)
				continue;
			Partition *par_now = it->second;
			
			double density_set_now = par_now->getAAF() - par_now->getAAFLeft();
			//check the whole job set first
			//there is no need to swap if the density of the whole job set is larger than the density.
			if(density_set_now < density && partitions[par_bf_id]->getAAFLeft() >= density_set_now 
				&& par_now->getAAF()>density && par_now->getAAF() - density <= epsilon)
			{
				//insert the job list in par_now into partition par_bf_id
				list<string> &jobs_par_now = partition_job_map[it->first];
				if(!jobs_par_now.empty())
				{
					for(auto it_job = jobs_par_now.begin();it_job!=jobs_par_now.end();it_job++)
					{
						//info stored in the job instance is also changed
						add_job_to_partition(par_bf_id, job_full_map[*it_job]);
					}
					update_par_edf(par_bf_id);
					partitions[par_bf_id]->setAAFLeft(partitions[par_bf_id]->getAAFLeft() - density_set_now);
					//clear the list in partition par_now
					partition_job_map[par_now->getID()].clear();
				}
				else if(partition_soft_job_map.count(it->first)!=0)
				{
					insert_soft_job(job_full_map[partition_soft_job_map[it->first]]);
					partition_soft_job_map.erase(it->first);
				}

				//add j into par_now
				add_job_to_partition(par_now->getID(), j);
				par_now->setAAFLeft(par_now->getAAF() - density);
				update_par_edf(par_now->getID());
				swapped = true;
				break;
			}
			//check one single job swap with j here
			list<string> &jobs_par_now = partition_job_map[it->first];
			for(auto it_job = jobs_par_now.begin();it_job!=jobs_par_now.end();it_job++)
			{
				double part_aaf_left = par_now->getAAFLeft()+job_densities[*it_job];
				if(job_densities[*it_job]<density && part_aaf_left>=density && part_aaf_left - density <= epsilon)
				{
					//swap the job *it_job and j
					//put it_job into partition par_bf_id
					add_job_to_partition(par_bf_id, job_full_map[*it_job]);
					jobs_par_now.erase(it_job);
					update_par_edf(par_bf_id);
					//put j into par_now
					add_job_to_partition(par_now->getID(), j);
					update_par_edf(par_now->getID());
					//update partition's parameters
					par_now->setAAFLeft(part_aaf_left - density);
					partitions[par_bf_id]->setAAFLeft(partitions[par_bf_id]->getAAFLeft() - job_densities[*it_job]);
					swapped = true;
					break;
				}
			}
			if(swapped)
				break;
		}
		if(!swapped)
		{
			//if not swapped, just insert j into partition par_bf_id
			add_job_to_partition(par_bf_id, j);
			partitions[par_bf_id]->setAAFLeft(partitions[par_bf_id]->getAAFLeft() - density);
			update_par_edf(par_bf_id);
		}
		/*
		add_job_to_partition(par_bf_id, j);
		partitions[par_bf_id]->setAAFLeft(partitions[par_bf_id]->getAAFLeft() - density);
		update_par_edf(par_bf_id);
		std::cout << j.getJobId() << " is assigned to "<<j.getPartitionId()<<std::endl;
		task_partition_map[j.getTaskId()] = par_bf_id;*/
		return;
	}
	else if(par_bf_id=="")
	{
		//std::cout << "This job is not schedulable by BF. "<<std::endl;
		//first find out the largest aaf left
		list<string> par_sorted_aaf_left;
		for(auto it_par = partitions.begin(); it_par!=partitions.end();it_par++)
		{
			Partition *p_now = it_par->second;
			auto it_list = par_sorted_aaf_left.begin();
			for(;it_list!=par_sorted_aaf_left.end();it_list++)
			{
				if(partitions[*it_list]->getAAFLeft() <= p_now->getAAFLeft())
				{
					par_sorted_aaf_left.insert(it_list, p_now->getID());
					break;
				}
			}
			if(it_list==par_sorted_aaf_left.end())
				par_sorted_aaf_left.insert(it_list, p_now->getID());

		}
		if(partitions[par_sorted_aaf_left.front()]->getAAFLeft()<=0)
		{
			//no capacity left at all, simply choose the one with largest capacity left for j
			add_job_to_partition(par_sorted_aaf_left.front(), j);
			update_par_edf(par_sorted_aaf_left.front());
			Partition *temp_p = partitions[par_sorted_aaf_left.front()];
			temp_p->setAAFLeft(temp_p->getAAFLeft() - density);
			//std::cout << j.getJobId() << " is assigned to "<<j.getPartitionId()<<" which makes it unschedulable"<<std::endl;
			task_partition_map[j.getTaskId()] = j.getPartitionId();
			return;
		}
		//if there exists some more space, try to see whether swap can help
		double smallest_hole = 100000;
		string swap_par = "";
		int swap_type = 0;//0 means no swap policy found, 1 means swap the whole job set, 2 means swap a certain job
		string job_name = "";//only valid when swap_type = 2
		for(auto it_par = partitions.begin();it_par!=partitions.end();it_par++)
		{
			//check job set and single jobs here to see whether the swap may help.
			//conditions for swapping
			//1. the total densities of the job set is smaller than density
			//2. the job set swapped out can be accommodated somewhere
			//3. after swapping, job j can be accommodated in this partition
			//3. The hole created by job j is the smallest
			Partition *p_now = it_par->second;
			//check the whole job set in p_now first
			double density_set_now = p_now->getAAF() - p_now->getAAFLeft();
			if(density_set_now < density && density_set_now <= partitions[par_sorted_aaf_left.front()]->getAAFLeft()
				&& p_now->getAAF() >= density && p_now->getAAF() - density < smallest_hole)
			{
				swap_type = 1;
				swap_par = it_par->first;
				smallest_hole = p_now->getAAF() - density;
			}

			//check single job in p_now 
			list<string> &job_list_now = partition_job_map[it_par->first];
			for(auto it_job = job_list_now.begin();it_job!=job_list_now.end();it_job++)
			{
				//similarly check the conditions above
				double density_job_now = job_densities[*it_job];
				if(density_job_now < density && density_job_now <= partitions[par_sorted_aaf_left.front()]->getAAFLeft()
					&& p_now->getAAFLeft() + density_job_now >= density && p_now->getAAFLeft() + density_job_now - density < smallest_hole)
				{
					swap_type = 2;
					smallest_hole = p_now->getAAFLeft() + density_job_now - density;
					job_name = *it_job;
					swap_par = it_par->first;
				}
			}

		}

		if(swap_type == 0)
		{
			//no valid swap is found, then simply put it into the partition with largest capacity left
			Partition *temp_p = partitions[par_sorted_aaf_left.front()];
			add_job_to_partition(par_sorted_aaf_left.front(), j);
			update_par_edf(par_sorted_aaf_left.front());
			temp_p->setAAFLeft(temp_p->getAAFLeft() - density);
			//std::cout << j.getJobId() << " is assigned to "<<j.getPartitionId()<<" which makes it unschedulable"<<std::endl;
			task_partition_map[j.getTaskId()] = j.getPartitionId();
			return;
		}
		else if(swap_type == 1)
		{
			//migrate the job set in the given partition based on BF
			//first find which partition the job set would be migrated to
			Partition *swap_par_ptr = partitions[swap_par];
			double density_set_now = swap_par_ptr->getAAF() - swap_par_ptr->getAAFLeft();
			string target_par = par_sorted_aaf_left.front();
			for(auto it_par = par_sorted_aaf_left.begin();it_par!=par_sorted_aaf_left.end();it_par++)
			{
				if(partitions[*it_par]->getAAFLeft() >= density_set_now)
				{
					target_par = *it_par;
				}
				else
				{
					break;
				}
			}
			//put the whole job set into target_par
			list<string> &job_set_list = partition_job_map[swap_par];
			for(auto it_job = job_set_list.begin(); it_job != job_set_list.end();it_job++)
			{
				add_job_to_partition(target_par, job_full_map[*it_job]);
			}
			update_par_edf(target_par);
			Partition *temp_p = partitions[target_par];
			temp_p->setAAFLeft(temp_p->getAAFLeft() - density_set_now);

			//first clear the job set
			partition_job_map[swap_par].clear();
			//then put j into swap_par
			add_job_to_partition(swap_par, j);
			update_par_edf(swap_par);
			swap_par_ptr->setAAFLeft(swap_par_ptr->getAAF() - density);
			//std::cout << j.getJobId() << " is assigned to "<<j.getPartitionId()
			//<<" by swapping with a set of job."<<std::endl;
			task_partition_map[j.getTaskId()] = j.getPartitionId();
			return;
		}
		else
		{
			//migrate the job selected
			double job_density = job_densities[job_name];
			//find a partition that can fit the job
			string target_par = par_sorted_aaf_left.front();
			for(auto it_par = par_sorted_aaf_left.begin();it_par!=par_sorted_aaf_left.end();it_par++)
			{
				if(partitions[*it_par]->getAAFLeft() >= job_density)
				{
					target_par = *it_par;
				}
				else
				{
					break;
				}
			}
			add_job_to_partition(target_par, job_full_map[job_name]);
			update_par_edf(target_par);
			Partition *temp_p = partitions[target_par];
			temp_p->setAAFLeft(temp_p->getAAFLeft() - job_density);
			//remove the job from the former partition
			partition_job_map[swap_par].remove(job_name);
			//put j into the selected swap_par
			Partition *swap_par_ptr = partitions[swap_par];
			add_job_to_partition(swap_par, j);
			update_par_edf(swap_par);
			swap_par_ptr->setAAFLeft(swap_par_ptr->getAAFLeft() + job_density - density);
			//std::cout << j.getJobId() << " is assigned to "<<j.getPartitionId()
			//<<" by swapping one job. "<<std::endl;
			task_partition_map[j.getTaskId()] = j.getPartitionId();
			return;
		}

	}


}

//notice: semi_remove_job will update the value of partitions
void Scheduler::semi_remove_job(string job_name)
{
	Job &j = job_full_map[job_name];
	string par = j.getPartitionId();
	list<string> &job_list_now = partition_job_map[par];
	for(auto it = job_list_now.begin();it!=job_list_now.end();it++)
	{
		if(*it == j.getJobId())
		{
			job_list_now.erase(it);
			break;
		}
	}
	//only updates the partition's parameter when job j is not a hard real-time job
	if(!j.isHardRT())
	{
		partitions[par]->setAAFLeft(partitions[par]->getAAFLeft()+job_densities[j.getJobId()]);
		job_densities.erase(j.getJobId());
	}
	job_full_map.erase(j.getJobId());
}

void Scheduler::insert_soft_job(Job &j)
{
	j.setPartitionId("");
	job_full_map[j.getJobId()] = j;
	auto it_job = waiting_jobs.begin();
	for(;it_job!=waiting_jobs.end();it_job++)
	{
		if(job_full_map[*it_job].getDDL()>j.getDDL())
		{
			waiting_jobs.insert(it_job, j.getJobId());
			break;
		}
	}
	if(it_job==waiting_jobs.end())
	{
		waiting_jobs.insert(it_job, j.getJobId());
	}
}

#endif