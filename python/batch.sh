#!/bin/bash 
#SBATCH -J OS_SIM_TEST
#SBATCH -o OS_SIM_TEST.o%j 
#SBATCH -t 00:03:00 
#SBATCH -N 1 -n 10
#SBATCH -A amcheng 

python3 ./main.py
