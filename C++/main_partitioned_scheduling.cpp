#include "Scheduler.h"
#include "Generation.h"
using std::cout;

int main()
{
	Generation g;
	//set target_af = 5, pcpu = 8, load_ratio = 0.5, periodic_ratio = 1, 
	//highest density = largest availability factor, hard rt ratio = 0
	int pcpu_num = 8;
	Scheduler s(pcpu_num);
	double target_af_sum = 5;
	vector<Partition> ps = g.generate_partitions(target_af_sum);
	while(!s.set_partitions(ps))
	{
		cout<<"Resetting partitions.\n";
		ps.clear();
		ps = g.generate_partitions(target_af_sum);
	}
	double highest_af = 0;
	for(int i=0;i<ps.size();i++)
	{
		if(ps.at(i).getAF()>highest_af)
			highest_af = ps.at(i).getAF();
	}
	double load_ratio = 0.5;
	double target_load = target_af_sum*load_ratio;
	vector<Task> tasks = g.generate_tasks(target_load, false, 1, highest_af, 0);

	//simulate 30 seconds, which is 30000 milliseconds.
	s.run(tasks, cout, 1, 30000);

	cout<<s.getJobNum()<<std::endl;
	cout<<s.getMissNum()<<std::endl;



}