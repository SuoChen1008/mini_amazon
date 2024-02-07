#include "Server.h"

#include "OrderProcess.h"
#include "protobufCommunication.hpp"
#include "socket.h"
#include "sql_function.h"

/* ------------------------ "server singleton pattern" ------------------------ */
Server::Ptr Server::m_instance_ptr = nullptr;
mutex Server::m_mutex;

Server::Ptr Server::get_instance() {
  if (m_instance_ptr == nullptr) {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_instance_ptr == nullptr) {
      m_instance_ptr = std::shared_ptr<Server>(new Server);
    }
  }
  return m_instance_ptr;
}

/* ------------------------ "server initialize functions" ------------------------ */
Server::Server() {
  cout << "initialize server configuration...." << endl;

  unique_ptr<connection> C(Server::connectDB("mini_amazon", "postgres", "passw0rd"));
  setTableDefaultValue(C.get());
  Server::disConnectDB(C.get());

  curSeqNum = 0;
  for (size_t i = 0; i < MAX_SEQNUM; i++) {
    seqNumTable.push_back(false);
  }

  n_warehouse = 9;  // should be an odd number for symetric
  wh_distance = 20;
  webPortNum = "9999";
  worldHostName = "vcm-26124.vm.duke.edu";  // for changhao testing
  //worldHostName = "vcm-25941.vm.duke.edu";   //for junfeng testing
  //worldHostName = "vcm-24717.vm.duke.edu";  // for fuzhengqi testing
  worldPortNum = "23456";

  //upsHostName = "vcm-24717.vm.duke.edu"; //for fuzhengqi testing
  upsPortNum = "8888";

  worldID = -1;  // set to -1 in testing
}

/* ------------------------ "server runtime functions" ------------------------ */
/*
  Initialize the connection to server and ups and get essential infomation. Then it 
  will keep listening the front-end web to receive order request.
*/
void Server::run() {
  try {
    getWorldIDFromUPS();  //for real testing
    initializeWorld();
    thread tI_world(&Server::keepReceivingMsgFromWorld, this);
    thread tI_ups(&Server::keepReceivingMsgFromUps, this);
    thread tO_world(&Server::keepSendingMsgToWorld, this);
    thread tO_ups(&Server::keepSendingMsgToUps, this);
    acceptOrderRequest();
  }
  catch (const std::exception & e) {
    std::cerr << e.what() << '\n';
    close(ups_fd);
    close(world_fd);
    return;
  }
}

/*
  Connect to UPS to get worldID. 
*/
void Server::getWorldIDFromUPS() {
  ups_fd = clientRequestConnection(upsHostName, upsPortNum);
  string msg = recvMsg(ups_fd);

  worldID = stoi(msg);
  cout << "get worldID = " << worldID << " from UPS.\n";
}

/*
  Connect to a new world, and then initialize warehouses for amazon.
  If connect unsuccessfully, it will throw exception. If worldID = -1, 
  server will create a new world and set worldID to the new worldID.
*/
void Server::initializeWorld() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  //connect to world
  world_fd = clientRequestConnection(worldHostName, worldPortNum);

  //set AConnect command
  AConnect ac;
  for (int i = 0; i <= n_warehouse / 2; i++) {
    if (i == 0) {
      AInitWarehouse * w = ac.add_initwh();
      w->set_id(i);
      w->set_x(0);
      w->set_y(0);
      whList.push_back(Warehouse(0, 0, i));
    }
    else {
      AInitWarehouse * w1 = ac.add_initwh();
      w1->set_id(i);
      w1->set_x(0.707 * wh_distance * i);
      w1->set_y(0.707 * wh_distance * i);
      whList.push_back(Warehouse(0.707 * wh_distance * i, 0.707 * wh_distance * i, i));

      AInitWarehouse * w2 = ac.add_initwh();
      w2->set_id(i + n_warehouse / 2);
      w2->set_x(-1 * 0.707 * wh_distance * i);
      w2->set_y(-1 * 0.707 * wh_distance * i);
      whList.push_back(Warehouse(-1 * 0.707 * wh_distance * i,
                                 -1 * 0.707 * wh_distance * i,
                                 i + n_warehouse / 2));
    }
  }
  if (worldID != -1) {
    ac.set_worldid(worldID);
  }
  ac.set_isamazon(true);

  //show warehouses
  for (auto & w : whList) {
    w.show();
  }

  //send AConnect command
  unique_ptr<socket_out> out(new socket_out(world_fd));
  if (sendMesgTo<AConnect>(ac, out.get()) == false) {
    throw MyException("fail to send AConnect to world.");
  }

  //receive AConnected command
  AConnected aced;
  unique_ptr<socket_in> in(new socket_in(world_fd));
  if (recvMesgFrom<AConnected>(aced, in.get()) == false) {
    throw MyException("fail to recv AConnected from world.");
  }

  //check AConnected command
  if (aced.result() != "connected!") {
    throw MyException("fail to initialize the world.");
  }
  worldID = aced.worldid();
  cout << "succefully connect to world and initialize warehouses.\n";
}

