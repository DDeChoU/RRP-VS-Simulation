#!/bin/bash
#SBATCH -J SIM_RRP
#SBATCH -o SIM_RRP.o%j
#SBATCH -t 15:00:00
#SBATCH -N 1 -n 10
#SBATCH -A amcheng

# Inclusion of timer to see the time elapsed

#loads=(0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0)
loads=(1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8 1.9 2.0)
for load in ${loads[@]};
do
  	echo $load
        # Enable Parallelization
        sbatch run.sh $load &
done

