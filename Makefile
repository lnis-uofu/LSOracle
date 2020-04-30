yosys_dir = /research/ece/lnis/USERS/austin/test/yosys
CXX = gcc
CXXFLAGS = -Wall -Wextra -ggdb -I"/research/ece/lnis/USERS/austin/test/yosys" \
                      -MD -D_YOSYS_ -fPIC -I/usr/local/include -std=c++11 \
                      -Os -DYOSYS_ENABLE_READLINE -DYOSYS_ENABLE_PLUGINS \
                      -DYOSYS_ENABLE_GLOB -DYOSYS_ENABLE_ZLIB -DYOSYS_ENABLE_ABC \
                      -DYOSYS_ENABLE_COVER

LDFLAGS = -L/usr/include -L/research/ece/lnis/USERS/austin/test/yosys/libs -rdynamic

LDLIBS = -lstdc++ -lm -lrt -lreadline -lffi -ldl -lz
PLUGINS_DIR = $(yosys_dir)/plugins

OBJS = oracle.o

oracle.so: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

.PHONY: install
install: oracle.so
	mkdir -p $(PLUGINS_DIR)
	cp $< $(PLUGINS_DIR)/$<

test: install
	$(MAKE) -C tests all

clean:
	rm -f *.d *.o oracle.so

