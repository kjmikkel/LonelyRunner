# LonelyRunner. make

LonelyRunner.out: main.o data_structure.o Geometric.o
	g++ -o LonelyRunner.out main.o data_structure.o Geometric.o

# above LonelyRunner.out depends on main.o, data_structure.o and Geometric.o

main.o: main.cpp data_structure.cpp Geometric.cpp
	g++ -c main.cpp

# above we are saying main.o depends on main.cpp data_structure.h and Geometric.cpp, and to compile main.cpp if and only if main.cpp or data_structure.h or geometric.cpp
# have changed since the last creation of main.o

Geometric.o: Geometric.cpp data_structure.cpp # event_point.cpp
	g++ -c Geometric.cpp

data_structure.o: data_structure.cpp
	g++ -c data_structure.cpp

#event_point.o: event_point.cpp event_point.h
#	g++ -c event_point.cpp

clean:
	rm *.o LonelyRunner.out

# END OF MAKE FILE 
