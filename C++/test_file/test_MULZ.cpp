#include <iostream>
#include <vector>
#include "Scheduler.h"
using std::vector;
using std::cout;
using std::endl;

int main()
{
	Scheduler s(4);
	vector<Partition> partition_list;
	Partition p1(0.3, 1, "P1");
	Partition p2(0.65, 1, "P2");
	Partition p3(0.6, 1, "P3");
	Partition p4(0.35, 1, "P4");
	Partition p5(0.5, 1, "P5");
	Partition p6(0.55, 1, "P6");
	Partition p7(0.25, 1, "P7");
	Partition p8(0.25, 1, "P8");
	Partition p9(0.25, 1, "P9");
	partition_list.push_back(p1);
	partition_list.push_back(p2);
	partition_list.push_back(p3);
	partition_list.push_back(p4);
	partition_list.push_back(p5);
	partition_list.push_back(p6);
	partition_list.push_back(p7);
	partition_list.push_back(p8);
	partition_list.push_back(p9);
	vector<Task> task_list;
	s.run(task_list, partition_list, cout);
	return 0;
}