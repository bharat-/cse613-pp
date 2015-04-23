#!/bin/bash
#SBATCH -J Test         # Job Name
#SBATCH -o Test.o%j     # Output and error file name (%j expands to jobID)
#SBATCH -n 1            # Total number of mpi tasks requested
#SBATCH -p normal       # Queue (partition) name -- normal, development, etc.
#SBATCH -t 04:00:00     # Run time (hh:mm:ss) - 1.5 hours
#SBATCH -N 1            # Number of nodes

export PATH=$PATH:$HOME/cilk/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/cilk/lib

export CILK_NWORKERS=16

#for cilkview scalability plot
#/home1/03442/tg827409/cilkutil/bin/cilkview Par_Deterministic_CC.out < /work/01905/rezaul/CSE613/HW2/turn-in/com-lj-in.txt >> scale-com-lj-det-CC-out.txt

./Par_Deterministic_CC.out < /work/01905/rezaul/CSE613/HW2/turn-in/ca-AstroPh-in.txt     >> ca-AstroPh-det-CC-out.txt
./Par_Deterministic_CC.out < /work/01905/rezaul/CSE613/HW2/turn-in/com-amazon-in.txt     >> com-amazon-det-CC-out.txt
./Par_Deterministic_CC.out < /work/01905/rezaul/CSE613/HW2/turn-in/com-dblp-in.txt       >> com-dblp-det-CC-out.txt
./Par_Deterministic_CC.out < /work/01905/rezaul/CSE613/HW2/turn-in/roadNet-PA-in.txt     >> roadNet-PA-det-CC-out.txt
./Par_Deterministic_CC.out < /work/01905/rezaul/CSE613/HW2/turn-in/roadNet-TX-in.txt     >> roadNet-TX-det-CC-out.txt
./Par_Deterministic_CC.out < /work/01905/rezaul/CSE613/HW2/turn-in/roadNet-CA-in.txt     >> roadNet-CA-det-CC-out.txt
./Par_Deterministic_CC.out < /work/01905/rezaul/CSE613/HW2/turn-in/as-skitter-in.txt     >> as-skitter-det-CC-out.txt
./Par_Deterministic_CC.out < /work/01905/rezaul/CSE613/HW2/turn-in/com-lj-in.txt         >> com-lj-det-CC-out.txt
./Par_Deterministic_CC.out < /work/01905/rezaul/CSE613/HW2/turn-in/com-orkut-in.txt      >> com-orkut-det-CC-out.txt
./Par_Deterministic_CC.out < /work/01905/rezaul/CSE613/HW2/turn-in/com-friendster-in.txt >> com-friendster-det-CC-out.txt
