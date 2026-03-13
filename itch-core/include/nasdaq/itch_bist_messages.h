/*
 * NASDAQ US ITCH 5.0 protocol messages
 */

#ifndef HELIX_NASDAQ_US_ITCH_PROTO_H
#define HELIX_NASDAQ_US_ITCH_PROTO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef _WIN32
#pragma pack(push,1)
#define pack_attr
#else
#define pack_attr __attribute__((packed))
#endif // WIN32
#define ITCH_SYMBOL_LEN 8

struct itch_bist_message {
    char MessageType;
};

/*!
* This message is sent every second for which at least one ITCH message is being generated. 
* The message contains the number of seconds since the start of 1970-01-01 00:00:00 UTC, also called Unix Time
*/
struct itch_bist_seconds {
  char     MessageType;
  uint32_t UtcSeconds;
} pack_attr;

enum class financial_product_e : uint8_t
{
  Option  = 1,
  Forward = 2,
  Future = 3,
  FRA = 4,
  Cash = 5,
  Payment = 6,
  ExchangeRate = 7,
  InterestRateSwap = 8,
  REPO = 9,
  SyntheticBoxLeg = 10,
  StandardCombination = 11,
  Guarantee = 12,
  OTCGeneral = 13,
  EquityWarrant = 14,
  SecurityLending = 15,
  Certificate = 18,
};
static_assert(sizeof(financial_product_e) == sizeof(char), 
              "financial product enum size must be 1 byte!");

enum class put_or_call_e : uint8_t
{
  Call = 1,
  Put = 2
};

/*!
* At the start of each trading day, Order book directory messages are disseminated for all active securities, 
* including halted securities, in the Genium INET Trading system.
*/
struct itch_bist_order_book_directory {
  char      MessageType; // "R"
  uint32_t  TimestampNanoseconds;
  uint32_t  OrderBookID; // Expired Order book IDs may be reused for new instruments
  char      Symbol[32];
  char      LongName[32];
  char      ISINCode[12];
  financial_product_e FinancialProduct;
  char      TradingCurrency[3];
  uint16_t  NumberOfDecimalsInPrice; // A value of 256 means that the instrument is traded in fractions (each fraction is 1/256).
  uint16_t  NumberOfDecimalsInNominalValue;
  uint32_t  OddLotSize; // A value of 0 indicates that this lot type is undefined for the order book.
  uint32_t  RoundLotSize;
  uint32_t  BlockLotSize; // A value of 0 indicates that this lot type is undefined for the order book.
  uint64_t  NominalValue;
  uint8_t   NumberOfLegs;
  uint32_t  UnderlyingOrderBookID;
  uint32_t  StrikePrice;
  uint32_t  ExpirationDate;
  uint16_t  DecimalsInStrikePrice;
  put_or_call_e PutOrCall;
} pack_attr;
static_assert(sizeof(itch_bist_order_book_directory) == 129, 
              "itch_bist_order_book_directory size must be 129 byte as defined in standart!");

enum class leg_side_e : int8_t
{
  AsDefined = 'B',
  Opposite = 'C'
};

struct itch_bist_combination_order_book_leg {
  char        MessageType; // "M"
  uint32_t    TimestampNanoseconds;
  uint32_t    OrderBookID; // Expired Order book IDs may be reused for new instruments
  uint32_t    LegOrderBookID; 
  leg_side_e  legSide;
  uint32_t    LegRatio; 
} pack_attr;
static_assert(sizeof(itch_bist_combination_order_book_leg) == 18,
              "itch_bist_combination_order_book_leg size must be 18 byte as defined in standart!");

struct itch_bist_tick_size_table_entry {
  char        MessageType; // "L"
  uint32_t    TimestampNanoseconds;
  uint32_t    OrderBookID; // Expired Order book IDs may be reused for new instruments
  uint64_t    TickSize;
  uint32_t    PriceFrom;
  uint32_t    PriceTo;
} pack_attr;
static_assert(sizeof(itch_bist_tick_size_table_entry) == 25,
              "tick_size_table_entry size must be 25 byte as defined in standart!");

enum class system_event_code_e : int8_t
{
  StartMessage = 'O',
  EndMessage = 'C'
};

struct itch_bist_system_event {
  char        MessageType; // "S"
  uint32_t    TimestampNanoseconds;
  system_event_code_e EventCode;
} pack_attr;
static_assert(sizeof(itch_bist_system_event) == 6,
              "itch_bist_system_event size must be 6 byte as defined in standart!");

struct itch_bist_order_book_state {
  char      MessageType; // "O"
  uint32_t  TimestampNanoseconds;
  uint32_t  OrderBookID; // Expired Order book IDs may be reused for new instruments
  char      StateName[20];
} pack_attr;
static_assert(sizeof(itch_bist_order_book_state) == 29,
              "itch_bist_order_book_state size must be 29 byte as defined in standart!");

