#ifndef HELIX_ENDIAN_H
#define HELIX_ENDIAN_H

// TODO(): make endian conversions with stl helpers instead of preprocessor directives

#ifndef _WIN32
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __APPLE__

#include <libkern/OSByteOrder.h>

#define be16toh(x) OSSwapBigToHostInt16(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define htobe16(x) OSSwapHostToBigInt16(x)
#define htobe32(x) OSSwapHostToBigInt32(x)
#define htobe64(x) OSSwapHostToBigInt64(x)

#else

#include <endian.h>

#endif

#ifdef __cplusplus
}
#endif

#else
#include <stdlib.h>

#define be16toh(x) _byteswap_ushort(x)
#define be32toh(x) _byteswap_ulong(x)
#define be64toh(x) _byteswap_uint64(x)
#define htobe16(x) (x)
#define htobe32(x) (x)
#define htobe64(x) (x)

//template <class in_t>
//[[nodiscard]] inline in_t swap_bytes(in_t&& in) {
//  return std::forward<in_t>(in);
//}

enum class endian_e : uint8_t {
  big = 1,
  little = 0,
};

static endian_e get_endian_of_os() noexcept {
  uint16_t val{ 1 };
  return static_cast<endian_e>(reinterpret_cast<char*>(&val)[0]);
}

[[nodiscard]] inline uint16_t swap_bytes(uint16_t in) {
  return _byteswap_ushort(in);
}

[[nodiscard]] inline int16_t swap_bytes(int16_t in) {
  return static_cast<int16_t>(swap_bytes(*reinterpret_cast<uint16_t*>(&in)));
}

[[nodiscard]] inline uint32_t swap_bytes(uint32_t in) {
  return _byteswap_ulong(in);
}

[[nodiscard]] inline int32_t swap_bytes(int32_t in) {
  return static_cast<int32_t>(swap_bytes(*reinterpret_cast<uint32_t*>(&in)));
}

[[nodiscard]] inline uint64_t swap_bytes(uint64_t in) {
  return _byteswap_uint64(in);
}

[[nodiscard]] inline int64_t swap_bytes(int64_t in) {
  return static_cast<int64_t>(swap_bytes(*reinterpret_cast<uint64_t*>(&in)));
}

#endif

#endif