#ifndef PCPU_H
#define PCPU_H
#include <chrono>
#include <math.h>
#include <iostream>
#include <list>
#include <set>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <string>
#include <stdlib.h>

#include "Socket_Conn.h"
#include "TaskSlice.h"
#include "Partition.h"

using namespace std::chrono;
using std::cout;
using std::endl;
using std::list;
using std::set;
using std::vector;
using std::unordered_map;
using std::string;

//circular linked list node 
struct job_node
{
	Job j;
	job_node* next;
	job_node* prev;
	job_node(Job &a):j(a)
	{
		next = prev = nullptr;
	}
};

struct time_slice
{
	Partition *p;
	time_slice *next;
	time_slice *prev;
	time_slice(Partition *p_now):p(p_now)
	{
		next = prev = nullptr;
	}
};

bool compare_partitions(const Partition *a, const Partition *b)
{
	return a->getAAF()>b->getAAF();
}

class PCPU
{
private:
	string pcpu_id;
	time_slice *t_table_root;
	time_slice *t_now;
	int hyper_period;
	unordered_map<string, Partition *> partitions;
	//job_node *root;
	//job_node *node_now;
	//vector<string> time_par_table;

public:
	PCPU(string id);

	void run_pcpu(int port);
	
	//schedule partitions based on the factor
	void partition_single(list<Partition *> partition_set);

	//calculate the hyper_period of this pcpu
	void calculate_hp(const list<Partition *> &partition_set);

	string showTTable();

	string getID() const{return pcpu_id;}

	~PCPU();

private:

	//can modify that into LLF or EDF, now is FIFO, can be optimized by directly taking string in
	/*
	void insert_job(Job &a);
	//remove node_now
	void remove_job(job_node *this_node);*/

		//execute execution_time milliseconds.
	void execute(int execution_time);

	//handles the precision
	double approximateValue(double value);

	//calculate the least common multiple of two integers
	int lcm(int t1, int t2);


	void insert_time_slice(Partition *p);

};
PCPU::PCPU(string id)
{
	pcpu_id = id;
	t_table_root = nullptr;
	t_now = nullptr;
	hyper_period = 1;
	//cout<<"Successfully constructed."<<endl;
}

//execute execution_time milliseconds.
void PCPU::execute(int execution_time)
{
	system_clock::time_point time_start = system_clock::now();
	while(true)
	{
		system_clock::time_point time_now = system_clock::now();
		auto dur = duration_cast<microseconds>(time_now - time_start);
		double ms = dur.count()/(double)1000;
		if(ms>=execution_time)
			return;
	}
}

void PCPU::run_pcpu(int port)
{
	
	//build up two pipes with the scheduler first
	Socket_Conn send_pipe(port, false);
	Socket_Conn recv_pipe(port+1, true); 
	//cout<<"RUN_PCPU connected!"<<endl;
	bool poweroff = false;
	int time_slice_length = 10;//the time slice length now is 10 milliseconds.
	while(true)
	{
		if(t_now!=nullptr)
		{
			t_now = t_now->next;
			if(t_now->p==nullptr)
			{
				//cout<<"Running an idle time slice"<<endl;
				execute(time_slice_length);
			}
			else
			{
				//cout<<"Running a non-idle time slice"<<endl;
				Job j_now;
				if(!t_now->p->schedule(j_now))
				{
					execute(time_slice_length);
				}
				else
				{
					execute(time_slice_length);
					double exe_time = j_now.getComputationTime();
					j_now.setComputationTime(exe_time-time_slice_length);
					if(exe_time - time_slice_length>=time_slice_length)
						t_now->p->insertJob(j_now);
					//else
						//std::cout << j_now.getJobId() << " is done and removed."<<std::endl;

					system_clock::time_point time_now = system_clock::now();
					//send a report here
					TaskSlice ts(j_now.getJobId(), j_now.getTaskId(), time_slice_length, j_now.getComputationTime(), j_now.isHardRT(), time_now, j_now.getDDL());
					//cout<<"In CPU: "<<ts.wrap_info()<<std::endl;
					send_pipe.sendInfo(ts.wrap_info());
				}
				
			}
		}
		
		
		//hanld jobs received from the scheduler, insert into the right partition
		vector<string> received_jobs = recv_pipe.receiveInfo();
		//std::cout<<received_jobs.size()<<std::endl;
		for(int i=0;i<received_jobs.size();i++)
		{
			//cout<<"One job received."<<std::endl;
			if(received_jobs.at(i).find("Poweroff")!=-1)
			{
				poweroff = true;
				break;
			}
			
			Job j(received_jobs.at(i));
			//cout<<"Received in pcpu: "<<j.print_info()<<endl;
			if(j.getPartitionId()=="")
			{
				std::cout<<j.getPartitionId()<<std::endl;
				std::cout<<"Job "<<j.getJobId()<<" does not have a partition."<<std::endl;
			}
			else
			{
				partitions[j.getPartitionId()]->insertJob(j);
			}
			
			
		}
		if(poweroff)
		{
			//std::cout<<"Cleaning objects"<<endl;
			break;
		}

	}
	//std::cout<<"PCPU #"<<pcpu_id<<" shutting down"<<endl;
	
}



