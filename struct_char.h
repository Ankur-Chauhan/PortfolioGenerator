#ifndef _STRUCT_CHAR_H_
#define _STRUCT_CHAR_H_


const int16_t SCRIP_SYMBOL_LEN            = 10;
const int16_t SCRIP_SERIES_LEN            = 2;
const int16_t SCRIP_OPTION_TYPE_LEN       = 2;
const int16_t TRADING_SYMBOL_NAME_LEN     = 25;
const int16_t SCRIP_INSTRUMENT_NAME_LEN   = 6;
const int16_t STRIKE_PRICE_LIST           = 2048;
const int16_t MAX_PORTFOLIO_PER_TOKEN     = 128;
const int16_t MAX_PORTFOLIO_NAME_LEN      = 64;
const int16_t WEEKLY_IDENTIFIER_LEN       = 2;

enum CUSTOM_INSTRUMENT : int16_t
{
  INSTRUMENT_FUTURE         = 1,
  INSTRUMENT_CALL           = 2,
  INSTRUMENT_PUT            = 3,
  INSTRUMENT_SPOT           = 4,
};

#pragma pack(1)

struct tagSymbolDetails
{
	int                     nStrikePrice;	
	int                     nExpiryDate;	
	short										nCALevel;	
	char										cInstrumentName[SCRIP_INSTRUMENT_NAME_LEN  + 1];	
	char										cSymbol[SCRIP_SYMBOL_LEN + 1];	
	char										cSeries[SCRIP_SERIES_LEN + 1];
	char										cOptionType[SCRIP_OPTION_TYPE_LEN + 1];
};

struct OMSScripInfo
{
  tagSymbolDetails        stSymbolDetails;

  int32_t                 nToken; 
	int                     nAssetToken;  
  int32_t                 nBoardLotQty;
  int32_t                 nMinLotQty;     
  int32_t                 nTickSize;
  int16_t                 nCustomInstrumentType;
  char                    cDeleteFlag;
  char                    cTradingSymbolName[TRADING_SYMBOL_NAME_LEN + 1]; //empty in case of Spread contract as not received from exchange
  int64_t                 nTotalTraded;
};

struct tagWeeklyStrikePriceStruct
{
  int32_t                nWeeklyExpiry1[MAX_PORTFOLIO_PER_TOKEN];
  int32_t                nWeeklyExpiry2[MAX_PORTFOLIO_PER_TOKEN];
  int32_t                nWeeklyExpiry3[MAX_PORTFOLIO_PER_TOKEN];
  int32_t                nWeeklyExpiry4[MAX_PORTFOLIO_PER_TOKEN];
};

struct tagIndividualSymbolDetails
{
	char										cSymbol[SCRIP_SYMBOL_LEN + 1];	
  char                    cExpiryDate;
  int32_t                 nStrikePrice[MAX_PORTFOLIO_PER_TOKEN];
  std::set<std::string>   bWeeklyExpirySet;
  
  void clear()
  {
    memset(nStrikePrice, 0, sizeof(int32_t)*MAX_PORTFOLIO_PER_TOKEN);
    cExpiryDate = '\0';
    bWeeklyExpirySet.clear();
  }
};

struct tagSymbolRangeDetails
{
	int32_t                 nLowerStartStrikePrice;	
  int32_t                 nLowerEndStrikePrice;	
	int32_t                 nUpperStartStrikePrice;	
	int32_t                 nUppeEndStrikePrice;	
	int32_t                 nOffset;	  
	char                    cExpiryDate;	
	char										cSymbol[SCRIP_SYMBOL_LEN + 1];	
};

#pragma pack()

#endif

