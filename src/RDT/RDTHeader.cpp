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

void net::RDTHeader::reset(){
  corruption = false;
  data = "";
  dataLength = 0;
  packet_t = Type::None;
  packet_s = State::NONE;
  seqNumber = ackNumber = remainderPackets = checksum = 0;
  windowSize = 0;
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

const net::RDTHeader::State& net::RDTHeader::getPacketState(){
  return packet_s;
}

const uint32_t& net::RDTHeader::getSeqNumber(){
  return seqNumber;
}

const uint32_t& net::RDTHeader::getAckNumber(){
  return ackNumber;
}

const uint32_t& net::RDTHeader::getRemainderPacketsCount(){
  return remainderPackets;
}

const uint16_t& net::RDTHeader::getWindowSize(){
  return windowSize;
}

void net::RDTHeader::setData(const std::string& rawData){
  data = rawData.substr(0, PAYLOAD - RDT_HEADER_BYTE_SIZE);
  dataLength = data.length();
}

void net::RDTHeader::setType(const Type& type){
  packet_t = type;
}

void net::RDTHeader::setState(const State& state){
  packet_s = state;
}

void net::RDTHeader::setSeqNumber(const uint32_t& seq){
  seqNumber = seq;
}

void net::RDTHeader::setAckNumber(const uint32_t& ack){
  ackNumber = ack;
}

void net::RDTHeader::setRemainderPacketsCount(const uint32_t& count){
  remainderPackets = count;
}

void net::RDTHeader::setWindowSize(const uint16_t& wSize){
  windowSize = wSize;
}

namespace net{
RDTHeader& operator<<(RDTHeader& packet, const std::string& rawPacket){
  packet.reset();
  std::stringstream rawPacketBuffer;
  rawPacketBuffer << rawPacket;
  uint32_t expectedChecksum = asWord<uint32_t>(read(rawPacketBuffer, 8), std::hex);
  packet.checksum = crypto::crc32(rawPacket.substr(8, PAYLOAD - 8));
  if(!(packet.corruption = (packet.checksum != expectedChecksum))){
    packet.seqNumber = asWord<uint32_t>(read(rawPacketBuffer, 8), std::hex);
    packet.ackNumber = asWord<uint32_t>(read(rawPacketBuffer, 8), std::hex);
    packet.packet_t = asWord<RDTHeader::Type>(read(rawPacketBuffer, 1), std::dec);
    packet.packet_s = asWord<RDTHeader::State>(read(rawPacketBuffer, 1), std::hex);
    packet.remainderPackets = asWord<uint32_t>(read(rawPacketBuffer, 8), std::hex);
    packet.windowSize = asWord<uint16_t>(read(rawPacketBuffer, 4), std::hex);
    packet.dataLength = asWord<uint16_t>(read(rawPacketBuffer, 4), std::hex);
    if(packet.dataLength)
      packet.data = read(rawPacketBuffer, packet.dataLength).str().substr(0, packet.dataLength);
  }
  return packet;
}

const RDTHeader& operator>>(const RDTHeader& packet, std::string& formatedPacket){
  if(packet.packet_s != RDTHeader::State::NONE){
    formatedPacket.clear();
    std::stringstream formated, checksumTest;
    checksumTest << fixFormatWord<uint32_t>(packet.seqNumber, std::hex);
    checksumTest << fixFormatWord<uint32_t>(packet.ackNumber, std::hex);
    checksumTest << std::dec << packet.packet_t;
    checksumTest << std::hex << packet.packet_s;
    checksumTest << fixFormatWord<uint32_t>(packet.remainderPackets, std::hex);
    checksumTest << fixFormatWord<uint16_t>(packet.windowSize, std::hex);
    checksumTest << fixFormatWord<uint16_t>(packet.dataLength, std::hex);
    std::string padding;
    padding.assign(PAYLOAD - (int64_t)(packet.dataLength + RDT_HEADER_BYTE_SIZE), '0');
    checksumTest << packet.data << padding;
    formated << fixFormatWord<uint32_t>(crypto::crc32(checksumTest.str()), std::hex) << checksumTest.str();
    formatedPacket = formated.str();
  }
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
    out << "|------------------------------------|\n";
    out << '|' << std::setw(22) << fixFormatWord<uint32_t>(packet.seqNumber, std::hex) << std::setw(16) << "|\n";
    out << "|------------------------------------|\n"; 
    out << '|' << std::setw(22) << fixFormatWord<uint32_t>(packet.ackNumber, std::hex) << std::setw(16) << "|\n";
    out << "|---+---+------------+-------+-------|\n";
    out << '|' << sType << '|' << std::setw(2) << std::dec << packet.packet_s << " |" << std::setw(11) << 
    std::dec << packet.remainderPackets << " |" << std::setw(6) << std::dec << packet.windowSize << " |" << 
    std::setw(6) << std::dec << packet.dataLength << " |\n";
    out << "+---+---+------------+-------+-------+\n";
    out << "Padding length: " << std::dec << PAYLOAD - packet.dataLength - RDT_HEADER_BYTE_SIZE << std::endl;
    if(packet.data.length()){
      out << "Message(sample[20]): " << packet.data.substr(0, 20);
      if(packet.data.length() > 20)
        out << "(...)\n";
    }
    else
      out << "Message(EMPTY).\n";
  }
  else
    out << "[Error]: CORRUPTED PACKET\n";
  return out;
}
}