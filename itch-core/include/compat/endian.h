#ifndef HELIX_ENDIAN_H
#define HELIX_ENDIAN_H

#include <cstdint>
#include <cstring>

#if defined(_MSC_VER)
#include <cstdlib>
#endif

enum class endian_e : uint8_t {
  big = 1,
  little = 0,
};

#if defined(_WIN32)
#define HELIX_ENDIAN_IS_LITTLE 1
#elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define HELIX_ENDIAN_IS_LITTLE 1
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define HELIX_ENDIAN_IS_LITTLE 0
#endif

[[nodiscard]] inline endian_e get_endian_of_os() noexcept {
#if defined(HELIX_ENDIAN_IS_LITTLE)
  return HELIX_ENDIAN_IS_LITTLE ? endian_e::little : endian_e::big;
#else
  const uint16_t value = 1;
  return *reinterpret_cast<const uint8_t*>(&value) == 1 ? endian_e::little : endian_e::big;
#endif
}

inline const endian_e endian_of_os = get_endian_of_os();

[[nodiscard]] inline uint16_t swap_bytes(uint16_t in) noexcept {
#if defined(_MSC_VER)
  return _byteswap_ushort(in);
#else
  return __builtin_bswap16(in);
#endif
}

[[nodiscard]] inline int16_t swap_bytes(int16_t in) noexcept {
  uint16_t bits = 0;
  std::memcpy(&bits, &in, sizeof(bits));
  bits = swap_bytes(bits);
  std::memcpy(&in, &bits, sizeof(in));
  return in;
}

[[nodiscard]] inline uint32_t swap_bytes(uint32_t in) noexcept {
#if defined(_MSC_VER)
  return _byteswap_ulong(in);
#else
  return __builtin_bswap32(in);
#endif
}

[[nodiscard]] inline int32_t swap_bytes(int32_t in) noexcept {
  uint32_t bits = 0;
  std::memcpy(&bits, &in, sizeof(bits));
  bits = swap_bytes(bits);
  std::memcpy(&in, &bits, sizeof(in));
  return in;
}

[[nodiscard]] inline uint64_t swap_bytes(uint64_t in) noexcept {
#if defined(_MSC_VER)
  return _byteswap_uint64(in);
#else
  return __builtin_bswap64(in);
#endif
}

[[nodiscard]] inline int64_t swap_bytes(int64_t in) noexcept {
  uint64_t bits = 0;
  std::memcpy(&bits, &in, sizeof(bits));
  bits = swap_bytes(bits);
  std::memcpy(&in, &bits, sizeof(in));
  return in;
}

[[nodiscard]] inline uint16_t helix_be16toh(uint16_t in) noexcept {
  if (endian_of_os == endian_e::little) {
    return swap_bytes(in);
  }
  return in;
}

[[nodiscard]] inline uint32_t helix_be32toh(uint32_t in) noexcept {
  if (endian_of_os == endian_e::little) {
    return swap_bytes(in);
  }
  return in;
}

[[nodiscard]] inline uint64_t helix_be64toh(uint64_t in) noexcept {
  if (endian_of_os == endian_e::little) {
    return swap_bytes(in);
  }
  return in;
}

[[nodiscard]] inline uint16_t helix_htobe16(uint16_t in) noexcept {
  return helix_be16toh(in);
}

[[nodiscard]] inline uint32_t helix_htobe32(uint32_t in) noexcept {
  return helix_be32toh(in);
}

[[nodiscard]] inline uint64_t helix_htobe64(uint64_t in) noexcept {
  return helix_be64toh(in);
}

#ifndef be16toh
#define be16toh(x) helix_be16toh(static_cast<uint16_t>(x))
#endif

#ifndef be32toh
#define be32toh(x) helix_be32toh(static_cast<uint32_t>(x))
#endif

#ifndef be64toh
#define be64toh(x) helix_be64toh(static_cast<uint64_t>(x))
#endif

#ifndef htobe16
#define htobe16(x) helix_htobe16(static_cast<uint16_t>(x))
#endif

#ifndef htobe32
#define htobe32(x) helix_htobe32(static_cast<uint32_t>(x))
#endif

#ifndef htobe64
#define htobe64(x) helix_htobe64(static_cast<uint64_t>(x))
#endif

#undef HELIX_ENDIAN_IS_LITTLE

#endif