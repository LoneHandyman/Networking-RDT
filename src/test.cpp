#include "RDT/RDTHeader.hpp"

int main(){
  net::RDTHeader header, header2;
  header.setAckNumber(5000);
  header.setSeqNumber(1205);
  header.setType(net::RDTHeader::Type::Synchronization);
  header.setState(net::RDTHeader::State::TIME_WAIT);
  header.setRemainderPacketsCount(UINT32_MAX);
  header.setWindowSize(1024);
  header.setData("PORT:30020 - IP:196.30.15.16");
  std::string packet;
  header >> packet;
  std::cout << packet << std::endl;
  header2 << packet;
  std::cout << header2 << std::endl;
  return 0;
}