CPP=g++
FLAGS= -std=c++11 -Wall -Wextra -pedantic -g

all: arfeed

arfeed: arfeed.cpp
	$(CPP) $(FLAGS) arfeed.cpp -o arfeed

clean:
	rm -f arfeed xgarda04.tar 

tar: *.cpp *.h Makefile
	tar -cvf xgarda04.tar *.cpp *.h Makefile

