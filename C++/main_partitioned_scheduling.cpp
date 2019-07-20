#include "Scheduler.h"
#include "Generation.h"
using std::cout;
using std::endl;

struct data
{
	int total_miss_num;
	int total_job_num;
	data()
	{
		total_job_num = 0;
		total_miss_num = 0;
	}
};


data run_single_round(int pcpu_num, double target_af_sum, double load_ratio, long long simulation_length)
{
	Generation g;
	//int pcpu_num = 5;
	Scheduler s(pcpu_num);
	//double target_af_sum = 3;
	vector<Partition> ps = g.generate_partitions(target_af_sum);//the partitions in ps is used in Scheduler s
	cout<<"partitions generated."<<endl;
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
	//double load_ratio = 0.5;
	double target_load = target_af_sum*load_ratio;
	vector<Task> tasks = g.generate_tasks(target_load, false, 1, highest_af, 0);

	//simulate 30 seconds, which is 30000 milliseconds.
	s.run(tasks, cout, 1, simulation_length);
	data result;
	result.total_miss_num = s.getMissNum();
	result.total_job_num = s.getJobNum();
	return result;
}

int main()
{
	int t_j = 0;
	int t_m = 0;
	int schdulable_num = 0;

	int repeat_time = 1000;

	int pcpu_num = 4;
	double target_af_sum = 3;
	double load_ratio = 0.5;
	long long simulation_length = 30000;

	for(int i=0;i<repeat_time;i++)
	{
		//this loop cannot be automatically repeated, or else: port already in use! Why???
		cout<<"********************"<<endl;
		//sleep(10);
		cout<<"Round "<<i<<endl;
		data temp_result = run_single_round(pcpu_num, target_af_sum,load_ratio,simulation_length);
		t_j += temp_result.total_job_num;
		t_m += temp_result.total_miss_num;
		if(temp_result.total_miss_num == 0)
			schdulable_num ++;
		cout<<"Miss ratio:"<<t_m<<" / "<<t_j<<endl;
		cout<<"Schedulability: "<<schdulable_num/(double)(i+1)<<endl;
		cout<<"********************"<<endl;

		//sleep(10);
	}



}