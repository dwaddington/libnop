#ifndef LIBNOP_INCLUDE_NOP_BASE_PAIR_H_
#define LIBNOP_INCLUDE_NOP_BASE_PAIR_H_

#include <type_traits>
#include <utility>

#include <nop/base/encoding.h>

namespace nop {

//
// std::pair<T, U> encoding format:
//
// +-----+---------+-------+--------+
// | ARY | INT64:2 | FIRST | SECOND |
// +-----+---------+-------+--------+
//
// First must be a valid encoding of T; second must be a valid encoding of U.
//

template <typename T, typename U>
struct Encoding<std::pair<T, U>> : EncodingIO<std::pair<T, U>> {
  using Type = std::pair<T, U>;

  static constexpr EncodingByte Prefix(const Type& value) {
    return EncodingByte::Array;
  }

  static constexpr std::size_t Size(const Type& value) {
    return BaseEncodingSize(Prefix(value)) + Encoding<std::uint64_t>::Size(2u) +
           Encoding<First>::Size(value.first) +
           Encoding<Second>::Size(value.second);
  }

  static constexpr bool Match(EncodingByte prefix) {
    return prefix == EncodingByte::Array;
  }

  template <typename Writer>
  static Status<void> WritePayload(EncodingByte prefix, const Type& value,
                                   Writer* writer) {
    auto status = Encoding<std::uint64_t>::Write(2u, writer);
    if (!status)
      return status;

    status = Encoding<First>::Write(value.first, writer);
    if (!status)
      return status;

    return Encoding<Second>::Write(value.second, writer);
  }

  template <typename Reader>
  static Status<void> ReadPayload(EncodingByte /*prefix*/, Type* value,
                                  Reader* reader) {
    std::uint64_t size;
    auto status = Encoding<std::uint64_t>::Read(&size, reader);
    if (!status)
      return status;
    else if (size != 2u)
      return ErrorStatus::InvalidContainerLength;

    status = Encoding<First>::Read(&value->first, reader);
    if (!status)
      return status;

    return Encoding<Second>::Read(&value->second, reader);
  }

 private:
  using First = std::remove_cv_t<std::remove_reference_t<T>>;
  using Second = std::remove_cv_t<std::remove_reference_t<U>>;
};

}  // namespace nop

#endif  // LIBNOP_INCLUDE_NOP_BASE_PAIR_H_
