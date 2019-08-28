#include "Scheduler.h"
#include "Generation.h"
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>

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


data run_single_round(int pcpu_num, double target_af_sum, double load_ratio, long long simulation_length, ostream &out, int starting_point)
{
	Generation g;
	//int pcpu_num = 5;
	Scheduler s(pcpu_num);
	//double target_af_sum = 3;
	vector<Partition> ps = g.generate_partitions(target_af_sum);//the partitions in ps is used in Scheduler s
	//cout<<"partitions generated."<<endl;
	while(!s.set_partitions(ps))
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
	/*for(int tc = 0; tc<tasks.size(); tc++)
	{
		cout<<tasks[tc].printInfo()<<"\n";
	}*/
	//simulate 30 seconds, which is 30000 milliseconds.
	int start_t = time(NULL);
	s.run(tasks, out, 1, simulation_length, starting_point);
	int end_t = time(NULL);
	//cout<<"Time spent in run: "<<end_t - start_t<<endl;
	data result;
	result.total_miss_num = s.getMissNum();
	result.total_job_num = s.getJobNum();
	return result;
}

int main(int argc, char *argv[])
{
	if(argc<=1)
		return 0;
	int t_j = 0;
	int t_m = 0;
	int schdulable_num = 0;

	int repeat_time = 1000;

	int pcpu_num = 4;
	double target_af_sum = 3;
	//double load_ratio = 0.5;
	double load_ratio = atof(argv[1]);
	long long simulation_length = 30000;
	int starting_point = 5000;

	int scheduling_mode = 1;
	std::fstream out, result_out;
	string file_name = std::to_string(scheduling_mode)+"_"+std::to_string(load_ratio)+".log";
	string result_file_name = std::to_string(scheduling_mode)+"_"+std::to_string(load_ratio)+"_result.txt";
	result_out.open(result_file_name, std::fstream::out);
	out.open(file_name, std::fstream::out);
	for(int i=0;i<repeat_time;i++)
	{
		if(i%50==0 && i!=0)
			starting_point += 10;
		int fd[2];
		pipe(fd);

		result_out<<"********************"<<endl;
		//sleep(10);
		result_out<<"Round "<<i<<endl;
		int pid = fork();
		if(pid==0)
		{
			close(fd[0]);
			data temp_result = run_single_round(pcpu_num, target_af_sum,load_ratio,simulation_length, out, starting_point);
			t_j += temp_result.total_job_num;
			t_m += temp_result.total_miss_num;
			if(temp_result.total_miss_num == 0)
				schdulable_num ++;
			write(fd[1], &t_j, sizeof(t_j));
			write(fd[1], &t_m, sizeof(t_m));
			write(fd[1], &schdulable_num, sizeof(schdulable_num));
			exit(0);
		}
		waitpid(pid, NULL, 0);
		close(fd[1]);
		read(fd[0], &t_j, sizeof(t_j));
		read(fd[0], &t_m, sizeof(t_m));
		read(fd[0], &schdulable_num, sizeof(schdulable_num));
		result_out<<"Miss ratio:"<<t_m<<" / "<<t_j<<endl;
		result_out<<"Schedulability: "<<schdulable_num/(double)(i+1)<<endl;
		result_out<<"********************"<<endl;
		result_out.flush();
	}
	out.flush();
	out.close();
	result_out.flush();
	result_out.close();


}