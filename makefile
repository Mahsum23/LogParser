logparser: logparser.cpp progress_bar.o
	g++ -o logparser logparser.cpp progress_bar.o -std=c++17

progress_bar.o: progress_bar.cpp progress_bar.hpp
	g++ -c progress_bar.cpp -o progress_bar.o

