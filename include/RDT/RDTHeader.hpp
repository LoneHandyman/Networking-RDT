#ifndef RDT_HEADER_HPP_
#define RDT_HEADER_HPP_

#include <iostream>
#include <string>

/*
>> RDT 4.0 HEADER STRUCTURE:
¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
      HEX16 = (unsigned short | hexadecimal base) -> 4 bytes ; ex: afe5
      HEX32 = (unsigned int   | hexadecimal base) -> 8 bytes ; ex: cc9ea073
      DEC   = (unsigned num   |     decimal base) -> 1 byte : [0,1,2,3,4,5,6,7,8,9]
      HEX   = (unsigned num   | hexadecimal base) -> 1 byte : [0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f]
      ┌───────────────┬──────────────────┬──────────────┬────────────────┐
 UDP: │  source port  │ destination port │    length    │    checksum    │
(UDT) └───────────────┴──────────────────┴──────────────┴────────────────┘
            ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓   
      ┌──────────────────────────────────────────────────────────────────┐ 
      │                          CRC32-checksum                          │ CRC32: HEX32                              (8)
      ├──────────────────────────────────────────────────────────────────┤
      │                          sequence number                         │ SEQ: HEX32                                (8)
      ├──────────────────────────────────────────────────────────────────┤
 RDT: │                       acknowledgement number                     │ ACK: HEX32                                (8)
      ├──────┬───────┬────────────────────┬───────────────┬──────────────┤
      │ Type │ State │ #remainder packets │  window size  │  data length │ Type: DEC, State: HEX                     (2)
      ├──────┴───────┴────────────────────┴───────────────┴──────────────┤ #remainder packets: HEX32                 (8)
      │                                                                  │ window: HEX16, data length: HEX16         (8)
      │               data                 ┌─────────────────────────────┤ HEADER = 8 + 8 + 8 + 2 + 8 + 8 = 42 bytes
      │                                    │          padding            │ MAX_DGRAM_SIZE = 1000
      └────────────────────────────────────┴─────────────────────────────┘ DATA + PADDING = 1000 - 42 = 958 free bytes

¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
charset: {│,┌,┘,─,┴,┬,┼,└,┐,├,┤,↓}
¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
*/

#define RDT_HEADER_BYTE_SIZE 42

namespace net{
    
  const uint16_t PAYLOAD = 1000;

  class RDTHeader{
  public:
    enum Type{Data, Acknowledgement, Synchronization, Finalization, Rejection, None};
    enum State{SYN_SENT, SYN_RECV, ESTABLISHED, RETRANSMISSION, WAIT_ACK, FIN_WAIT1, 
               FIN_WAIT2, CLOSE_WAIT, LAST_ACK, TIME_WAIT, CLOSED, LISTEN, NONE};
  private:
    std::string data;
    std::size_t dataLength;
    uint32_t seqNumber, ackNumber, checksum, remainderPackets;
    uint16_t windowSize;
    Type packet_t;
    State packet_s;
    bool corruption;

    void reset();
  public:
    bool isCorrupted();
    const std::string& getData();
    const Type& getPacketType();
    const State& getPacketState();
    const uint32_t& getSeqNumber();
    const uint32_t& getAckNumber();
    const uint32_t& getRemainderPacketsCount();
    const uint16_t& getWindowSize();

    void setData(const std::string& rawData);
    void setType(const Type& type);
    void setState(const State& state);
    void setSeqNumber(const uint32_t& seq);
    void setAckNumber(const uint32_t& ack);
    void setRemainderPacketsCount(const uint32_t& count);
    void setWindowSize(const uint16_t& wSize);

    friend RDTHeader& operator<<(RDTHeader& packet, const std::string& rawPacket);
    friend const RDTHeader& operator>>(const RDTHeader& packet, std::string& formatedPacket);
    friend std::ostream& operator<<(std::ostream& out, RDTHeader& packet);
  };

}

#endif//RDT_HEADER_HPP_