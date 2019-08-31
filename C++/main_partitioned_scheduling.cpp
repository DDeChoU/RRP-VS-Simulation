#include "Scheduler.h"
#include "Generation.h"
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>
#include <vector>

using std::cout;
using std::endl;
using std::vector;

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


vector<data> run_single_round(int pcpu_num, double target_af_sum, double load_ratio, long long simulation_length, vector<string> &file_names, int starting_point)
{
	Generation g;
	vector<Partition> partitions;
	vector<Task> tasks;
	vector<data> results;
	int policy_num =4;
	bool first = true;
	for(int i=0;i<policy_num;i++)
	{
		Scheduler s(pcpu_num);
		vector<Partition> temp_ps;
		vector<Task> temp_ts;
		if(first)
		{
			temp_ps = g.generate_partitions(target_af_sum);
			while(!s.set_partitions(temp_ps))
			{
				//cout<<"Resetting partitions.\n";
				temp_ps.clear();
				temp_ps = g.generate_partitions(target_af_sum);
			}
			double highest_af = 0;
			for(int j=0;j<temp_ps.size();j++)
			{
				if(temp_ps.at(j).getAF()>highest_af)
					highest_af = temp_ps.at(i).getAF();
			}
			double target_load = target_af_sum*load_ratio;
			temp_ts = g.generate_tasks(target_load, false, 1, highest_af, 0);
			partitions = temp_ps;
			tasks = temp_ts;
			first = false;	
		}
		else
		{
			temp_ps = partitions;
			temp_ts = tasks;
			s.set_partitions(temp_ps);
		}
		//run the simulation here and collect info
		std::fstream out;
		out.open(file_names[i], std::fstream::app);
		//modify the parameter based on the invokes!
		s.run(temp_ts, out, i+1, simulation_length, starting_point+i*10);
		out.flush();
		out.close();
		//put the result into the result
		data temp_result;
		temp_result.total_miss_num = s.getMissNum();
		temp_result.total_job_num = s.getJobNum();
		results.push_back(temp_result);
	}
	return results;
	/*
	//int pcpu_num = 5;
	Scheduler first_s(pcpu_num);
	//double target_af_sum = 3;
	ps = g.generate_partitions(target_af_sum);//the partitions in ps is used in Scheduler s
	//cout<<"partitions generated."<<endl;
	while(!first_s.set_partitions(ps))
	{
		//cout<<"Resetting partitions.\n";
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
	int start_t = time(NULL);
	s.run(tasks, out, 1, simulation_length, starting_point);
	int end_t = time(NULL);
	//cout<<"Time spent in run: "<<end_t - start_t<<endl;
	data result;
	result.total_miss_num = s.getMissNum();
	result.total_job_num = s.getJobNum();
	return result;*/
}

int main(int argc, char *argv[])
{
	if(argc<=1)
		return 0;

	int repeat_time = 1000;

	int pcpu_num = 3;
	double target_af_sum = 2;
	//double load_ratio = 0.5;
	double load_ratio = atof(argv[1]);
	long long simulation_length = 30000;
	int starting_point = 5000;

	int policy_num = 4;
	int scheduling_mode = 1;
	//vector<std::fstream> out, result_out;
	vector<long long> total_miss_num(policy_num, 0);
	vector<long long> total_job_num(policy_num, 0);
	vector<double> schedulable_num(policy_num, 0);
	vector<string> out;
	vector<std::fstream> result_out(policy_num);
	//cout<<"Initializing files."<<endl;
	for(int i=0;i<policy_num;i++)
	{
		string file_name = std::to_string(i+1)+"_"+std::to_string(load_ratio)+".log";
		out.push_back(file_name);
		string result_file_name = std::to_string(i+1)+"_"+std::to_string(load_ratio)+"_result.txt";
		result_out[i].open(result_file_name, std::ios_base::out);	
		std::fstream temp_out;
		temp_out.open(file_name, std::ios::out);
		temp_out.close();
	}
	//cout<<"Start repeating."<<endl;
	for(int i=0;i<repeat_time;i++)
	{
		if(i%50==0 && i!=0)
			starting_point += 50;
		int fd[2];
		pipe(fd);
		for(int j=0;j<policy_num;j++)
		{
			result_out[j]<<"********************"<<endl;
			//sleep(10);
			result_out[j]<<"Round "<<i<<endl;
		}
		int pid = fork();
		if(pid==0)
		{
			close(fd[0]);
			vector<data> temp_result = run_single_round(pcpu_num, target_af_sum,load_ratio,simulation_length, out, starting_point);
			for(int j=0;j<policy_num;j++)
			{
				total_miss_num[j] += temp_result[j].total_miss_num;
				total_job_num[j] += temp_result[j].total_job_num;
				if(temp_result[j].total_miss_num==0)
					schedulable_num[j]++;
				write(fd[1], &total_job_num[j], sizeof(total_job_num[j]));
				write(fd[1], &total_miss_num[j], sizeof(total_miss_num[j]));
				write(fd[1], &schedulable_num[j], sizeof(schedulable_num[j]));
			}

			exit(0);
		}
		waitpid(pid, NULL, 0);
		close(fd[1]);
		for(int j=0;j<policy_num;j++)
		{
			read(fd[0], &total_job_num[j], sizeof(total_job_num[j]));
			read(fd[0], &total_miss_num[j], sizeof(total_miss_num[j]));
			read(fd[0], &schedulable_num[j], sizeof(schedulable_num[j]));
			result_out[j]<<"Miss ratio:"<<total_miss_num[j]<<" / "<<total_job_num[j]<<endl;
			result_out[j]<<"Schedulability: "<<schedulable_num[j]/(double)(i+1)<<endl;
			result_out[j]<<"********************"<<endl;
			result_out[j].flush();
		}
		
	}

	for(int j=0;j<policy_num;j++)
	{
		result_out[j].flush();
		result_out[j].close();
	}

}