#loads=(0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0)
loads=(1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8 1.9 2.0)
for load in ${loads[@]};
do
	echo $load
	sbatch run.sh $load
done
