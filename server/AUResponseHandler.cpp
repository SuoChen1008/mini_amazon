#include "AUResponseHandler.h"

AUResponseHandler::AUResponseHandler(const UACommand & r) {
  for (int i = 0; i < r.arrive_size(); i++) {
    utruckarrives.push_back(std::move(r.arrive(i)));
    seqNums.push_back(r.arrive(i).seqnum());
  }
  for (int i = 0; i < r.delivered_size(); i++) {
    udelivereds.push_back(std::move(r.delivered(i)));
    seqNums.push_back(r.delivered(i).seqnum());
  }

  // record acks from ups
  for (int i = 0; i < r.acks_size(); i++) {
    Server::Ptr server = Server::get_instance();
    server->seqNumTable[r.acks(i)] = true;
  }
}

/*
  check whether given seqNum has been executed.If yes, return true,
  else return false. If given seqNum is not executed, record it in 
  the executed table.
*/
bool AUResponseHandler::checkExecutedAndRecordIt(int seqNum) {
  // check whether this response has been executed

  Server::Ptr server = Server::get_instance();
  auto it = server->executeTable_Ups.find(seqNum);

  // if not exists, insert seqNum in the set, else exit
  if (it == server->executeTable_Ups.end()) {
    server->executeTable_Ups.insert(seqNum);
    return false;
  }
  else {
    return true;
  }
}

/*
    use different threads to handle different type of responses, and ack those messages.
*/
void AUResponseHandler::handle() {
  // ACK responses to UPS.
  AUCommand aucommand;
  for (int i = 0; i < seqNums.size(); i++) {
    aucommand.add_acks(i);
    aucommand.set_acks(i, seqNums[i]);
  }
  Server::Ptr server = Server::get_instance();
  server->upsQueue.push(aucommand);

  // use different threads to handle different responses.
  for (auto r : utruckarrives) {
    if (checkExecutedAndRecordIt(r.seqnum()) == false) {
      thread t(processTruckArrived, r);
      t.detach();
    }
  }

  for (auto r : udelivereds) {
    if (checkExecutedAndRecordIt(r.seqnum()) == false) {
      thread t(processUDelivered, r);
      t.detach();
    }
  }
}