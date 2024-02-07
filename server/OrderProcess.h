#ifndef _ORDER_PROCESS_H
#define _ORDER_PROCESS_H
#include "Server.h"
#include "sql_function.h"
#include "Warehouse.h"

void parseOrder(string msg, int client_fd);
int selectWareHouse(const vector<Warehouse> & whList, const Order & order);
void processOrder(const Order& order); 
void processPurchaseMore(APurchaseMore r);
void processPacked(APacked r);
void processLoaded(ALoaded r);
void processTruckArrived(UTruckArrive r);
void processUDelivered(UDelivered r);
void pushWorldQueue(const ACommands& acommand, int seqNum);
void pushUpsQueue(const AUCommand& aucommand, int seqNum);
void packOrder(Order order);
void callATruck(Order order);

#endif
