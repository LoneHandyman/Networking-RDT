#ifndef CYCLIC_REDUNDANCY_CHECK_HPP_
#define CYCLIC_REDUNDANCY_CHECK_HPP_

#include <iostream>
#include <string>

namespace crypto{

  uint32_t crc32(const std::string& buffer);

}

#endif//CYCLIC_REDUNDANCY_CHECK_HPP_