OPT_FLAGS = -O3
CXX = g++
CXX_FLAGS = $(OPT_FLAGS) --std=c++17 -Wall

LIBS = -llept -ltesseract
INCLUDES = -I./include/

src_dir = ./src/
dst_dir = ./
src = $(wildcard $(src_dir)*.cpp)
obj = $(patsubst %.cpp, %.o, $(src))

default: StripeReassembly
.PHONY: clean

StripeReassembly: $(obj)
	$(CXX) $< $(LIBS) -o $(dst_dir)$@

%.o: %.cpp
	$(CXX) $< $(CXX_FLAGS) $(INCLUDES) -c -o $@

clean:
	rm $(dst)StripeReassembly $(src_dir)*.o
