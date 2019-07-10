#ifndef PCPU_H
#define PCPU_H
#include <chrono>
#include <math.h>
#include "Socket_Conn.h"
#include "TaskSlice.h"
#include <iostream>

using namespace std::chrono;
using std::cout;
using std::endl;

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

class PCPU
{
private:
	string pcpu_id;
	job_node *root;
	job_node *node_now;
	//vector<string> time_par_table;

public:
	PCPU(string id);

	void run_pcpu(int port);
	
private:

	//can modify that into LLF or EDF, now is FIFO, can be optimized by directly taking string in
	void insert_job(Job &a);
	//remove node_now
	void remove_job(job_node *this_node);

		//execute execution_time milliseconds.
	void execute(int execution_time);

	double approximateValue(double value);

};
PCPU::PCPU(string id)
{
	pcpu_id = id;
	root = nullptr;
	node_now = nullptr;
	//cout<<"Successfully constructed."<<endl;
}

//execute execution_time milliseconds.
void PCPU::execute(int execution_time)
{
	system_clock::time_point time_start = system_clock::now();
	while(true)
	{
		system_clock::time_point time_now = system_clock::now();
		auto dur = time_now - time_start;
		double ms = dur.count()/(double)1000;
		if(ms>=execution_time)
			return;
	}
}

void PCPU::run_pcpu(int port)
{
	//build up com_pipeections with the scheduler first
	Socket_Conn com_pipe(port, false);
	//cout<<"RUN_PCPU connected!"<<endl;
	bool poweroff = false;
	while(true)
	{
		if(node_now)
		{
			//cout<<"Trying to execute"<<endl;
			//if there exists some vcpus in the pcpu list, then proceed to execute next one.
			node_now = node_now->next;
			Job *job_now = &node_now->j;
			int basic_ts = 10;
			double time_left = job_now->getComputationTime();

			
			if(time_left>basic_ts)
			{
				//cout<<"Executing a time slice"<<endl;
				//if the task still needs more than basic_ts time, execute basic_ts.
				execute(basic_ts);
				job_now->setComputationTime(time_left - basic_ts);
				//send execution info for documentation
				//build a TaskSlice
				TaskSlice ts(job_now->getJobId(), job_now->getTaskId(), basic_ts, job_now->getComputationTime(), job_now->isHardRT(), system_clock::now(), job_now->getDDL());
				com_pipe.sendInfo(ts.wrap_info());
			}
			else
			{
				//cout<<"Executing what is left"<<endl;
				//else execute the time left.
				execute(time_left);
				job_now->setComputationTime(0);
				TaskSlice ts(job_now->getJobId(), job_now->getTaskId(), time_left, job_now->getComputationTime(), job_now->isHardRT(), system_clock::now(), job_now->getDDL());
				com_pipe.sendInfo(ts.wrap_info());
				//send the accomplish info

				//update the job list
				job_node *tmp = node_now;
				node_now = node_now->prev;
				remove_job(tmp);
			}
		}
		//hanld jobs received from the scheduler.
		vector<string> received_jobs = com_pipe.receiveInfo();
		for(int i=0;i<received_jobs.size();i++)
		{
			if(received_jobs.at(i).find("Poweroff")!=-1)
			{
				poweroff = true;
				break;
			}
			
			Job j(received_jobs.at(i));
			//cout<<"Received in pcpu: "<<received_jobs.at(i)<<endl;
			insert_job(j);
		}
		if(poweroff)
		{
			cout<<"Cleaning objects"<<endl;
			while(root!=nullptr)
			{
				remove_job(root);
			}
			break;
		}

	}
	cout<<"PCPU #"<<pcpu_id<<" shutting down"<<endl;

}

void PCPU::insert_job(Job &a)
{
	job_node *temp = new job_node(a);
	/* outputs used for debugging.
	cout<<a.getJobId()<<": ";
	system_clock::time_point t_now = system_clock::now();
	stringstream ss;
	time_t et = system_clock::to_time_t(t_now);
	const auto nowMS = duration_cast<milliseconds>(t_now.time_since_epoch())%1000;
	ss<<std::put_time(localtime(&et), "%F %T")<<"."<<std::setfill('0')<<std::setw(3)<<nowMS.count();
	cout<<ss.str()<<endl;
	*/
	if(root==nullptr)
	{
		root = temp;
		temp->next = root;
		temp->prev = root;
		node_now = root;

	}
	else
	{
		job_node *iter = root;
		while(iter->next!=root)
		{
			iter = iter->next;
		}
		iter->next = temp;
		temp->prev = iter;
		temp->next = root;
	}
}
//remove node_now
void PCPU::remove_job(job_node *this_node)
{
	if(this_node==root)
	{
		root = nullptr;
		node_now = root;
		delete this_node;
	}
	else
	{
		this_node->prev->next = this_node->next;
		this_node->next->prev = this_node->prev;
		node_now = this_node->prev;
		delete this_node;
	}
}

#endif