void PCPU::insert_time_slice(Partition *p)
{
	time_slice *temp = new time_slice(p);
	/* outputs used for debugging.
	cout<<a.getJobId()<<": ";
	system_clock::time_point t_now = system_clock::now();
	stringstream ss;
	time_t et = system_clock::to_time_t(t_now);
	const auto nowMS = duration_cast<milliseconds>(t_now.time_since_epoch())%1000;
	ss<<std::put_time(localtime(&et), "%F %T")<<"."<<std::setfill('0')<<std::setw(3)<<nowMS.count();
	cout<<ss.str()<<endl;
	*/

	if(t_table_root==nullptr)
	{
		t_table_root = temp;
		temp->next = t_table_root;
		temp->prev = t_table_root;
		t_now = t_table_root;

	}
	else
	{
		time_slice *iter = t_table_root;
		while(iter->next!=t_table_root)
		{
			iter = iter->next;
		}
		iter->next = temp;
		temp->prev = iter;
		temp->next = t_table_root;
		t_table_root->prev = temp;
	}
}

//remove all time_slice
PCPU::~PCPU()
{
	cout<<"Cleaning the object of pcpu."<<std::endl;
	time_slice *now = t_table_root;
	if(now==nullptr)
		return;
	while(now->next!=now)
	{
		//delete node now
		now->prev->next = now->next;
		now->next->prev = now->prev;
		time_slice *temp = now->next;
		delete now;
		now = temp;
	}
	delete now;
	t_table_root = nullptr;
	t_now = nullptr;
}


double PCPU::approximateValue(double value)
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

//needs to be modified!!!
void PCPU::calculate_hp(const list<Partition *> &partition_set)
{
	int hp = 1;
	//hyper period cannot be done well.
	for(auto it = partition_set.begin();it!=partition_set.end(); it++)
	{
		int period_now = (*it)->getAAFDown();
		hp = lcm(hp, period_now);
	}

	hyper_period = hp;
}


int PCPU::lcm(int temp1, int temp2)
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

void PCPU::partition_single(list<Partition *> partition_set)
{
	calculate_hp(partition_set);
	vector<int> avail_timeslices;
	for(int i=0;i<hyper_period;i++)
		avail_timeslices.push_back(i);
	//sort the partition_set
	partition_set.sort(compare_partitions);
	vector<int> qs;
	int qs_total = 0;
	for(auto it = partition_set.begin(); it!=partition_set.end();it++)
	{
		int q_now = hyper_period/(*it)->getAAFDown()*(*it)->getAAFUp();
		qs_total += q_now;
		qs.push_back(q_now);
		partitions[(*it)->getID()] = (*it);
	}
	vector<Partition *> t_list(hyper_period, nullptr);//this is the time-slice table in vector's form
	int i=0;
	for(auto it = partition_set.begin();it!=partition_set.end();it++)
	{
		int hp_now = avail_timeslices.size();
		//std::cout<<"Allocating "<<(*it)->getID()<<std::endl;
		//assign time slices to the first partition, maintain a set that could be used when check delta
		set<int> occupied_time_index;
		for(int k=0;k<qs[i];k++)
		{
			int index_now = (int)(floor(k*hp_now/qs[i]))%hp_now;
			occupied_time_index.insert(index_now);
			//std::cout<<"Assigning "<<avail_timeslices[index_now]<<" to "<<(*it)->getID()<<std::endl;
			//if(t_list[avail_timeslices[index_now]]!=nullptr)
			//{
				//std::cout<<"Not null pointer!?   "<<avail_timeslices[index_now]<<"  ";
				//std::cout<<t_list[avail_timeslices[index_now]]->getID()<<std::endl;
			//}
			t_list[avail_timeslices[index_now]] = *it;//what if this place is not nullptr now
		}
		qs_total -= qs[i];

		vector<int> temp;
		for(int i=0;i<hp_now;i++)
		{
			if(occupied_time_index.count(i)==0)
				temp.push_back(avail_timeslices[i]);
		}
		sort(temp.begin(), temp.end(), std::less<int>());
		avail_timeslices = temp;

		i++;
	}

		

	//check the regularity and the time_slice table here
	/*
	int counter = 1;
	unordered_map<string, int> t_num;
	for(auto it = partition_set.begin(); it!= partition_set.end();it++)
		t_num[(*it)->getID()] = 0;
	for(int i=0;i<t_list.size();i++)
	{
		std::cout<<counter<<": ";
		if(t_list.at(i)==nullptr)
			std::cout<<"IDLE"<<std::endl;
		else
		{
			std::cout<<t_list.at(i)->getID()<<std::endl;
			t_num[t_list.at(i)->getID()] ++;
		}
		for(auto it2 = partition_set.begin();it2!=partition_set.end();it2++)
		{
			std::cout<<"("<<(*it2)->getID()<<")";
			double s_regularity = t_num[(*it2)->getID()] - counter*((*it2)->getAAF());
			std::cout<<s_regularity<<"; ";

		}
		std::cout<<std::endl;
		counter++;
		
	}
	*/
	//assign t_list to the real timeslice-domain table.
	for(int i=0;i<t_list.size();i++)
	{
		Partition *p_now = t_list[i];
		insert_time_slice(p_now);
	}
	//the insertion needs further validation


}
string PCPU::showTTable()
{
	time_slice *now = t_table_root;
	string result ="";
	int counter = 1;
	do
	{
		result += std::to_string(counter);
		result += ": ";
		if(now->p!=nullptr)
			result += now->p->getID();
		else
			result += "IDLE";
		result += "\n";
		counter ++;
		now = now->next;
	} while(now!=t_table_root);
	return result;

}

#endif