typedef char buy_sell_e;
//enum class buy_sell_e : int8_t
//{
//  Buy = 'B',
//  Sell = 'S'
//};

struct itch_bist_add_order {
  char       MessageType; // "A"
  uint32_t   TimestampNanoseconds;
  uint64_t   OrderID; // The number is only unique per Order book and side.
  uint32_t   OrderBookID; // Expired Order book IDs may be reused for new instruments
  buy_sell_e Side;
  uint32_t   OrderBookPosition;
  uint64_t   Quantity; // Orders with an undisclosed quantity will have this field set to 0.
  uint32_t   Price;
  uint16_t   OrderAttributes;
  uint8_t    LotType;
} pack_attr;
static_assert(sizeof(itch_bist_add_order) == 37,
              "itch_bist_add_order size must be 37 byte as defined in standart!");

struct itch_bist_add_order_mpid {
  char       MessageType; // "F"
  uint32_t   TimestampNanoseconds;
  uint64_t   OrderID;     // The number is only unique per Order book and side.
  uint32_t   OrderBookID; // Expired Order book IDs may be reused for new instruments
  buy_sell_e Side;
  uint32_t   OrderBookPosition;
  uint64_t   Quantity;    // Orders with an undisclosed quantity will have this field set to 0.
  uint32_t   Price;
  uint16_t   OrderAttributes;
  uint8_t    LotType;
  char       ParticipantID[7];
} pack_attr;
static_assert(sizeof(itch_bist_add_order_mpid) == 44,
              "itch_bist_add_order_mpid size must be 44 byte as defined in standart!");

struct itch_bist_order_executed {
  char       MessageType; // "E"
  uint32_t   TimestampNanoseconds;
  uint64_t   OrderID;     // The number is only unique per Order book and side.
  uint32_t   OrderBookID; // Expired Order book IDs may be reused for new instruments
  buy_sell_e Side;
  uint64_t   ExecutedQuantity;
  uint64_t   MatchID;
  uint32_t   ComboGroupID;
  char       reserved_1[7];
  char       reserved_2[7];
} pack_attr;
static_assert(sizeof(itch_bist_order_executed) == 52,
              "itch_bist_order_executed size must be 52 byte as defined in standart!");

struct itch_bist_order_executed_with_price {
  char       MessageType; // "C"
  uint32_t   TimestampNanoseconds;
  uint64_t   OrderID;     // The number is only unique per Order book and side.
  uint32_t   OrderBookID; // Expired Order book IDs may be reused for new instruments
  buy_sell_e Side;
  uint64_t   ExecutedQuantity;
  uint64_t   MatchID;
  uint32_t   ComboGroupID;
  char       reserved_1[7];
  char       reserved_2[7];
  uint32_t   TradePrice;
  char       OccurredAtCross;
  char       Printable;
} pack_attr;
static_assert(sizeof(itch_bist_order_executed_with_price) == 58,
              "itch_bist_order_executed_with_price size must be 58 byte as defined in standart!");

struct itch_bist_order_replace {
  char       MessageType; // "U"
  uint32_t   TimestampNanoseconds;
  uint64_t   OrderID;     // The number is only unique per Order book and side.
  uint32_t   OrderBookID; // Expired Order book IDs may be reused for new instruments
  buy_sell_e Side;
  uint32_t   NewOrderBookPosition;
  uint64_t   Quantity;
  uint32_t   Price;
  uint16_t   OrderAttributes;
} pack_attr;
static_assert(sizeof(itch_bist_order_replace) == 36,
              "itch_bist_order_replace size must be 36 byte as defined in standart!");

struct itch_bist_order_delete {
  char       MessageType; // "D"
  uint32_t   TimestampNanoseconds;
  uint64_t   OrderID;     // The number is only unique per Order book and side.
  uint32_t   OrderBookID; // Expired Order book IDs may be reused for new instruments
  buy_sell_e Side;
} pack_attr;
static_assert(sizeof(itch_bist_order_delete) == 18,
              "itch_bist_order_delete size must be 18 byte as defined in standart!");

struct itch_bist_trade {
  char       MessageType; // "P"
  uint32_t   TimestampNanoseconds;
  uint64_t   MatchID;     
  uint32_t   ComboGroupID;
  buy_sell_e Side;
  uint64_t   Quantity;
  uint32_t   OrderBookID; // Expired Order book IDs may be reused for new instruments
  uint32_t   TradePrice;
  char       reserved_1[7];
  char       reserved_2[7];
  char       Printable;
  char       OccurredAtCross;
} pack_attr;
static_assert(sizeof(itch_bist_trade) == 50,
              "itch_bist_trade size must be 50 byte as defined in standart!");

