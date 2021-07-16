#include "RDT/RDTHeader.hpp"
#include "Algorithm/crc.hpp"
#include <iomanip>
#include <sstream>

std::stringstream read(std::stringstream& buffer, const std::size_t& bytes){
  char chunk[bytes + 1];
  buffer.read(chunk, bytes);
  std::stringstream item;
  item << chunk;
  return item;
}

template<typename T>
T asWord(std::stringstream item, std::ios_base &(*base)(std::ios_base &)){
  uint64_t temp;
  item >> base >> temp;
  return static_cast<T>(temp);
}

template<typename T>
std::string fixFormatWord(T item, std::ios_base &(*base)(std::ios_base &)){
  std::stringstream buffer;
  uint64_t temp = static_cast<uint64_t>(item);
  buffer << base << temp;
  return std::string().assign(sizeof(T)*2 - buffer.str().length(), '0') + buffer.str();
}

bool net::RDTHeader::isCorrupted(){
  return corruption;
}

const std::string& net::RDTHeader::getData(){
  return data;
}

const net::RDTHeader::Type& net::RDTHeader::getPacketType(){
  return packet_t;
}

const uint32_t& net::RDTHeader::getSeqNumber(){
  return seqNumber;
}

const uint32_t& net::RDTHeader::getAckNumber(){
  return ackNumber;
}

const uint16_t& net::RDTHeader::getWindowSize(){
  return windowSize;
}

const uint16_t& net::RDTHeader::getRemainderPacketsCount(){
  return remainderPackets;
}

void net::RDTHeader::setData(const std::string& rawData){
  data = rawData;
}

void net::RDTHeader::setType(const Type& type){
  packet_t = type;
}

void net::RDTHeader::setSeqNumber(const uint32_t& seq){
  seqNumber = seq;
}

void net::RDTHeader::setAckNumber(const uint32_t& ack){
  ackNumber = ack;
}

void net::RDTHeader::setWindowSize(const uint16_t& wSize){
  windowSize = wSize;
}

void net::RDTHeader::setRemainderPacketsCount(const uint16_t& count){
  remainderPackets = count;
}

namespace net{
RDTHeader& operator<<(RDTHeader& packet, const std::string& rawPacket){
  std::stringstream rawPacketBuffer;
  rawPacketBuffer << rawPacket;
  packet.checksum = asWord<uint32_t>(read(rawPacketBuffer, 8), std::hex);
  if(!(packet.corruption = !(crypto::crc32(rawPacketBuffer.str()) ^ packet.checksum))){
    packet.seqNumber = asWord<uint32_t>(read(rawPacketBuffer, 8), std::hex);
    packet.ackNumber = asWord<uint32_t>(read(rawPacketBuffer, 8), std::hex);
    packet.packet_t = asWord<RDTHeader::Type>(read(rawPacketBuffer, 1), std::dec);
    packet.remainderPackets = asWord<uint16_t>(read(rawPacketBuffer, 4), std::hex);
    packet.windowSize = asWord<uint16_t>(read(rawPacketBuffer, 4), std::hex);
    uint16_t dataLength = asWord<uint16_t>(read(rawPacketBuffer, 4), std::hex);
    if(dataLength)
      packet.data = read(rawPacketBuffer, dataLength).str();
  }
  return packet;
}

const RDTHeader& operator>>(const RDTHeader& packet, std::string& formatedPacket){
  std::stringstream formated, checksumTest;
  checksumTest << fixFormatWord<uint32_t>(packet.seqNumber, std::hex);
  checksumTest << fixFormatWord<uint32_t>(packet.ackNumber, std::hex);
  checksumTest << packet.packet_t;
  checksumTest << fixFormatWord<uint16_t>(packet.remainderPackets, std::hex);
  checksumTest << fixFormatWord<uint16_t>(packet.windowSize, std::hex);
  checksumTest << fixFormatWord<uint16_t>(packet.data.length(), std::hex);
  std::string padding;
  padding.assign(MAX_DGRAM_SIZE - (int64_t)(packet.data.length() + checksumTest.str().length()), '0');
  checksumTest << packet.data << padding;
  formated << fixFormatWord<uint32_t>(crypto::crc32(checksumTest.str()), std::hex) << checksumTest.str();
  formatedPacket = formated.str();
  return packet;
}

std::ostream& operator<<(std::ostream& out, RDTHeader& packet) {
  if(!packet.isCorrupted()){
    std::string sType = "";
    switch (packet.getPacketType()){
    case RDTHeader::Type::Data:
      sType = "DAT";
      break;
    case RDTHeader::Type::Acknowledgement:
      sType = "ACK";
      break;
    case RDTHeader::Type::Synchronization:
      sType = "SYN";
      break;
    case RDTHeader::Type::Finalization:
      sType = "FIN";
      break;
    case RDTHeader::Type::Rejection:
      sType = "REJ";
      break;
    }
    out << "+------------------------------------+\n";
    out << '|' << std::setw(22) << fixFormatWord<uint32_t>(packet.checksum, std::hex) << std::setw(16) << "|\n";
    out << "+------------------------------------+\n";
    out << '|' << std::setw(22) << fixFormatWord<uint32_t>(packet.seqNumber, std::hex) << std::setw(16) << "|\n";
    out << "+------------------------------------+\n"; 
    out << '|' << std::setw(22) << fixFormatWord<uint32_t>(packet.ackNumber, std::hex) << std::setw(16) << "|\n";
    out << "+---+----------+----------+----------+\n";
    out << '|' << sType << '|' << std::setw(10) << packet.remainderPackets << '|' << std::setw(10) << 
    packet.windowSize << '|' << std::setw(10) << packet.data.length() << "|\n";
    out << "+---+----------+----------+----------+\n";
    out << "Padding length: " << MAX_DGRAM_SIZE - packet.data.length() - 37 << std::endl;
    if(packet.data.length()){
      out << "Message(sample[20]): " << packet.data.substr(0, 20);
      if(packet.data.length() > 20)
        out << "(...)\n";
    }
    else
      out << "Message(EMPTY).\n";
  }
  else
    std::cout << "[Error]: CORRUPTED PACKET\n";
  return out;
}
}