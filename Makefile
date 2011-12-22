default: test
	./test.rb
	./test

test: test.o
	g++ -o test -lboost_unit_test_framework-mt test.o

test.o: test.cpp rodb.h Value.h Database.h
	g++ -c -Wall -o test.o test.cpp

clean:
	rm test
	rm *.o
	rm unit_test.rodb
	rm unit_test.yaml
