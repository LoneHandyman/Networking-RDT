#include "UDPWrapper/UdpSocket.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <algorithm>

net::UdpSocket::UdpSocket(){
  if((socketId = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    std::cerr << "[Socket Creation] : --FAILURE--\n";
  }
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

net::Status net::UdpSocket::send(const std::string& message, const std::string& remoteAddress, const uint16_t& remotePort){
  struct sockaddr_in destination;
  destination.sin_family = AF_INET;
  destination.sin_addr.s_addr = inet_addr(remoteAddress.c_str());
  destination.sin_port = htons(remotePort);
  if (destination.sin_addr.s_addr == INADDR_NONE) {
    std::cerr << "[Socket IpConfig] : --FAILURE--\n";
    close(socketId);
    return net::Status::Error;
  }
  std::size_t bytesSent = 0, bytesLeft = MAX_DGRAM_SIZE;
  char buffer[MAX_DGRAM_SIZE + 1];
  memset(buffer, 0, strlen(buffer) + 1);
  strcpy(buffer, message.c_str());
  while(bytesSent < MAX_DGRAM_SIZE){
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

net::Status net::UdpSocket::receive(std::string& message, std::string& remoteAddress, uint16_t& remotePort){
  struct sockaddr_in destination;
  socklen_t len = sizeof(destination);
  std::size_t bytesRecv = 0, bytesLeft = MAX_DGRAM_SIZE;
  char buffer[MAX_DGRAM_SIZE + 1];
  memset(buffer, 0, strlen(buffer) + 1);
  while(bytesRecv < MAX_DGRAM_SIZE){
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
  message = std::string(buffer).substr(0, MAX_DGRAM_SIZE);
  return net::Status::Done;
}

const uint16_t& net::UdpSocket::getLocalPort() const{
  return localPort;
}

const std::string& net::UdpSocket::getLocalIp() const{
  return localIp;
}