default: example example.rodb

example: example.o
	g++ -o example example.o

example.o: example.cpp ../rodb.h ../Database.h ../Value.h
	g++ -c -Wall -o example.o example.cpp

example.rodb: example.yaml
	../yaml2rodb.rb example.yaml example.rodb

clean:
	rm -f example example.o example.rodb
