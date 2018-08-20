OBJS 	= drat-trim.o
CXX 	= g++

CPPFLAGS 	= -Wall -Wno-deprecated -O3 -ggdb -fpermissive -std=c++11
LFLAGS 	= -Wall -lz
NAME 	= drat-trim
prefix  = /usr/local

$(NAME) : $(OBJS)
	$(CXX) -o $(NAME) $(OBJS) $(LFLAGS)

drat-trim.o : drat-trim.cpp
	$(CXX) $(CPPFLAGS) -c drat-trim.cpp

clean:
	rm -f *.o 
	rm -f $(NAME)

install: drat-trim
	install -m 0755 drat-trim $(prefix)/bin

.PHONY: install
