/*
 * BinaryFILE support
 *
 * This implementation is based on the following specification provided
 * by NASDAQ:
 *
 *   BinaryFILE
 *   Version 1.00
 *   3/30/2010
 */

#pragma once

#include "compat/endian.h"
#include "helix.hh"

#include <memory>

namespace helix {

namespace nasdaq {

template<typename Handler>
class binaryfile_session final : public session {
    Handler _handler;
public:
    explicit binaryfile_session(void* data);

    bool is_rth_timestamp(uint64_t timestamp) override;

    std::string subscribe(const std::string& symbol, size_t max_orders) override;

    void register_callback(event_callback callback) override;

    void set_send_callback(send_callback send_cb) override;

    size_t process_packet(const net::packet_view& packet) override;

    void stop() override;
};

template<typename Handler>
binaryfile_session<Handler>::binaryfile_session(void* data)
    : session{data}
{
}

template<typename Handler>
bool binaryfile_session<Handler>::is_rth_timestamp(uint64_t timestamp)
{
    return _handler.is_rth_timestamp(timestamp);
}

template<typename Handler>
std::string binaryfile_session<Handler>::subscribe(const std::string& symbol, size_t max_orders)
{
    return _handler.subscribe(symbol, max_orders);
}

template<typename Handler>
void binaryfile_session<Handler>::register_callback(event_callback callback)
{
    _handler.register_callback(callback);
}

template<typename Handler>
void binaryfile_session<Handler>::stop()
{
  _handler.stop();
}

template<typename Handler>
void binaryfile_session<Handler>::set_send_callback(send_callback send_cb)
{
}

template<typename Handler>
size_t binaryfile_session<Handler>::process_packet(const net::packet_view& packet)
{
  uint16_t payload_len = swap_bytes(*packet.cast<uint16_t>());
  if (!payload_len) {
    // End of session.
    return 0;
  }
  size_t offset = sizeof(uint16_t);
  while (payload_len) {
    size_t nr = _handler.process_packet(net::packet_view{ packet.buf() + offset, payload_len });
    if (nr > payload_len) {
      throw std::runtime_error("payload overflow");
    }
    payload_len -= static_cast<uint16_t>(nr);
    offset += nr;
  }
  return offset;
}

}

}
