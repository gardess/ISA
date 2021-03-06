#Makefile pro ISA projekt
#Autor: Milan Gardáš <xgarda04@stud.fit.vutbr.cz>

CPP=g++
CFLAGS= -std=c++11 -Wall -Wextra -pedantic -g `xml2-config --cflags`
LDFLAGS = -L/usr/local/ssl/lib `xml2-config --libs`
LDLIBS = -lssl -lcrypto

all: arfeed

arfeed: arfeed.cpp
	$(CPP) $(CFLAGS) arfeed.cpp -o arfeed $(LDFLAGS) $(LDLIBS)

clean:
	rm -f arfeed xgarda04.tar 

tar: *.cpp *.h Makefile
	tar -cvf xgarda04.tar *.cpp *.h Makefile README

commit: clean
	git commit -a
	git pull origin master
	git push origin master

