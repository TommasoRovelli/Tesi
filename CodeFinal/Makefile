COMPILER = mpicxx
FLAGS= -O3 -std=c++11

Monte_Carlo_NVT_MPI.x: Monte_Carlo_NVT_MPI.o
	$(COMPILER) Monte_Carlo_NVT_MPI.o $(FLAGS) -o Monte_Carlo_NVT_MPI.x

Monte_Carlo_NVT_MPI.o: Monte_Carlo_NVT_MPI.cpp Monte_Carlo_NVT_MPI.h
	$(COMPILER) -c Monte_Carlo_NVT_MPI.cpp  $(FLAGS) -o Monte_Carlo_NVT_MPI.o

clear:
	rm -f *.o *.x

clean:
	./clean.sh

remove_output:
	rm output*
	rm restart.dat
	rm OutputConfig/output*
	rm OutputEpotCores/output*
	rm OutputEpotDistribCores/output*
	rm OutputAngleDistribCores/output*
	rm OutputBoxxDistribCores/output*
	rm OutputBoxyDistribCores/output*
	rm OutputSpecificHeatCores/output*

