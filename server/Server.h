#ifndef _SERVER_H
#define _SERVER_H

#include <errno.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <pqxx/pqxx>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "./protobuf/AUprotocolV4.pb.h"
#include "./protobuf/world_amazon.pb.h"
#include "AResponseHandler.h"
#include "AUResponseHandler.h"
#include "Order.h"
#include "ThreadSafe_queue.h"
#include "Warehouse.h"
#include "exception.h"
#include "sql_function.h"

using namespace std;
using namespace pqxx;

const int MAX_SEQNUM = 65536;

class Server {
  // singleton pattern member and member funtions
 public:
  typedef std::shared_ptr<Server> Ptr;
  ~Server() { std::cout << "destructor called!" << std::endl; }
  Server(Server &) = delete;
  Server & operator=(const Server &) = delete;
  static Ptr get_instance();

 private:
  static Ptr m_instance_ptr;
  static mutex m_mutex;

  // Server member functions and its related fields
 private:
  Server();
  void acceptOrderRequest();
  void initializeWorld();
  void getWorldIDFromUPS();
  void keepSendingMsgToWorld();
  void keepSendingMsgToUps();
  void keepReceivingMsgFromUps();
  void keepReceivingMsgFromWorld();

 private:
  string webPortNum;
  string worldHostName;
  string worldPortNum;
  string upsHostName;
  string upsPortNum;
  int worldID;
  int n_warehouse;           // number of warehouse
  int wh_distance;           // distance between neighbour warehouse
  vector<Warehouse> whList;  // list of warehouse
  int ups_fd;
  int world_fd;

 public:
  // maintain seqNum from amazon
  vector<bool> seqNumTable;  // record whether commands with specific seqNum is acked.
  size_t curSeqNum;          // next seqNum to be used.
  mutex seqNum_lck;          // mutex used to lock seqNum

  // Records whether a response with a specific sequence number is executed
  // if seqNum is in executeTable, this response has been executed.
  unordered_set<int> executeTable_World;
  unordered_set<int> executeTable_Ups;

  //message queue, transfer message to sending threads
  ThreadSafe_queue<ACommands> worldQueue;
  ThreadSafe_queue<AUCommand> upsQueue;

  //order queue. save orders for later processing
  queue<Order> orderQueue;
  mutex order_lck;

 public:
  void run();
  vector<Warehouse> getWhList() { return whList; }
  size_t getSeqNum();
  static connection * connectDB(string dbName, string userName, string password);
  static void initializeDB(connection * C);
  static void disConnectDB(connection * C);
};

#endif
