#ifndef UDP_SOCKET_HPP_
#define UDP_SOCKET_HPP_

#include <iostream>
#include <string>

namespace net{

  enum Status{Done, Error};
  enum TimerAns{Success, TimeOut, Error, TimerNone};

  class UdpSocket{
  private:
    int32_t socketId;
    std::string localIp;
    uint16_t localPort;
    bool blocking;
  public:
    UdpSocket();
    ~UdpSocket();
    Status bind(const uint16_t& localPort);
    void unbind();
    Status send(const std::string& message, const std::string& remoteAddress, const uint16_t& remotePort, const std::size_t bytes);
    Status receive(std::string& message, std::string& remoteAddress, uint16_t& remotePort, const std::size_t bytes);

    const uint16_t& getLocalPort() const;
    const std::string& getLocalIp() const;

    void setBlocking(bool enabled);

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;
    UdpSocket& operator=(UdpSocket&&) = delete;

    friend TimerAns waitResponse(UdpSocket& socket, const int32_t& milliseconds);
  };

}

#endif//UDP_SOCKET_HPP_