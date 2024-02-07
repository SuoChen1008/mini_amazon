#ifndef _AURESPONSEHANDLER_H
#define _AURESPONSEHANDLER_H

#include <vector>

#include "OrderProcess.h"
#include "Server.h"
#include "protobufCommunication.hpp"

using namespace std;

class AUResponseHandler {
 private:
  vector<UTruckArrive> utruckarrives;
  vector<UDelivered> udelivereds;
  vector<int> seqNums;

 public:
  AUResponseHandler(const UACommand & r);
  ~AUResponseHandler() {}
  void handle();

 private:
  bool checkExecutedAndRecordIt(int seqNum);
};

#endif