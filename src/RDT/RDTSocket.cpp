#include "RDT/RDTSocket.hpp"
#include <chrono>
#include <vector>

net::RDTSocket::RDTSocket(){
  window = PAYLOAD - RDT_HEADER_BYTE_SIZE;
  windowScale = 0;
  initSeqNumber = 0;
}

net::RDTSocket::~RDTSocket(){
  udt_socket.reset();
}

net::Status net::RDTSocket::connect(){
  udt_socket = std::make_unique<UdpSocket>();
}

net::Status net::RDTSocket::passiveDisconnect(){

}

net::Status net::RDTSocket::activeDisconnect(){

}

net::Status net::RDTSocket::send(const std::string& message){
  if(udt_socket != nullptr){
    std::vector<std::string> dataBuffered;
    for(std::size_t offset = 0; offset < message.length(); offset += chunkSize){
      dataBuffered.push_back(message.substr(offset, chunkSize));
    }
    uint32_t sendBase, nextSeqNum;
    int32_t rtt = 200;
    sendBase = nextSeqNum = 0;
    std::string packet, ack, currRemoteIp;
    uint16_t currRemotePort;
    TimerAns timerStatus = TimerAns::TimerNone;
    udt_socket->setBlocking(false);
    std::chrono::high_resolution_clock::time_point delayStart, delayEnd;
    int32_t accumulativeTime = 0;
    while(sendBase < dataBuffered.size()){
      delayStart = std::chrono::high_resolution_clock::now();
      if(nextSeqNum < sendBase + window){
        RDTHeader dataHeader;
        dataHeader.setData(dataBuffered[nextSeqNum]);
        dataHeader.setSeqNumber(nextSeqNum);
        dataHeader.setRemainderPacketsCount(dataBuffered.size() - nextSeqNum - 1);
        dataHeader >> packet;
        if(udt_socket->send(packet, remoteIp, remotePort, PAYLOAD) != Status::Done)
          return Status::Error;
        if(nextSeqNum == sendBase){
          rtt = 200;
          accumulativeTime = 0;
        }
        ++nextSeqNum;
      }
      if(udt_socket->receive(ack, currRemoteIp, currRemotePort, PAYLOAD) == Status::Done || timerStatus == TimerAns::Success){
        RDTHeader ackHeader;
        ackHeader << ack;
        if(!ackHeader.isCorrupted()){
          sendBase = std::max(ackHeader.getAckNumber() + 1, sendBase);
          window = ackHeader.getWindowSize();
        }
      }
      else if(timerStatus == TimerAns::TimeOut){
        for(std::size_t packetIdx = sendBase; packetIdx < nextSeqNum; ++packetIdx){
          RDTHeader dataHeader;
          dataHeader.setData(dataBuffered[packetIdx]);
          dataHeader.setSeqNumber(packetIdx);
          dataHeader >> packet;
          if(udt_socket->send(packet, remoteIp, remotePort, PAYLOAD) != Status::Done)
            return Status::Error;
        }
        timerStatus = TimerAns::TimerNone;
      }
      if(nextSeqNum == sendBase + window)
        timerStatus = waitResponse(*udt_socket, rtt - accumulativeTime);
      delayEnd = std::chrono::high_resolution_clock::now();
      accumulativeTime += std::chrono::duration_cast<std::chrono::milliseconds>(delayEnd - delayStart).count();
    }
    return Status::Done;
  }
  return Status::Error;
}

net::Status net::RDTSocket::receive(std::string& message){

}