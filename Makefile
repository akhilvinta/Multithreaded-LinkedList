#Akhil Vinta

build:
	gcc -Wall -Wextra -pthread ParallelizedAdd.c -o add
	gcc -Wall -Wextra -pthread ParallelizedList.c SortedList.c -o list
tests: add list test.sh
	rm -rf add.csv list.csv
	./test.sh
graphs: add.csv list.csv
	gnuplot ./add.gp
	gnuplot ./list.gp
dist: build tests graphs 
	tar -czvf MultiThreadedLinkedList.tar.gz Makefile README ParallelizedAdd.c ParallelizedList.c SortedList.h SortedList.c test.sh 
clean:
	rm -f add list lab2a-405288527.tar.gz
