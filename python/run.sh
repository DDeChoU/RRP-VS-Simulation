#!/bin/bash 
#SBATCH -J SIM_RRP
#SBATCH -o SIM_RRP.o%j 
#SBATCH -t 15:00:00 
#SBATCH -N 1 -n 10
#SBATCH -A amcheng 

echo $1
python3 main.py 5 8 30 $1
