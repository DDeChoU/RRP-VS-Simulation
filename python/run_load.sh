
#!/bin/bash
#SBATCH -J SIM_RRP
#SBATCH -o SIM_RRP.o%j
#SBATCH -t 15:00:00
#SBATCH -N 1 -n 10
#SBATCH -A amcheng

# Inclusion of timer to see the time elapsed
START=$(date +%s)

loads=(0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0)

for load in ${loads[@]};
do
  	echo $load
        # Enable Parallelization
        sbatch run.sh $load &
done
END=$(date +%s)
echo $?
DIFF=$(( END - START ))
echo "Time Elapsed: $DIFF seconds"