/*
  IO function. Server keep receiving order requests from front-end web.
  It will throw exception when server_socket create unsuccessfully. For each 
  order request, send it to task queue for processing.
*/
void Server::acceptOrderRequest() {
  // create server socket, listen to port.
  int server_fd = createServerSocket(webPortNum);

  // keep receving orders request from front-end web.
  while (1) {
    // wait to accept connection.
    int client_fd;
    string clientIP;
    try {
      client_fd = serverAcceptConnection(server_fd, clientIP);
    }
    catch (const std::exception & e) {
      std::cerr << e.what() << '\n';
      continue;
    }

    // receive request.
    string msg;
    try {
      msg = recvMsg(client_fd);
    }
    catch (const std::exception & e) {
      std::cerr << e.what() << '\n';
      continue;
    }

    // put request into task queue, using thread pool
    thread t(parseOrder, msg, client_fd);
    t.detach();
  }
}

/* ------------------------ "DB related functions" ------------------------ */
/*
  connect to specific database using the given userName and passWord, reutnr 
  the connection* C. It will throw an exception if fails. 
*/
connection * Server::connectDB(string dbName, string userName, string password) {
  // connection * C = new connection("dbname=" + dbName + " user=" + userName +
  //                                 " password=" + password);  // use in real sys
  connection * C =
      new connection("host=db port=5432 dbname=" + dbName + " user=" + userName +
                     " password=" + password);  // use in docker
  if (C->is_open()) {
    //cout << "Opened database successfully: " << C->dbname() << endl;
  }
  else {
    throw MyException("Can't open database.");
  }

  return C;
}

/*
  initialize database when server run at the first time. It will drop all the existing tables,
  and then create new ones.
*/
void Server::initializeDB(connection * C) {
  dropAllTable(C);
  createTable(C, "./sql/table.sql");
}

/*
  close the connection to database.
*/
void Server::disConnectDB(connection * C) {
  C->disconnect();
}

/* ------------------------ "IO Functions" ------------------------ */
/*
  keep receiving response from the world.
*/
void Server::keepReceivingMsgFromWorld() {
  unique_ptr<socket_in> world_in(new socket_in(world_fd));
  while (1) {
    AResponses r;
    if (recvMesgFrom<AResponses>(r, world_in.get()) == false) {
      continue;
    }
    AResponseHandler h(r);
    h.handle();
  }
}

/*
  keep receiving response from the ups. 
*/
void Server::keepReceivingMsgFromUps() {
  unique_ptr<socket_in> ups_in(new socket_in(ups_fd));
  while (1) {
    UACommand r;
    if (recvMesgFrom<UACommand>(r, ups_in.get()) == false) {
      continue;
    }
    AUResponseHandler h(r);
    h.handle();
  }
}

/*
  keep sending message from worldQueue to world. this function will block when
  the queue is empty. 
*/
void Server::keepSendingMsgToWorld() {
  unique_ptr<socket_out> out(new socket_out(world_fd));
  while (1) {
    ACommands msg;
    worldQueue.wait_and_pop(msg);
    if (sendMesgTo(msg, out.get()) == false) {
      throw MyException("fail to send message in world.");
    }
  }
}

/*
  keep sending message from upsQueue to UPS. this function will block when
  the queue is empty. 
*/
void Server::keepSendingMsgToUps() {
  unique_ptr<socket_out> out(new socket_out(ups_fd));
  while (1) {
    AUCommand msg;
    upsQueue.wait_and_pop(msg);
    if (sendMesgTo(msg, out.get()) == false) {
      throw MyException("fail to send message in ups.");
    }
  }
}

/* ------------------------ "server SeqNum related functions" ------------------------ */
/*
  return current sequence number and curSeqNum self increase. thw whole function workd atomically. 
*/
size_t Server::getSeqNum() {
  lock_guard<mutex> lck(seqNum_lck);
  size_t res = curSeqNum;
  curSeqNum++;
  return res;
}