/*
 * NASDAQ US ITCH 5.0 protocol messages
 */

#ifndef HELIX_NASDAQ_US_ITCH_PROTO_H
#define HELIX_NASDAQ_US_ITCH_PROTO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define ITCH_SYMBOL_LEN 8

#ifdef _WIN32
#pragma pack(push,1)
#define pack_attr
#else
#define pack_attr __attribute__((packed))
#endif // WIN32

struct itch_bist_message {
    char MessageType;
};

struct itch_bist_system_event {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    char     EventCode;
} pack_attr;

struct itch_bist_stock_directory {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    char     Stock[8];
    char     MarketCategory;
    char     FinancialStatusIndicator;
    uint32_t RoundLotSize;
    char     RoundLotsOnly;
    char     IssueClassification;
    char     IssueSubType[2];
    char     Authenticity;
    char     ShortSaleThresholdIndicator;
    char     IPOFlag;
    char     LULDReferencePriceTier;
    char     ETPFlag;
    uint32_t ETPLeverageFactor;
    char     InverseIndicator;
} pack_attr;

struct itch_bist_stock_trading_action {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    char     Stock[8];
    char     TradingState;
    char     Reserved;
    char     Reason[4];
} pack_attr;

static_assert(sizeof(itch_bist_stock_trading_action) == 27, "structures must be packed");

struct itch_bist_reg_sho_restriction {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    char     Stock[8];
    char     RegSHOAction;
} pack_attr;

struct itch_bist_market_participant_position {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    char     MPID[4];
    char     Stock[8];
    char     PrimaryMarketMaker;
    char     MarketMakerMode;
    char     MarketParticipantState;
} pack_attr;

struct itch_bist_mwcb_decline_level {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    uint64_t Level1;
    uint64_t Level2;
    uint64_t Level3;
} pack_attr;

struct itch_bist_mwcb_breach {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    char     BeachedLevel;
} pack_attr;

struct itch_bist_ipo_quoting_period_update {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    char     Stock[8];
    uint32_t IPOQuotationReleaseTime;
    char     IPOQuotationReleaseQualifier;
    uint32_t IPOPrice;
} pack_attr;

struct itch_bist_add_order {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    uint64_t OrderReferenceNumber;
    char     BuySellIndicator;
    uint32_t Shares;
    char     Stock[8];
    uint32_t Price;
} pack_attr;

struct itch_bist_add_order_mpid {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    uint64_t OrderReferenceNumber;
    char     BuySellIndicator;
    uint32_t Shares;
    char     Stock[8];
    uint32_t Price;
    char     Attribution[4];
} pack_attr;

struct itch_bist_order_executed {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    uint64_t OrderReferenceNumber;
    uint32_t ExecutedShares;
    uint64_t MatchNumber;
} pack_attr;

struct itch_bist_order_executed_with_price {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    uint64_t OrderReferenceNumber;
    uint32_t ExecutedShares;
    uint64_t MatchNumber;
    char     Printable;
    uint32_t ExecutionPrice;
} pack_attr;

struct itch_bist_order_cancel {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    uint64_t OrderReferenceNumber;
    uint32_t CanceledShares;
} pack_attr;

struct itch_bist_order_delete {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    uint64_t OrderReferenceNumber;
} pack_attr;

struct itch_bist_order_replace {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    uint64_t OriginalOrderReferenceNumber;
    uint64_t NewOrderReferenceNumber;
    uint32_t Shares;
    uint32_t Price;
} pack_attr;

struct itch_bist_trade {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    uint64_t OrderReferenceNumber;
    char     BuySellIndicator;
    uint32_t Shares;
    char     Stock[8];
    uint32_t Price;
    uint64_t MatchNumber;
} pack_attr;

struct itch_bist_cross_trade {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    uint64_t Shares;
    char     Stock[8];
    uint32_t CrossPrice;
    uint64_t MatchNumber;
    char     CrossType;
} pack_attr;

struct itch_bist_broken_trade {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    uint64_t MatchNumber;
} pack_attr;

struct itch_bist_noii {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    uint64_t PairedShares;
    uint64_t ImbalanceShares;
    char     ImbalanceDirection;
    char     Stock[8];
    uint32_t FarPrice;
    uint32_t NearPrice;
    uint32_t CurrentReferencePrice;
    char     CrossType;
    char     PriceVariationIndicator;
} pack_attr;

struct itch_bist_rpii {
    char     MessageType;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp:48;
    char     Stock[8];
    char     InterestFlag;
} pack_attr;

#ifdef _WIN32
#pragma pack(pop)
#endif // WIN32


#ifdef __cplusplus
}
#endif

struct itch_bist_stock_trading_action_aligned {
  char     MessageType;
  uint16_t StockLocate;
  uint16_t TrackingNumber;
  uint64_t Timestamp : 48;
  char     Stock[8];
  char     TradingState;
  char     Reserved;
  char     Reason[4];
};
static_assert(sizeof(itch_bist_stock_trading_action_aligned) != sizeof(itch_bist_stock_trading_action), "structures must be packed");

#endif