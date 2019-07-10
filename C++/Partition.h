#ifndef PARTITION_H
#define PARTITION_H
#include <vector>
#include <math.h>
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
	string partition_id;
	double aaf;
	int aaf_up;
	int aaf_down;

	vector<Job> job_queue;

public:
	//constructor takes in the availability factor and regularity. There is no way to change once it is set.
	Partition(double af, int reg, string id)
	{
		availability_factor = af;
		regularity = reg;
		partition_id = id;
		aaf = availability_factor;
	}
	Partition(const Partition &a)
	{
		availability_factor = a.availability_factor;
		regularity = a.regularity;
		partition_id = a.partition_id;
	}
	double getAF() const{return availability_factor;} 
	double getReg() const{return regularity;}
	string getID() const{return partition_id;}
	void setAAF(double aaf_in){aaf = ceil(aaf_in*1000)/(double)1000;}//keep 4-digits' precision
	void setAAFFrac(int a, int b){aaf_up = a; aaf_down = b; reduceAAF();}
	int getAAFUp(){return aaf_up;}
	int getAAFDown(){return aaf_down;}
	double getAAF() const{return aaf;}
	string printInfo()
	{
		string result = partition_id+":";
		result+= std::to_string(availability_factor)+","+std::to_string(regularity);
		return result;

	}

	//comparison function overloaded for the usage of priority_queue
	bool operator()(const Partition &a, const Partition &b) const
	{
		return a.availability_factor<b.availability_factor;
	}
private:
	void reduceAAF()
	{
		int a = aaf_down, b = aaf_up;
		while(b!=0)
		{
			int temp = b;
			b = a%b;
			a = temp;
		}
		aaf_down /= a;
		aaf_up /= a;
	}


};
#endif