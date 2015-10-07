#Makefile pro ISA projekt
#Autor: Milan Gardáš <xgarda04@stud.fit.vutbr.cz>

CPP=g++
FLAGS= -std=c++11 -Wall -Wextra -pedantic -g

all: arfeed

arfeed: arfeed.cpp
	$(CPP) $(FLAGS) arfeed.cpp -o arfeed

clean:
	rm -f arfeed xgarda04.tar 

tar: *.cpp *.h Makefile
	tar -cvf xgarda04.tar *.cpp *.h Makefile

commit: clean
	git commit -a
	git pull origin master
	git push origin master

