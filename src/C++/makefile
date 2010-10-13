# LonelyRunner. make



TestCode.out: main_test.o Geometric.o Numerical.o Prime_Number.o
	g++ -o TestCode.out main_test.o Geometric.o Numerical.o Prime_Number.o -ljson

LonelyRunner.out: main.o Geometric.o Numerical.o
	g++ -o LonelyRunner.out main.o Geometric.o Numerical.o `pkg-config gtk+-2.0 --libs`

# above LonelyRunner.out depends on main.o, data_structure.o and Geometric.o

main.o: main.c data_structure.h Geometric.h Numerical.h
	g++ -c main.c `pkg-config gtk+-2.0 --cflags --libs`

main_test.o: main_test.cpp Geometric.h Numerical.h Prime_Number.h
	g++ -c main_test.cpp -ljson

# above we are saying main.o depends on main.cpp data_structure.h and Geometric.cpp, and to compile main.cpp if and only if main.cpp or data_structure.h or geometric.cpp
# have changed since the last creation of main.o

Geometric.o: Geometric.cpp
	g++ -c Geometric.cpp

Numerical.o: Numerical.cpp
	g++ -c Numerical.cpp

Prime_Number.o: Prime_Number.cpp
	g++ -c Prime_Number.cpp

clean:
	rm *.o *~ LonelyRunner.out

# END OF MAKE FILE 
