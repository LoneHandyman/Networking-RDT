#ifndef RDT_SOCKET_HPP_
#define RDT_SOCKET_HPP_

#include "RDTHeader.hpp"

namespace net{

  class RDTSocket{
  public:
    enum Status{SYN_SENT, SYN_RECV, ESTABLISHED,
                WAIT_ACK, FIN_WAIT1, FIN_WAIT2,
                CLOSE_WAIT, LAST_ACK, CLOSED,
                LISTEN};
  };

}

#endif//RDT_SOCKET_HPP_