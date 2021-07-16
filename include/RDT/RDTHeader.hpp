#ifndef RDT_HEADER_HPP_
#define RDT_HEADER_HPP_

#include <iostream>
#include <string>

/*
>> RDT HEADER STRUCTURE:
¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
      HEX16 = (unsigned short hexadecimal base) -> 4 bytes ; ex: afe5
      HEX32 = (unsigned int   hexadecimal base) -> 8 bytes ; ex: cc9ea073
      ┌──────────────┬──────────────────┬────────────┬────────────────┐
 UDP: │  source port │ destination port │   length   │    checksum    │
(UDT) └──────────────┴──────────────────┴────────────┴────────────────┘
          ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   
      ┌───────────────────────────────────────────────────────────────┐ 
      │                        CRC32-checksum                         │ CRC32: HEX32                              (8)
      ├───────────────────────────────────────────────────────────────┤
      │                        sequence number                        │ SEQ: HEX32                                (8)
      ├───────────────────────────────────────────────────────────────┤
 RDT: │                     acknowledgement number                    │ ACK: HEX32                                (8)
      ├────────────┬────────────────────┬───────────────┬─────────────┤
      │ flag(Type) │ #remainder packets │  window size  │ data length │ flag: 1 byte, #remainder packets: HEX16   (5)
      ├────────────┴────────────────────┴───────────────┴─────────────┤ window: HEX16, data length: HEX16         (8)
      │                                                               │
      │               data                 ┌──────────────────────────┤ HEADER = 8 + 8 + 8 + 5 + 8 = 37 bytes
      │                                    │          padding         │ MAX_DGRAM_SIZE = 1000
      └────────────────────────────────────┴──────────────────────────┘ DATA + PADDING = 1000 - 37 = 963 free bytes

¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
charset: {│,┌,┘,─,┴,┬,┼,└,┐,├,┤,↓}
¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
*/

namespace net{
    
  const uint16_t MAX_DGRAM_SIZE = 1000;

  class RDTHeader{
  public:
    enum Type{Data, Acknowledgement, Synchronization, Finalization, Rejection};
  private:
    std::string data;
    uint32_t seqNumber, ackNumber, checksum;
    uint16_t windowSize, remainderPackets;
    Type packet_t;
    bool corruption;
  public:
    bool isCorrupted();
    const std::string& getData();
    const Type& getPacketType();
    const uint32_t& getSeqNumber();
    const uint32_t& getAckNumber();
    const uint16_t& getWindowSize();
    const uint16_t& getRemainderPacketsCount();

    void setData(const std::string& rawData);
    void setType(const Type& type);
    void setSeqNumber(const uint32_t& seq);
    void setAckNumber(const uint32_t& ack);
    void setWindowSize(const uint16_t& wSize);
    void setRemainderPacketsCount(const uint16_t& count);

    friend RDTHeader& operator<<(RDTHeader& packet, const std::string& rawPacket);
    friend const RDTHeader& operator>>(const RDTHeader& packet, std::string& formatedPacket);
    friend std::ostream& operator<<(std::ostream& out, RDTHeader& packet);
  };

}

#endif//RDT_HEADER_HPP_