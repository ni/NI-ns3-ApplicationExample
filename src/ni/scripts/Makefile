all: udp-perf-test

udp-perf-test: udp-perf-test_v1.o udp-perf-test_v2.o
	g++ udp-perf-test_v1.o -L/usr/lib/x86_64-linux-gnu -pthread -lboost_system -lboost_thread -lboost_program_options -std=c++11 -o udp-perf-test_v1
	g++ udp-perf-test_v2.o -L/usr/lib/x86_64-linux-gnu -pthread -lboost_system -lboost_thread -lboost_program_options -std=c++11 -o udp-perf-test_v2

udp-perf-test_v1.o: udp-perf-test_v1.cc
	g++ -std=c++11 -c udp-perf-test_v1.cc -I/usr/include/boost

udp-perf-test_v2.o: udp-perf-test_v2.cc
	g++ -std=c++11 -c udp-perf-test_v2.cc -I/usr/include/boost

clean:
	rm -f *.o
