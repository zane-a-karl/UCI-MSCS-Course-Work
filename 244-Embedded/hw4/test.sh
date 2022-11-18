#!/bin/bash

#Remember! This script takes the
#img id 0, 1 ,2 as input=$1

# 3^6 different outputs
sigma=(0.6 1.5 2.4)
tlow=(0.2 0.35 0.5)
thigh=(0.6 0.75 0.9)
width=(400 850 1300)
height=(100 550 1000)
#MP    : 1x, pthread, openmp

for sig in ${sigma[@]};
do
		for low in ${tlow[@]};
		do
				for high in ${thigh[@]};
				do
						for w in ${width[@]};
						do
								for h in ${height[@]};
								do
										#assume that they
										#have already been compiled
										./psnr $sig $low $high $w $h 0 $1 &
										./psnr_pt $sig $low $high $w $h 1 $1 &
										./psnr_omp $sig $low $high $w $h 2 $1 &
								done
						done
				done
		done
done

										
