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
        dataHeader.setType(RDTHeader::Type::Data);
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
        if(!ackHeader.isCorrupted() && ackHeader.getPacketType() == RDTHeader::Type::Acknowledgement){
          sendBase = std::max(ackHeader.getAckNumber() + 1, sendBase);
          window = ackHeader.getWindowSize();
        }
      }
      else if(timerStatus == TimerAns::TimeOut){
        for(std::size_t packetIdx = sendBase; packetIdx < nextSeqNum; ++packetIdx){
          RDTHeader dataHeader;
          dataHeader.setData(dataBuffered[packetIdx]);
          dataHeader.setSeqNumber(packetIdx);
          dataHeader.setType(RDTHeader::Type::Data);
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
  if(udt_socket != nullptr){
    message.clear();
    uint32_t nextSeqNum = 0, remainderPackets = 1;
    std::string packet, ack, currRemoteIp;
    uint16_t currRemotePort;
    udt_socket->setBlocking(true);
    while(remainderPackets){
      if(udt_socket->receive(packet, currRemoteIp, currRemotePort, PAYLOAD) != Status::Done)
        return Status::Error;
      RDTHeader dataHeader;
      dataHeader << packet;
      if(!dataHeader.isCorrupted() && dataHeader.getSeqNumber() == nextSeqNum){
        message += dataHeader.getData();
        remainderPackets = dataHeader.getRemainderPacketsCount();
        RDTHeader ackHeader;
        ackHeader.setAckNumber(nextSeqNum);
        ackHeader.setType(RDTHeader::Type::Acknowledgement);
        ackHeader >> ack;
        if(udt_socket->send(ack, remoteIp, remotePort, PAYLOAD) != Status::Done)
          return Status::Error;
        ++nextSeqNum;
      }
      else{
        RDTHeader ackHeader;
        ackHeader.setAckNumber(nextSeqNum - 1);
        ackHeader.setType(RDTHeader::Type::Acknowledgement);
        ackHeader >> ack;
        if(udt_socket->send(ack, currRemoteIp, currRemotePort, PAYLOAD) != Status::Done)
          return Status::Error;
      }
    }
    return Status::Done;
  }
  return Status::Error;
}