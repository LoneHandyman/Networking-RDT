#ifndef UDP_SOCKET_HPP_
#define UDP_SOCKET_HPP_

#include <iostream>
#include <string>

namespace net{
  const size_t MAX_DGRAM_SIZE = 1000;

  enum Status{Done, Connected, Disconnected, Error};

  class UdpSocket{
  private:
    int32_t socketId;
    std::string localIp;
    uint16_t localPort;
  public:
    UdpSocket();
    ~UdpSocket();
    Status bind(const uint16_t& localPort);
    void unbind();
    Status send(const std::string& message, const std::string& remoteAddress, const uint16_t& remotePort);
    Status receive(std::string& message, std::string& remoteAddress, uint16_t& remotePort);

    const uint16_t& getLocalPort() const;
    const std::string& getLocalIp() const;

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;
    UdpSocket& operator=(UdpSocket&&) = delete;
  };

}

#endif//UDP_SOCKET_HPP_