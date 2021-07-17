#ifndef RDT_SOCKET_HPP_
#define RDT_SOCKET_HPP_

#include "RDTHeader.hpp"
#include "../UDPWrapper/UdpSocket.hpp"
#include <memory>

/*

>> RDT 4.0 FEATURES:
¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
 - Go Back-N
 - Fast Retransmission
 - Controlled RTT
*/

namespace net{

  class RDTSocket{
  private:
    const uint16_t chunkSize = PAYLOAD - RDT_HEADER_BYTE_SIZE;

    std::unique_ptr<UdpSocket> udt_socket;
    uint32_t initSeqNumber;
    uint16_t window;
    uint8_t windowScale;
    RDTHeader::State currentState;
    std::string remoteIp;
    uint16_t remotePort;
  public:
    RDTSocket();
    ~RDTSocket();
    Status connect();
    Status passiveDisconnect();
    Status activeDisconnect();
    Status send(const std::string& message);
    Status receive(std::string& message);
  };

}

#endif//RDT_SOCKET_HPP_