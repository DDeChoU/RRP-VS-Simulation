#!/bin/bash 
#SBATCH -J TEST_0.5
#SBATCH -o TEST_0.5.o%j 
#SBATCH -t 15:00:00 
#SBATCH -N 1 -n 10
#SBATCH -A amcheng 

python3 main.py 5 8 30 0.5
