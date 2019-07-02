//#include "Partition.h"
#include "Job.h"
#include "Task.h"
#include "Socket_Conn.h"
#include<chrono>
#include<cstdint>
#include<iostream>
#include <stdlib.h>
#include <ctime> 
using namespace std;
using namespace std::chrono;
#ifndef OS_SIMULATOR_H
#define OS_SIMULATOR_H

class OS_Simulator
{

private:
	vector<Task> task_list;

public:

	OS_Simulator(vector<Task> tlist)
	{
		task_list = tlist;
	}

	void generate_jobs(int port)//to do, set each process binded to core automatically
	{
		//build the connection
		//send in the pipe and mark that this process is not a server
		//to be tested

		Socket_Conn com_pipe(port, false);

		//sort tasks in arrival time order
		sort(task_list.begin(), task_list.end());

		//initialize data structures
		vector<long> phases(task_list.size(), 0);//all phases are set to 0 at the beginning
		//phase indicates which job of a task is now;
		int counter = 0;//indicates the index of next possibly arriving task
		//when counter==task_list.size(), all have arrived

		system_clock::time_point start_point = system_clock::now();
		//cout<<"Start point: "<<printTime(start_point)<<endl;
		//generate jobs
		while(1)
		{
			system_clock::time_point time_now = system_clock::now();
			auto du = time_now - start_point;//by default duration is using microseconds
			double time_passed = du.count()/(double)1000;
			//if not all tasks have arrived
			if(counter<task_list.size())
			{
				double arr_time_now = task_list[counter].getArrTime();
				while(arr_time_now < time_passed )
				{
					counter++;
					//cout<<"Task "<<counter<<" arrives"<<endl;
					if(counter>=task_list.size())
					{
						//cout<<"All tasks have arrived!"<<endl;
						break;
					}
					arr_time_now = task_list[counter].getArrTime();
					//cout<<arr_time_now<<", "<<time_passed<<endl;
				}

			}

			for(int i=0;i<counter;i++)
			{
				Task *t_now = &task_list[i];

				if(time_passed>= t_now->getArrTime() + phases.at(i)*t_now->getPeriod())
				{
					//cout<<"Releasing jobs here"<<endl;
					long long du_ms = t_now->getArrTime()+phases.at(i)*t_now->getPeriod();
					duration<long long, milli> fromStart(du_ms);
					
					//initialize the job here

					if(true)//!t_now->isPeriodic())
					{
						srand((unsigned)time(NULL));
						int dice = rand()%2;
						if(dice==0)
						{
							continue;
						}
					}
					duration<long long, milli> rddl(t_now->getRelativeDdl());
					
					system_clock::time_point arb_ddl = start_point + fromStart + rddl;

					//send arb_ddl, WCET and id into the constructor
					Job j_now(t_now->getWCET(), arb_ddl, t_now->getID(), phases.at(i), t_now->isPeriodic());
					//send it through the socket.
					//cout<<j_now.wrap_info()<<endl;

					phases[i]++;
					com_pipe.sendInfo(j_now.wrap_info());

				}
			}
			bool poweroff = false;
			vector<string> end_signal = com_pipe.receiveInfo();
			for(int i=0;i<end_signal.size();i++)
			{
				if(end_signal[i].find("Poweroff")!=-1)
				{
					poweroff = true;
					break;
				}
			}
			if(poweroff)
				break;

		}
		cout<<"OS Simulator shutting down!"<<endl;

	}

private:
	string printTime(system_clock::time_point a)
	{
		stringstream ss;
		time_t et = system_clock::to_time_t(a);
		const auto nowMS = duration_cast<milliseconds>(a.time_since_epoch())%1000;
		ss<<std::put_time(localtime(&et), "%F %T")<<"."<<std::setfill('0')<<std::setw(3)<<nowMS.count();
		string result = ss.str();
		return result;
	}
};
#endif