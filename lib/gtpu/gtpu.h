/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */
#pragma once

#include "srsgnb/adt/byte_buffer.h"
#include "srsgnb/srslog/srslog.h"
#include <cstdint>

namespace srsgnb {

/****************************************************************************
 * GTPU Header
 * Ref: 3GPP TS 29.281 v10.1.0 Section 5
 *
 *        | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
 *
 * 1      |  Version  |PT | * | E | S |PN |
 * 2      |         Message Type          |
 * 3      |     Length (1st Octet)        |
 * 4      |     Length (2nd Octet)        |
 * 5      |      TEID (1st Octet)         |
 * 6      |      TEID (2nd Octet)         |
 * 7      |      TEID (3rd Octet)         |
 * 8      |      TEID (4th Octet)         |
 * 9      |    Seq Number (1st Octet)     |
 * 10     |    Seq Number (2st Octet)     |
 * 11     |            N-PDU              |
 * 12     |  Next Extension Header Type   |
 ***************************************************************************/

constexpr unsigned GTPU_BASE_HEADER_LEN     = 8;
constexpr unsigned GTPU_EXTENDED_HEADER_LEN = 12;

constexpr unsigned GTPU_FLAGS_VERSION_MASK       = 0xe0;
constexpr unsigned GTPU_FLAGS_VERSION_V1         = 0x01;
constexpr unsigned GTPU_FLAGS_GTP_PRIME_PROTOCOL = 0x00;
constexpr unsigned GTPU_FLAGS_GTP_PROTOCOL       = 0x01;

constexpr unsigned GTPU_MSG_ECHO_REQUEST                             = 1;
constexpr unsigned GTPU_MSG_ECHO_RESPONSE                            = 2;
constexpr unsigned GTPU_MSG_ERROR_INDICATION                         = 26;
constexpr unsigned GTPU_MSG_SUPPORTED_EXTENSION_HEADERS_NOTIFICATION = 31;
constexpr unsigned GTPU_MSG_END_MARKER                               = 254;
constexpr unsigned GTPU_MSG_DATA_PDU                                 = 255;

constexpr unsigned GTPU_EXT_NO_MORE_EXTENSION_HEADERS        = 0x00;
constexpr unsigned GTPU_EXT_HEADER_PDCP_PDU_NUMBER           = 0b11000000;
constexpr unsigned GTPU_EXT_HEADER_PDU_SESSION_CONTAINER     = 0x85;
constexpr unsigned GTPU_EXT_HEADER_PDU_SESSION_CONTAINER_LEN = 4;

struct gtpu_header {
  struct gtpu_flags {
    uint8_t version       = 1;
    uint8_t protocol_type = 0;
    bool    ext_hdr       = false;
    bool    seq_number    = false;
    bool    n_pdu         = false;
  } flags;
  uint8_t              message_type      = 0;
  uint16_t             length            = 0;
  uint32_t             teid              = 0;
  uint16_t             seq_number        = 0;
  uint8_t              n_pdu             = 0;
  uint8_t              next_ext_hdr_type = 0;
  std::vector<uint8_t> ext_buffer;
};

bool gtpu_read_and_strip_header(gtpu_header& header, byte_buffer& pdu, srslog::basic_logger& logger);
bool gtpu_write_header(byte_buffer& pdu, const gtpu_header& header, srslog::basic_logger& logger);

inline bool gtpu_supported_flags_check(const gtpu_header& header, srslog::basic_logger& logger)
{
  // flags
  if (header.flags.version != GTPU_FLAGS_VERSION_V1) {
    logger.error("gtpu_header - Unhandled GTP-U Version. Flags: {}", header.flags);
    return false;
  }
  if (header.flags.protocol_type != GTPU_FLAGS_GTP_PROTOCOL) {
    logger.error("gtpu_header - Unhandled Protocol Type. Flags: {}", header.flags);
    return false;
  }
  if (header.flags.n_pdu) {
    logger.error("gtpu_header - Unhandled Packet Number. Flags: {}", header.flags);
    return false;
  }
  return true;
}

inline bool gtpu_supported_msg_type_check(const gtpu_header& header, srslog::basic_logger& logger)
{
  // msg_tpye
  if (header.message_type != GTPU_MSG_DATA_PDU && header.message_type != GTPU_MSG_ECHO_REQUEST &&
      header.message_type != GTPU_MSG_ECHO_RESPONSE && header.message_type != GTPU_MSG_ERROR_INDICATION &&
      header.message_type != GTPU_MSG_END_MARKER) {
    logger.error("gtpu_header - Unhandled message type: 0x%x", header.message_type);
    return false;
  }
  return true;
}

} // namespace srsgnb

// Formatters
namespace fmt {

// GTP-U flags
template <>
struct formatter<srsgnb::gtpu_header::gtpu_flags> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsgnb::gtpu_header::gtpu_flags flags, FormatContext& ctx)
      -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(),
                     "GTP-U version={} bit, PT={}, E={}, S={}, PN={}",
                     flags.version,
                     flags.protocol_type,
                     flags.ext_hdr,
                     flags.seq_number,
                     flags.n_pdu);
  }
};

template <>
struct formatter<srsgnb::gtpu_header> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsgnb::gtpu_header hdr, FormatContext& ctx) -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(), "Flags[{}], Length={}, TEID={}", hdr.flags, hdr.length, hdr.teid);
  }
};
} // namespace fmt
