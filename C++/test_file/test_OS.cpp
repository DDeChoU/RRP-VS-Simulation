/*#include<iostream>
#include "Socket_Conn.h"
#include <unistd.h>
using namespace std;

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

int main()
{

	//test sock

	int pid = fork();
	
	
	if(pid<0)
		cout<<"Fork error!"<<endl;
	else if(pid==0)
	{
		cout<<"Child progress in"<<endl;
		sleep(1);
		Socket_Conn a(20, false);
		keepSending(a);
	}
	else
	{
		Socket_Conn b(20, true);
		int counter = 0;
		while(counter<5)
		{
			sleep(1);
			vector<string> this_time = b.receiveInfo();
			for(int i=0;i<this_time.size();i++)
			{
				cout<<this_time.at(i)<<endl;
			}
			counter++;
		}
	}

	return 0;
}*/


#include<iostream>
#include "OS_Simulator.h"
#include <unistd.h>
using namespace std;


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
		system_clock::time_point a = system_clock::now();
		os.generate_jobs(a, 20);

	}
	else
	{
		Socket_Conn receiver(20, true);
		int counter = 0;
		while(counter<=10)
		{
			sleep(1);
			vector<string> this_receive = receiver.receiveInfo();
			for(int i=0;i<this_receive.size();i++)
			{
				//cout<<this_receive.at(i)<<endl;
				Job j(this_receive.at(i));
				cout<<j.print_info()<<endl;
			}
			counter++;
		}
	}
}
