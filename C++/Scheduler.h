#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "Task.h"
#include "Job.h"
#include "PCPU.h"
#include "Partition.h"
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
using std::string;
using std::unordered_map;
using std::list;
using std::priority_queue;
using std::set;
using std::vector;
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
	unordered_map<string, list<Partition *> > time_partition_tables;

	//partition_cpu_map records which pcpu a partition is mapped to.
	unordered_map<string, string> partition_cpu_map;

	//task_partition_map records which partition the task is executed last time
	unordered_map<string, string> task_partition_map;

	//domain_partition_map includes a map from domain to partition
	//for the simplified version simulation, only one key is stored.
	unordered_map<string, list<string> > domain_partition_map;

	unordered_map<string, int> hyper_periods;//all aafs are constrained to 3-digit-long after the decimal point.
	//initialize some parameters used to record the task execution 
	//to be added

public:
	//the constructor takes in the number of pcpus
	Scheduler(int p_num);

	//run simulates the schedule and should return a schedule in the form of string.
	//pass the output redirection to run through ostream &out.
	void run(vector<Task> taskList, vector<Partition> partitionList, std::ostream &out);

private:
	bool MulZ(); //allocate partitions to pcpu, if not schedulable, return false.
	bool Magic7(string pcpu);//build the time slice table of a certain pcpu based on the allocation returned by MulZ
	vector<int> z_approx(double w, double n);//calculate the approximated availability factor

	//pass in a partition and returns the pcpu that partition should lie on.
	//this function needs to track the space left on a pcpu all the time.
	string mulZ_FFD_Alloc(Partition *p, unordered_map<string, double> &factors, unordered_map<string, double> &rests);

	struct cmp_par
	{
		bool operator()(const Partition *a, const Partition *b) const
		{
			return a->getAF()<b->getAF();
		}
	};


	double approximateValue(double value);

	//initialize the time-partition table in the given pcpu;
	void partition_single(string pcpu_id);

	//calculate the hyperperiod of corresponding pcpu.
	void calculate_hp(string pcpu_id, vector<Partition *> partition_set);
	//calculate the lcm of a and b
	int lcm(int a, int b);
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

}

vector<int> Scheduler::z_approx(double w, double n)
{
	int i = 1, j=0, m = 2;
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

	while(!pq.empty())
	{
		Partition *p_temp = pq.top();
		pq.pop();
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
	
	//run aaf for each pcpu to set up the time-partition tables.
	//the aaf of each partition is already calculated, no need to recalculate, invoke partition_single in pcpu here
	/* Test this after the fraction part is tested. */
	for(auto it = pcpus.begin();it!=pcpus.end();it++)
	{
		partition_single(it->first);

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
			std::cout<<"Final factor is: "<<r<<std::endl;
			return pcpu_id_now;
		}
		else if(rests[it->first]>=r)
		{
			vector<int> rfrac= z_approx(p->getAF(), factors[pcpu_id_now]);
			r = (double)rfrac[0]/rfrac[1];
			std::cout<<"Final factor is: "<<r<<std::endl;
			rests[pcpu_id_now] -= r;
			p->setAAF(r);
			p->setAAFFrac(rfrac[0], rfrac[1]);
			return pcpu_id_now;
		}
	}
	return "";
}

void Scheduler::run(vector<Task> taskList, vector<Partition> partition_list, std::ostream &out)
{
	//insert the partitions into variable 'partitions'
	for(int i=0;i<partition_list.size();i++)
	{
		partitions[partition_list[i].getID()] = &partition_list[i];
		//mind that if partition_list gets eliminated, the pointers in "partitions" will be invalid as well.
	}
	// map partitions to pcpu and maintain the time slice table here
	if(!MulZ())
	{
		std::cout<<"The partitions are not schedulable"<<std::endl;
		//put a log here saying something is wrong.
	}


}

double Scheduler::approximateValue(double value)
{
	double result = floor(value);
	if(value-result>0.99999)
		return result+1;
	if(value-result>0.49999 && value-result<0.5)
		return result+0.5;
	if(value-result>0 && value-result<0.00001)
		return result;
	return value;
}

