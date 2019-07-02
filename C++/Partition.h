#ifndef PARTITION_H
#define PARTITION_H
#include <vector>
#include "Task.h"
#include "Job.h"
#include "string"
using std::string;
using std::vector;
class Partition
{
private:
	double availability_factor;
	int regularity;
	
	vector<Job> job_queue;

public:
	//constructor takes in the availability factor and regularity. There is no way to change once it is set.
	Partition(double af, int reg)
	{
		availability_factor = af;
		regularity = reg;
	}
	double getAF(){return availability_factor;}
	double getReg(){return regularity;}
	string printInfo()
	{
		string result;
		result+= std::to_string(availability_factor)+","+std::to_string(regularity);
		return result;

	}

};
#endif