# LonelyRunner. make

LonelyRunner.out: main.o Geometric.o
	g++ -o LonelyRunner.out main.o Geometric.o `pkg-config gtk+-2.0 --libs`

# above LonelyRunner.out depends on main.o, data_structure.o and Geometric.o

main.o: main.c data_structure.h Geometric.cpp Geometric.h
	g++ -c main.c `pkg-config gtk+-2.0 --cflags --libs`

# above we are saying main.o depends on main.cpp data_structure.h and Geometric.cpp, and to compile main.cpp if and only if main.cpp or data_structure.h or geometric.cpp
# have changed since the last creation of main.o

Geometric.o: Geometric.cpp
	g++ -c Geometric.cpp

#event_point.o: event_point.cpp event_point.h
#	g++ -c event_point.cpp

clean:
	rm *.o *~ LonelyRunner.out

# END OF MAKE FILE 