void Scheduler::partition_single(string pcpu_id)
{
	//std::cout<<"Scheduling in "<<pcpu_id<<std::endl;
	//calculate the hyper_period for this pcpu first !!!!! 


	//list of partitions invovled in this pcpu
	vector<Partition *> partitions_now;
	unordered_map<string, double> aaf_left;


	for(auto it = partition_cpu_map.begin();it!=partition_cpu_map.end();it++)
	{
		if(it->second==pcpu_id)
		{
			partitions_now.push_back(partitions[it->first]);
			aaf_left[it->first] = partitions[it->first]->getAAF();
		}
	}
	calculate_hp(pcpu_id, partitions_now);
	int hyper_period = hyper_periods[pcpu_id];

	//initialize the time_partition_table for this pcpu
	vector<string> t_partition_table(hyper_period, "");
	//initialize available time slice set
	set<int, std::less<int> > avail_timeslices;
	for(int i=0;i<hyper_period;i++)
		avail_timeslices.insert(i);

	int par_num= partitions_now.size(), level = 0, firstAvailableTime = 0;

	//loop until all partitions are set
	while(par_num>0)
	{
		double w = 1/(pow(2, level));
		int num = floor(w*hyper_period);
		int interval = pow(2, level);
		bool end = false;
		//add a judgement telling whether interval is already too large
		if(w<=0.0001)
			break;
		//check each partition in the list
		for(int i=0;i<partitions_now.size();i++)
		{
			double aaf_now = aaf_left[partitions_now[i]->getID()];
			//std::cout<<aaf_now<<", "<<w<<std::endl;
			if(aaf_now>=w|| (approximateValue(abs(aaf_now-w))==0&&aaf_now>0))
			{
				//start allocating time slices to the partition the level can hold.
				for(int j=0;j<=num;j++)
				{
					int time_slice_now = firstAvailableTime + j*interval;
					if(time_slice_now>=hyper_period)
					{
						std::cout<<"Out of range: "<<time_slice_now<<std::endl;
						break;//out of range
					}
					else if(avail_timeslices.count(time_slice_now)==0)
					{
						std::cout<<"Conflicts at "<<time_slice_now<<std::endl;
						continue;//allocation conflicts
					}
					else
					{
						//allocate time_slice_now to partition P_i
						t_partition_table[time_slice_now] = partitions_now[i]->getID();
						//std::cout<<"Assigning "<<time_slice_now<<" to partition "<<partitions_now[i]->getID()<<std::endl;
						avail_timeslices.erase(time_slice_now);

					}

				}

				//consider the smallest is -1, using lower_bound can find the smallest in the set now.
				firstAvailableTime = *(avail_timeslices.lower_bound(-1));
				//std::cout<<"First available time now is: "<<firstAvailableTime<<std::endl;
				aaf_left[partitions_now[i]->getID()] -= w;
				std::cout<<partitions_now[i]->getID()<<" , "<<aaf_left[partitions_now[i]->getID()]<<","<<w<<endl;
				if(aaf_left[partitions_now[i]->getID()]<=0.001)
				{
					//std::cout<<partitions_now[i]->getID()<<" done."<<endl;
					par_num -=1;
				}
				//std::cout<<"Num of time slices left: "<<avail_timeslices.size()<<endl;
				if(avail_timeslices.empty())
				{
					end = true;
					break;
				}
			}
		}
		if(end)
			break;
		level++;
	}

	//put the strings in partitions_now into time_slice tables one after another.
	for(int i=0;i<hyper_period;i++)
	{
		time_partition_tables[pcpu_id].push_back(partitions[t_partition_table[i]]);
	}

	//print for test
	list<Partition *> &l_now = time_partition_tables[pcpu_id];
	//print out here
	std::cout<<"Time slice table for "<<pcpu_id<<" is: "<<std::endl;
	int counter = 0;
	unordered_map<string, int> t_num;
	for(auto it = partitions_now.begin();it!= partitions_now.end();it++)
		t_num[(*it)->getID()] = 0;
	for(int r_time = 0;r_time<100;r_time++)
	{
		for(auto it = l_now.begin();it!=l_now.end();it++)
		{	
			std::cout<<++counter<<": ";
			if(*it!=NULL)
			{
				t_num[(*it)->getID()]++;
				std::cout<<(*it)->getID()<<std::endl;
			}
			else
				std::cout<<"IDLE"<<std::endl;
			for(auto it2 = partitions_now.begin();it2!=partitions_now.end();it2++)
			{
				std::cout<<"("<<(*it2)->getID()<<")";
				double s_regularity = t_num[(*it2)->getID()] - counter*((*it2)->getAAF());
				std::cout<<s_regularity<<"; ";

			}
			std::cout<<std::endl;
			/*
			if(*it!=NULL)
				std::cout<<(*it)->getID()<<std::endl;
			else
				std::cout<<"IDLE"<<std::endl;
				*/
		}
	}


}

void Scheduler::calculate_hp(string pcpu_id, vector<Partition *> partition_set)
{
	int hp = hyper_periods[pcpu_id];

	/*hyper period cannot be done well.*/
	for(int i=0;i<partition_set.size();i++)
	{
		int period_now = partition_set[i]->getAAFDown();
		hp = lcm(hp, period_now);
	}
	//hp  = hp*10;
	
	//hp = 1024;//when the accuracy required is 0.001
	hyper_periods[pcpu_id] = hp;


}

int Scheduler::lcm(int temp1, int temp2)
{
	int a = std::max(temp1, temp2);
	int b = std::min(temp1, temp2);
	while(b!=0)
	{
		int t = b;
		b = a%b;
		a = t;
	}
	return temp1*temp2/a;
}
//TO DO:
//1. Get input: task_list and partition_list
//2. Run AAF/MulZ to allocate partitions
//3. Run PCPUs
//4. Run OS_Simulator
//5. Accomplish the dead loop that employs various scheduling policy (partitioned/global and etc.) 
#endif