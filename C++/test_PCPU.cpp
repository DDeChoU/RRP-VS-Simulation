
#include<iostream>
#include "OS_Simulator.h"
#include <unistd.h>
#include "PCPU.h"
using namespace std;

//test the connection to pcpu first

void keepSending(Socket_Conn &f)
{
	for(int i=0;i<1000;i++)
	{
		string to_send = to_string(i)+" test string. Use long sentence to test data loss under multiple send before one recv\n";
		f.sendInfo(to_send);
		//sleep(5);
	}
	return;
}
/*
int main()
{
	int pid = fork();
	if(pid==0)
	{
		PCPU p("1");
		sleep(1);
		p.run_pcpu(20);
	}
	else
	{
		Socket_Conn com_pipe(20, true);
		sleep(2);
		com_pipe.sendInfo("Test 1\n");
		com_pipe.sendInfo("Test 2\n");
		//keepSending(com_pipe);
		//cout<<"Shutting down!"<<endl;
		com_pipe.sendInfo("Poweroff\n");
		sleep(5);

		
		int counter = 0;
		while(counter<10)
		{
			sleep(1);
			vector<string> this_time = com_pipe.receiveInfo();
			for(int i=0;i<this_time.size();i++)
			{
				cout<<this_time.at(i)<<endl;
			}
			counter++;
		}
		
		
	}
}
*/

string printTime(system_clock::time_point a)
{
		stringstream ss;
		time_t et = system_clock::to_time_t(a);
		const auto nowMS = duration_cast<milliseconds>(a.time_since_epoch())%1000;
		ss<<std::put_time(localtime(&et), "%F %T")<<"."<<std::setfill('0')<<std::setw(3)<<nowMS.count();
		string result = ss.str();
		return result;
}

int main()
{
	
	vector<Task> task_list;
	Task t1(10, 3000, 3000, 5, to_string(1), true, false);
	task_list.push_back(t1);
	Task t2(20, 1000, 1000, 100, to_string(2), true, false);
	task_list.push_back(t2);
	Task t3(50, 1200, 1200, 150, to_string(3), true, false);
	task_list.push_back(t3);
	Task t4(5, 1000, 1000, 1, to_string(4), true, false);
	task_list.push_back(t4);
	OS_Simulator os(task_list);
	
	
	int pid = fork();
	if(pid==0)
	{
		sleep(1);
		//system_clock::time_point a = system_clock::now();
		//cout<<"Start point: "<<printTime(a)<<endl;
		os.generate_jobs(20);

	}
	else
	{
		Socket_Conn receiver(20, true);
		int ppid = fork();
		if(ppid==0)
		{
			PCPU p("1");
			p.run_pcpu(1025);
		}
		Socket_Conn conn(1025, true);
		system_clock::time_point start_t = system_clock::now();
		while(true)
		{
			system_clock::time_point temp_t = system_clock::now();
			auto dur = temp_t - start_t;
			double ms = dur.count()/(double)1000;
			if(ms>=20000)
				break;
			vector<string> this_receive = receiver.receiveInfo();
			for(int i=0;i<this_receive.size();i++)
			{
				//cout<<this_receive.at(i)<<endl;
				Job j(this_receive.at(i));
				//cout<<j.print_info()<<endl;
				//cout<<"Sending out";
				conn.sendInfo(j.wrap_info());

			}
			
			vector<string> task_slices = conn.receiveInfo();
			for(int i=0;i<task_slices.size();i++)
			{
				//cout<<"In test, "<<task_slices.at(i)<<endl;
				TaskSlice ts(task_slices.at(i));
				cout<<ts.getJobID()<<" is executed for "<<ts.getTaskSliceLen()<<" and needs "
					<<ts.getTimeLeft()<<" more time. ";
				if(ts.isDone())
					cout<<" Is finished ";
				if(ts.isOnTime())
					cout<<" On time.";
				else
					cout<<" Not on time.";
				//if(!ts.isOnTime())
				//{
					cout<<endl;
					cout<<printTime(ts.getDDL())<<endl;
					cout<<printTime(ts.getTaskSliceEnd())<<endl;

				//}
				cout<<endl;
			}

		}
		cout<<"Shutting down!"<<endl;
		conn.sendInfo("Poweroff\n");
		receiver.sendInfo("Poweroff\n");
		//wait long enough to make sure the process is killed properly
		sleep(10);
	}


}
