/*
* author: @Guangli Dai
* Date created: July 1st, 2019
* Last modified: 
*/
#ifndef GENERATION_H
#define GENERATION_H
#include <vector>
#include <random>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "Task.h"
#include "Partition.h"
#include <iostream>
using std::vector;

#define TIME_SLICE_LEN 10
class Generation
{
private:
	vector<double> gen_kato_utilizations(double target_val, double min_val, double max_val);
public:
	vector<Partition> generate_partitions(double target_af);
	vector<Task> generate_tasks(double target_load, bool has_sporadic = false, 
			double periodic_ratio = 0.5, double highest_density = 1, double hard_rt_ratio = 0);
};
vector<double> Generation::gen_kato_utilizations(double target_val, double min_val, double max_val)
{
	/*
		This function is modified from the function gen_kato_utilizations in class simso.generator.task_generator.
	*/
	std::default_random_engine generator(time(NULL));
	std::uniform_real_distribution<double> distribution(min_val,max_val);
	double total_val = 0;
	vector<double> result;
	while(total_val<target_val)
	{
		double val = distribution(generator);
		if(val+total_val>target_val)
			val = target_val - total_val;
		total_val += val;
		result.push_back(val);
	}
	return result;
}
vector<Partition> Generation::generate_partitions(double target_af)
{
	/*
	*	This function takes in the target total availability factor and returns
	*	a randomly generated partition set.
	*/
	vector<Partition> result;
	int counter = 0;
	vector<double> afs = gen_kato_utilizations(target_af, 0.1, 1);
	srand(time(NULL));
	string prefix = "Par#";
	for(int i=0;i<afs.size();i++)
	{
		counter ++;
		int reg = rand()%2+1;
		//std::cout<<reg<<std::endl;
		Partition p_now = Partition(afs.at(i), reg,  prefix+std::to_string(counter));
		result.push_back(p_now);
	}
	return result;
}

vector<Task> Generation:: generate_tasks(double target_load, bool has_sporadic, 
			double periodic_ratio, double highest_density, double hard_rt_ratio)
{
	/*
		Args:
		- target_load: Total utilization to reach.
		- has_sporadic: whether tasks has_sporadic or not
		- periodic_ratio: the ratio of periodic tasks.
		- highest_density: the highest possible density.
	*/
	vector<Task> tasks;
	vector<double> densities = gen_kato_utilizations(target_load, 0.00001, highest_density);
	srand(time(NULL));
	int counter = 0;
	string prefix = "Task#";
	for(int i=0;i<densities.size();i++)
	{
		counter ++;
		double density_now = densities.at(i);		
		int period = (rand()%(2000-5)+5)*TIME_SLICE_LEN;
		double wcet = ceil(density_now*period);
		int deadline = period;
		double arrival = rand()%3000*TIME_SLICE_LEN;
		bool is_p = true;
		if(has_sporadic)
		{
			int dice = rand()%10;
			if(dice>=periodic_ratio*10)
				is_p = false;
		}
		bool is_hard_rt = false;
		if(hard_rt_ratio!=0)
		{
			int dice = rand()%10;
			if(dice<=hard_rt_ratio*10)
				is_hard_rt = true;
		}
		Task task_now = Task(wcet, period, deadline, arrival, prefix+std::to_string(counter), is_p, is_hard_rt);
		tasks.push_back(task_now);
	}
	return tasks;


}
#endif