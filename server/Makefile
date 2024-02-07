GDB = -g
THREAD = -pthread
PQXX = -lpqxx -lpq
GPB = `pkg-config --cflags --libs protobuf`
PROTO_CC = ./protobuf/world_amazon.pb.cc  ./protobuf/AUprotocolV4.pb.cc
PROTO_H = ./protobuf/world_amazon.pb.h ./protobuf/AUprotocolV4.pb.h
VERSION = -std=c++11
TOOLS_H = ThreadSafe_queue.h exception.h socket.h protobufCommunication.hpp
TOOLS_CPP = socket.cpp 
DATA_STRUCTURE_H = Order.h Warehouse.h 


all: server

server:  Server.h AResponseHandler.h OrderProcess.h sql_function.h OrderProcess.h AUResponseHandler.h $(PROTO_H) $(TOOLS_H) $(DATA_STRUCTURE_H)
	g++ -o server main.cpp Server.cpp sql_function.cpp AResponseHandler.cpp OrderProcess.cpp AUResponseHandler.cpp $(TOOLS_CPP) $(PROTO_CC) $(PQXX) $(GDB) $(THREAD) $(GPB) $(VERSION)

.PHONY:
	clean
clean:
	rm -rf *.o server  