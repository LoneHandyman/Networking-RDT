#include "UDPWrapper/UdpSocket.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <algorithm>

net::UdpSocket::UdpSocket(){
  if((socketId = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    std::cerr << "[Socket Creation] : --FAILURE--\n";
  }
  blocking = true;
}

net::UdpSocket::~UdpSocket(){
  unbind();
}

net::Status net::UdpSocket::bind(const uint16_t& localPort){
  struct sockaddr_in sockSettings;
  sockSettings.sin_family = AF_INET;
  sockSettings.sin_port = (localPort == 0)?0:htons(localPort);
  sockSettings.sin_addr.s_addr = INADDR_ANY;
  if(::bind(socketId, reinterpret_cast<const sockaddr*>(&sockSettings), sizeof(sockSettings)) < 0){
    std::cerr << "[Socket Binding] : --FAILURE--\n";
    return net::Status::Error;
  }
  memset(&sockSettings, 0, sizeof(sockSettings));
  socklen_t len = sizeof(sockSettings);
  getsockname(socketId, reinterpret_cast<sockaddr*>(&sockSettings), &len);
  this->localIp = std::string(inet_ntoa(sockSettings.sin_addr));
  this->localPort = ntohs(sockSettings.sin_port);
  return net::Status::Done;
}

void net::UdpSocket::unbind(){
  close(socketId);
}

net::Status net::UdpSocket::send(const std::string& message, const std::string& remoteAddress, const uint16_t& remotePort, const std::size_t bytes){
  struct sockaddr_in destination;
  destination.sin_family = AF_INET;
  destination.sin_addr.s_addr = inet_addr(remoteAddress.c_str());
  destination.sin_port = htons(remotePort);
  if (destination.sin_addr.s_addr == INADDR_NONE) {
    std::cerr << "[Socket IpConfig] : --FAILURE--\n";
    close(socketId);
    return net::Status::Error;
  }
  std::size_t bytesSent = 0, bytesLeft = bytes;
  char buffer[bytes + 1];
  memset(buffer, 0, strlen(buffer) + 1);
  strcpy(buffer, message.c_str());
  while(bytesSent < bytes){
    size_t nbytesSent = 0;
    if ((nbytesSent = sendto(socketId, buffer + bytesSent, bytesLeft, 0, reinterpret_cast<const sockaddr*>(&destination), sizeof(destination))) < 0){
      std::cerr << "[Socket Sending] : --FAILURE--\n";
      return net::Status::Error;
    }
    bytesSent += nbytesSent;
    bytesLeft -= nbytesSent;
  }
  return net::Status::Done;
}

net::Status net::UdpSocket::receive(std::string& message, std::string& remoteAddress, uint16_t& remotePort, const std::size_t bytes){
  struct sockaddr_in destination;
  socklen_t len = sizeof(destination);
  std::size_t bytesRecv = 0, bytesLeft = bytes;
  char buffer[bytes + 1];
  memset(buffer, 0, strlen(buffer) + 1);
  while(bytesRecv < bytes){
    size_t nbytesRecv = 0;
    if((nbytesRecv = recvfrom(socketId, buffer + bytesRecv, bytesLeft, 0, reinterpret_cast<sockaddr*>(&destination), &len)) < 0){
      std::cerr << "[Socket Receiving] : --FAILURE--\n";
      return net::Status::Error;
    }
    bytesRecv += nbytesRecv;
    bytesLeft -= nbytesRecv;
  }
  remoteAddress = std::string(inet_ntoa(destination.sin_addr));
  remotePort = ntohs(destination.sin_port);
  message = std::string(buffer).substr(0, bytes);
  return net::Status::Done;
}

const uint16_t& net::UdpSocket::getLocalPort() const{
  return localPort;
}

const std::string& net::UdpSocket::getLocalIp() const{
  return localIp;
}

void net::UdpSocket::setBlocking(bool enabled){
  int32_t status = fcntl(socketId, F_GETFL);
  if(enabled != blocking){
    blocking = enabled;
    if (fcntl(socketId, F_SETFL, ((blocking)?(status & ~O_NONBLOCK):(status | O_NONBLOCK))) == -1)
      std::cerr << "[Socket Flag Change] : --FAILURE--\n";
  }
}

net::TimerAns net::waitResponse(net::UdpSocket& socket, const int32_t& milliseconds){
  fd_set rfds;
  struct timeval tv;
  int32_t retval;
  FD_ZERO(&rfds);
  FD_SET(socket.socketId, &rfds);
  tv.tv_sec = std::max((time_t)milliseconds, (time_t)0);
  tv.tv_usec = 0;
  retval = select(1, &rfds, NULL, NULL, &tv);
  if (retval == -1)
    return net::TimerAns::Error;
  else if (retval)
    return net::TimerAns::Success;
  return net::TimerAns::TimeOut;
}