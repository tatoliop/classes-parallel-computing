include ../common/config.mk
BINS := hello_threads counter deadlock sum_threads counter_tsan

all: $(BINS)
%: %.cpp
	$(CXX) $(CXXFLAGS) -pthread $< -o $@
# ThreadSanitizer build of the racy counter (demo of automatic race detection)
counter_tsan: counter.cpp
	$(CXX) -std=c++17 -O1 -g -fsanitize=thread -I../common -pthread $< -o $@
clean:
	rm -f $(BINS)
.PHONY: all clean
