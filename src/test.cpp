#include "RDT/RDTHeader.hpp"

int main(){
  net::RDTHeader header, header2;
  header.setAckNumber(5000);
  header.setSeqNumber(1205);
  header.setType(net::RDTHeader::Type::Synchronization);
  header.setRemainderPacketsCount(6);
  header.setWindowSize(1024);
  header.setData("PORT");
  std::string packet;
  header >> packet;
  std::cout << packet << std::endl;
  header2 << packet;
  std::cout << header2 << std::endl;
  return 0;
}