# LonelyRunner. make

op = -O2

LonelyRunner.out: main.o Geometric.o Numerical.o Prime_Number.o util.o 
	g++ $(op) -o LonelyRunner.out main.o Geometric.o Numerical.o util.o  `pkg-config gtk+-2.0 --libs` -I$(HOME)/sw/include -L$(HOME)/sw/lib -lntl -ljson -lgmp -lm

TestCode.out: main_test.o Geometric.o Numerical.o Prime_Number.o util.o 
	g++ $(op) -o TestCode.out main_test.o Geometric.o Numerical.o Prime_Number.o util.o -I$(HOME)/sw/include -L$(HOME)/sw/lib -lntl -ljson -lntl -lgmp -lm 

# above LonelyRunner.out depends on main.o, data_structure.o and Geometric.o

main.o: main.c Geometric.h Numerical.h data_structure.h util.o
	g++ $(op) -c main.c -l util.o `pkg-config gtk+-2.0 --cflags --libs` -I$(HOME)/sw/include -L$(HOME)/sw/lib -lntl -ljson -lgmp -lm 

main_test.o: main_test.cpp Geometric.h Numerical.h util.h Prime_Number.h util.o
	g++ $(op) -c main_test.cpp -ljson -l util.o

# above we are saying main.o depends on main.cpp data_structure.h and Geometric.cpp, and to compile main.cpp if and only if main.cpp or data_structure.h or geometric.cpp
# have changed since the last creation of main.o

Geometric.o: Geometric.cpp data_structure.h
	g++ $(op) -c Geometric.cpp -I$(HOME)/sw/include -L$(HOME)/sw/lib -lntl -lgmp -lm

Numerical.o: Numerical.cpp data_structure.h
	g++ $(op) -c Numerical.cpp -I$(HOME)/sw/include -L$(HOME)/sw/lib -lntl -lgmp -lm

Prime_Number.o: Prime_Number.cpp util.o
	g++ $(op) -c Prime_Number.cpp -l util.o

util.o: util.cpp data_structure.h
	g++ $(op) -c util.cpp -ljson

clean:
	rm *.o *~ *.out

# END OF MAKE FILE 
