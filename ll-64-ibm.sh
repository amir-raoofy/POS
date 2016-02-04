#!/bin/bash 
#@ wall_clock_limit = 00:05:00
#@ job_name = pos-cannon-mpi-ibm
#@ job_type = Parallel
#@ output = cannon_64_$(jobid).out
#@ error = cannon_64_$(jobid).out
#@ class = test
#@ node = 4
#@ total_tasks = 64
#@ node_usage = not_shared
#@ energy_policy_tag = cannon
#@ minimize_time_to_solution = yes
#@ notification = never
#@ island_count = 1
#@ queue

. /etc/profile
. /etc/profile.d/modules.sh

cd /home/hpc/h039y/h039y29/cannon_src

mpiexec -n 1 ./conversion 64x64-1.in  64x64-1.dat
mpiexec -n 1 ./conversion 64x64-2.in  64x64-2.dat
max=50
for (( i=1; i <= $max; ++i ))
        do
                date
		mpiexec -n 64 ./cannon 64x64-1.dat 64x64-2.dat 64x64-3.dat 64x64-1.in 64x64-2.in
        done

mpiexec -n 1 ./conversion 128x128-1.in  128x128-1.dat
mpiexec -n 1 ./conversion 128x128-2.in  128x128-2.dat
max=50
for (( i=1; i <= $max; ++i ))
        do
                date
		mpiexec -n 64 ./cannon 128x128-1.dat 128x128-2.dat 128x128-3.dat 128x128-1.in 128x128-2.in
        done



mpiexec -n 1 ./conversion 256x256-1.in  256x256-1.dat
mpiexec -n 1 ./conversion 256x256-2.in  256x256-2.dat
max=50
for (( i=1; i <= $max; ++i ))
        do
                date
		mpiexec -n 64 ./cannon 256x256-1.dat 256x256-2.dat 256x256-3.dat 256x256-1.in 256x256-2.in
        done

mpiexec -n 1 ./conversion 512x512-1.in  512x512-1.dat
mpiexec -n 1 ./conversion 512x512-2.in  512x512-2.dat
max=50
for (( i=1; i <= $max; ++i ))
        do
                date
		mpiexec -n 64 ./cannon 512x512-1.dat 512x512-2.dat 512x512-3.dat 512x512-1.in 512x512-2.in
        done

mpiexec -n 1 ./conversion 1024x1024-1.in  1024x1024-1.dat
mpiexec -n 1 ./conversion 1024x1024-2.in  1024x1024-2.dat
max=50
for (( i=1; i <= $max; ++i ))
        do
                date
		mpiexec -n 64 ./cannon 1024x1024-1.dat 1024x1024-2.dat 1024x1024-3.dat 1024x1024-1.in 1024x1024-2.in
        done

mpiexec -n 1 ./conversion 2048x2048-1.in  2048x2048-1.dat
mpiexec -n 1 ./conversion 2048x2048-2.in  2048x2048-2.dat
max=50
for (( i=1; i <= $max; ++i ))
        do
                date
		mpiexec -n 64 ./cannon 2048x2048-1.dat 2048x2048-2.dat 2048x2048-3.dat 2048x2048-1.in 2048x2048-2.in
        done

mpiexec -n 1 ./conversion 4096x4096-1.in  4096x4096-1.dat
mpiexec -n 1 ./conversion 4096x4096-2.in  4096x4096-2.dat
max=50
for (( i=1; i <= $max; ++i ))
        do
                date
		mpiexec -n 64 ./cannon 4096x4096-1.dat 4096x4096-2.dat 4096x4096-3.dat 4096x4096-1.in 4096x4096-2.in
        done













#mpiexec -n 64 ./cannon 128x128-1.in 128x128-2.in
#date
#mpiexec -n 64 ./cannon 256x256-1.in 256x256-2.in
#date
#mpiexec -n 64 ./cannon 512x512-1.in 512x512-2.in
#date
#mpiexec -n 64 ./cannon 1024x1024-1.in 1024x1024-2.in
#date
#mpiexec -n 64 ./cannon 2048x2048-1.in 2048x2048-2.in
#date
#mpiexec -n 64 ./cannon 4096x4096-1.in 4096x4096-2.in
#date
