# LonelyRunner. make



TestCode.out: main_test.o Geometric.o Numerical.o Prime_Number.o
	g++ -O2 -o TestCode.out main_test.o Geometric.o Numerical.o Prime_Number.o -I$(HOME)/sw/include -L$(HOME)/sw/lib -lntl -ljson -lntl -lgmp -lm 

LonelyRunner.out: main.o Geometric.o Numerical.o
	g++ -O2 -o LonelyRunner.out main.o Geometric.o Numerical.o `pkg-config gtk+-2.0 --libs`

# above LonelyRunner.out depends on main.o, data_structure.o and Geometric.o

main.o: main.c data_structure.o Geometric.h Numerical.h
	g++ -O2 -c main.c `pkg-config gtk+-2.0 --cflags --libs`

main_test.o: main_test.cpp Geometric.h Numerical.h Prime_Number.h
	g++ -O2 -c main_test.cpp -ljson

# above we are saying main.o depends on main.cpp data_structure.h and Geometric.cpp, and to compile main.cpp if and only if main.cpp or data_structure.h or geometric.cpp
# have changed since the last creation of main.o

Geometric.o: Geometric.cpp data_structure.h
	g++ -O2 -c Geometric.cpp -I$(HOME)/sw/include -L$(HOME)/sw/lib -lntl -lgmp -lm

#g++  -I$HOME/sw/include foo.c -o foo  -L$HOME/sw/lib -lntl -lgmp -lgf2x  -lm


Numerical.o: Numerical.cpp data_structure.h
	g++ -O2 -c Numerical.cpp -I$(HOME)/sw/include -L$(HOME)/sw/lib -lntl -lgmp -lm

Prime_Number.o: Prime_Number.cpp
	g++ -O2 -c Prime_Number.cpp

clean:
	rm *.o *~ LonelyRunner.out

# END OF MAKE FILE 
