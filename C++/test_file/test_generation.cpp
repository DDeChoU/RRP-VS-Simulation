#include "Generation.h"
#include <iostream>
using std::cout;
using std::endl;


int main()
{
	Generation g;
	double target = 4.5;
	//partition generation tested
	
	vector<Partition> p = g.generate_partitions(4.5);
	for(int i=0;i<p.size();i++)
	{
		cout<<p[i].printInfo()<<endl;
	}
	

	//task generation tested
	vector<Task> ts = g.generate_tasks(target, true, 0.5, 0.7, 0.5);
	for(int i=0;i<ts.size();i++)
	{
		cout<<ts[i].printInfo()<<endl;
	}
	return 0;
}