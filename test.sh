#!/bin/bash

for i in 1 2 3 4 5 6 7 8 9 10 
do 
    for j in 10 100 1000 10000 100000
    do
        ./add --threads=$i --iterations=$j >> add.csv   
    done
done

#only yield is passed
for i in 1 2 4 8 12 16 24
do
    for j in 1 10 20 40 80 100 1000 10000
    do 
        ./add --threads=$i --iterations=$j --yield --sync=m >> add.csv
        ./add --threads=$i --iterations=$j --yield --sync=s >> add.csv
        ./add --threads=$i --iterations=$j --yield --sync=c >> add.csv
        ./add --threads=$i --iterations=$j --yield >> add.csv
        ./add --threads=$i --iterations=$j --sync=m >> add.csv
        ./add --threads=$i --iterations=$j --sync=s >> add.csv
        ./add --threads=$i --iterations=$j --sync=c >> add.csv
        ./add --threads=$i --iterations=$j >> add.csv
    done
done


#yield vs no yield
for i in 2 8
do 
    for j in 100 1000 10000 100000
    do
        ./add --threads=$i --iterations=$j --yield >> add.csv
        ./add --threads=$i --iterations=$j >> add.csv
    done
done

for i in 10 100 1000 10000 100000
do 
    ./add --threads=1 --iterations=$i >> add.csv
done

#c vs m vs s with yield
for a in 1 2 3
do 
    for i in 2 4 6 8
    do 
        ./add --threads=$i --iterations=10000 --yield --sync='m' >> add.csv
        ./add --threads=$i --iterations=10000 --yield --sync='c' >> add.csv
        ./add --threads=$i --iterations=1000 --yield --sync='s' >> add.csv
    done
done


#c vs m vs s vs none with no yield
for i in 1 2 4 8 12
do 
    ./add --threads=$i --iterations=10000 >> add.csv
    ./add --threads=$i --iterations=1000 --sync='s' >> add.csv
    ./add --threads=$i --iterations=1000 --sync='m' >> add.csv
    ./add --threads=$i --iterations=1000 --sync='c' >> add.csv
done


#------------START OF list.C---------------




#Run your program with a single thread, and increasing numbers of iterations (10, 100, 1000, 10000, 20000), capture the output, and plot (using the supplied data reduction script) the mean cost per operation vs the number of iterations. 
for i in 10 100 1000 10000 20000
do
    ./list --iterations=$i 1>>list.csv 2>/dev/null
done


#Run your program and see how many parallel threads (2,4,8,12) and iterations (1, 10,100,1000) it takes to fairly consistently demonstrate a problem.

for i in 2 4 8 12
do
    for j in 1 10 100 1000
    do
        ./list --threads=$i --iterations=$j 1>>list.csv 2>/dev/null
    done
done


#Then run it again using various combinations of yield options and see how many threads (2,4,8,12) and iterations (1, 2,4,8,16,32) it takes to fairly consistently demonstrate the problem.
for i in 2 4 8 12 16 24
do 
    for j in  1 2 4 8 16 32
    do 
        ./list --threads=$i --iterations=$j --yield=i 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=l 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=d 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=il 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=id 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=ld 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=ild 1>>list.csv 2>/dev/null
    done
done

#Using your --yield options, demonstrate that either of these protections seems to eliminate all of the problems, even for moderate numbers of threads (12) and iterations (32).
for i in 2 4 8 12 16 24
do 
    for j in  1 2 4 8 16 32
    do 
        ./list --threads=$i --iterations=$j --yield=i --sync=m 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=i --sync=s 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=l --sync=m 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=l --sync=s 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=d --sync=m 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=d --sync=s 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=il --sync=m 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=il --sync=s 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=id --sync=m 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=id --sync=s 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=ld --sync=m 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=ld --sync=s 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=ild --sync=m 1>>list.csv 2>/dev/null
        ./list --threads=$i --iterations=$j --yield=ild --sync=s 1>>list.csv 2>/dev/null
    done
done


#Choose an appropriate number of iterations (e.g. 1000) to overcome start-up costs and rerun your program without the yields for a range of # threads (1, 2, 4, 8, 12, 16, 24). 
for i in 1 2 4 8 12 16 24 32
do  
    ./list --threads=$i --iterations=1000 1>>list.csv 2>/dev/null
    ./list --threads=$i --iterations=1000 --sync=s 1>>list.csv 2>/dev/null
    ./list --threads=$i --iterations=1000 --sync=m 1>>list.csv 2>/dev/null
done