struct itch_bist_equilibrium_price_update {
  char       MessageType; // "Z"
  uint32_t   TimestampNanoseconds;
  uint32_t   OrderBookID; // Expired Order book IDs may be reused for new instruments
  uint64_t   AvailableBidQuantityAtPrice;
  uint64_t   AvailableAskQuantityAtPrice;
  uint32_t   EquilibriumPrice;
  uint32_t   BestBidPrice;
  uint32_t   BestAskPrice;
  uint64_t   BestBidQuantity;
  uint64_t   BestAskQuantity;
} pack_attr;
static_assert(sizeof(itch_bist_equilibrium_price_update) == 53,
              "itch_bist_equilibrium_price_update size must be 53 byte as defined in standart!");


#ifdef _WIN32
#pragma pack(pop)
#endif // WIN32


#ifdef __cplusplus
}
#endif

// template <class msg_t>
// constexpr static const bool should_process = false;

// template <>
// constexpr static const bool should_process<itch_bist_seconds> = true;

template <typename T>
struct itch_message_traits {
  constexpr const static char identifier = '?';
  constexpr const static uint32_t packet_len = 0;
};

template <>
struct itch_message_traits<itch_bist_message> {
  constexpr const static uint32_t packet_len = sizeof(itch_bist_message);
};

template <>
struct itch_message_traits<itch_bist_seconds> {
  constexpr const static char identifier = 'T';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_seconds);
};

template <>
struct itch_message_traits<itch_bist_order_book_directory> {
  constexpr const static char identifier = 'R';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_order_book_directory);
};

template <>
struct itch_message_traits<itch_bist_combination_order_book_leg> {
  constexpr const static char identifier = 'M';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_combination_order_book_leg);
};

template <>
struct itch_message_traits<itch_bist_tick_size_table_entry> {
  constexpr const static char identifier = 'L';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_tick_size_table_entry);
};

template <>
struct itch_message_traits<itch_bist_system_event> {
  constexpr const static char identifier = 'S';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_system_event);
};

template <>
struct itch_message_traits<itch_bist_order_book_state> {
  constexpr const static char identifier = 'O';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_order_book_state);
};

template <>
struct itch_message_traits<itch_bist_add_order> {
  constexpr const static char identifier = 'A';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_add_order);
};

template <>
struct itch_message_traits<itch_bist_add_order_mpid> {
  constexpr const static char identifier = 'F';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_add_order_mpid);
};

template <>
struct itch_message_traits<itch_bist_order_executed> {
  constexpr const static char identifier = 'E';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_order_executed);
};

template <>
struct itch_message_traits<itch_bist_order_executed_with_price> {
  constexpr const static char identifier = 'C';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_order_executed_with_price);
};

template <>
struct itch_message_traits<itch_bist_order_replace> {
  constexpr const static char identifier = 'U';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_order_replace);
};

template <>
struct itch_message_traits<itch_bist_order_delete> {
  constexpr const static char identifier = 'D';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_order_delete);
};

template <>
struct itch_message_traits<itch_bist_trade> {
  constexpr const static char identifier = 'P';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_trade);
};

template <>
struct itch_message_traits<itch_bist_equilibrium_price_update> {
  constexpr const static char identifier = 'Z';
  constexpr const static uint32_t packet_len = sizeof(itch_bist_equilibrium_price_update);
};

template <typename T>
constexpr uint32_t itch_packet_size() {
  return itch_message_traits<T>::packet_len;
}

constexpr uint32_t itch_packet_size(const char message_type) {
  switch (message_type) {
    case 'T': return itch_message_traits<itch_bist_seconds>::packet_len;
    case 'R': return itch_message_traits<itch_bist_order_book_directory>::packet_len;
    case 'M': return itch_message_traits<itch_bist_combination_order_book_leg>::packet_len;
    case 'L': return itch_message_traits<itch_bist_tick_size_table_entry>::packet_len;
    case 'S': return itch_message_traits<itch_bist_system_event>::packet_len;
    case 'O': return itch_message_traits<itch_bist_order_book_state>::packet_len;
    case 'A': return itch_message_traits<itch_bist_add_order>::packet_len;
    case 'F': return itch_message_traits<itch_bist_add_order_mpid>::packet_len;
    case 'E': return itch_message_traits<itch_bist_order_executed>::packet_len;
    case 'C': return itch_message_traits<itch_bist_order_executed_with_price>::packet_len;
    case 'U': return itch_message_traits<itch_bist_order_replace>::packet_len;
    case 'D': return itch_message_traits<itch_bist_order_delete>::packet_len;
    case 'P': return itch_message_traits<itch_bist_trade>::packet_len;
    case 'Z': return itch_message_traits<itch_bist_equilibrium_price_update>::packet_len;
    // default: throw std::runtime_error("unknown message type: " + std::string(1, message_type));
    default: return 0;
  }
}

template <typename T>
constexpr char itch_packet_identifier() {
  return itch_message_traits<T>::identifier;
}


#endif // ITCH_BIST_MESSAGES_HH
