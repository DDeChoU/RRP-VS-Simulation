#include<iostream>
#include "Socket_Conn.h"
#include <unistd.h>
using namespace std;

void keepSending(Socket_Conn &f)
{
	for(int i=0;i<10;i++)
	{
		
		string to_send = to_string(i)+" test string. Use long sentence to test data loss under multiple send before one recv\n";
		f.sendInfo(to_send);
		sleep(1);
	}
	return;
}

int main()
{

	//test sock

	int pid = fork();
	
	
	if(pid<0)
		cout<<"Fork error!"<<endl;
	else if(pid!=0)
	{
		cout<<"Child progress in"<<endl;
		//sleep(1);	
		Socket_Conn a(20, true);
		cout<<"Server sending"<<endl;
		keepSending(a);
		cout<<"All sent"<<endl;
		//sleep(10);
	}
	else
	{
		sleep(1);
		Socket_Conn b(20, false);
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
}