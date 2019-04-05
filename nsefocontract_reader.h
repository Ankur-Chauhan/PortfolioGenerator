#ifndef _NSEFO_CONTRACT_READER_H_
#define _NSEFO_CONTRACT_READER_H_

#include <iostream>
#include "filereader.h"
#include "struct_char.h"

class CNSEFOContractFileReader : public CFileReader
{
	enum ENUM_NSEFO_COLUMN_ID : uint16_t
	{
		CONTRACT_TOKEN              = 0,				
		CONTRACT_ASSETTOKEN         = 1,
		CONTRACT_INSTRUMENT_NAME    = 2,		
		CONTRACT_SYMBOL             = 3,			
		CONTRACT_SERIES             = 4,					
		CONTRACT_EXPIRY_DATE        = 6,					
		CONTRACT_STRIKE_PRICE       = 7,					
		CONTRACT_OPTION_TYPE        = 8,
		CONTRACT_CA_LEVEL           = 10,		
		CONTRACT_PERMITTED_TO_TRADE	= 12,	
		CONTRACT_SECURITY_STATUS		= 15,	//NORMAL MKT		
		CONTRACT_ELIGIBILITY				= 15,	
		CONTRACT_MIN_LOT_QTY				= 30,			
		CONTRACT_BROAD_LOT_QTY			= 31,					
		CONTRACT_TICK_SIZE          = 32,						
    CONTRACT_ISSUE_CAPITAL      = 33,    
		CONTRACT_FREEZE_QTY         = 34,								
		CONTRACT_LOW_PRICE_RANGE		= 42,								
		CONTRACT_HIGH_PRICE_RANGE		= 43,						
		CONTRACT_NAME               = 53,								
		CONTRACT_BASE_PRICE         = 67,		
		CONTRACT_DELETE_FLAG				= 68,		
	};
	
	public:
		CNSEFOContractFileReader(const char* pcFullFileName, char chSeparator);
		~CNSEFOContractFileReader();
    
    bool GetNextRow(OMSScripInfo& stOMSScripInfo);
};


class CIgnoreSymbolReader : public CFileReader
{
	enum ENUM_NSEFO_COLUMN_ID : uint16_t
	{
		IGNORE_SYMBOL_NAME          = 0,				  
  };
  
  public:
    CIgnoreSymbolReader(const char* pcFullFileName, char chSeparator);
    ~CIgnoreSymbolReader();
    
    bool GetNextRow(char* pcSymbol);
};


class CSymbolFilterReader : public CFileReader
{
	enum ENUM_NSEFO_COLUMN_ID : uint16_t
	{
		SYMBOL_FILTER_SYMBOL_NAME          = 0,				
    SYMBOL_FILTER_CURR_MONTH_LIST      = 1,
    SYMBOL_FILTER_NEXT_MONTH_LIST      = 2,
    SYMBOL_FILTER_FAR_MONTH_LIST       = 3,
    SYMBOL_FILTER_X_MONTH_LIST         = 4,
	};
	
	public:
		CSymbolFilterReader(const char* pcFullFileName, char chSeparator);
		~CSymbolFilterReader();
    
    bool GetNextRow(tagIndividualSymbolDetails& stIndividualSymbolDetails, char* pcStrikePriceList1, char* pcStrikePriceList2, char* pcStrikePriceList3);
};


class CNiftySymbolFilterReader : public CFileReader
{
	enum ENUM_NIFTY_SP_FILTER_ID : uint16_t
	{
		SYMBOL_FILTER_SYMBOL_NAME          = 0,				
    NIFTY_SP_FILTER_1                  = 1,
    NIFTY_SP_FILTER_2                  = 2,
    NIFTY_SP_FILTER_3                  = 3,    
    NIFTY_SP_FILTER_4                  = 4,
    NIFTY_SP_FILTER_5                  = 5,
    NIFTY_SP_FILTER_6                  = 6,
    NIFTY_SP_FILTER_7                  = 7,
    NIFTY_SP_FILTER_8                  = 8,
    NIFTY_SP_FILTER_9                  = 9,
    NIFTY_SP_FILTER_10                 = 10,    
    NIFTY_SP_FILTER_11                 = 11,
    NIFTY_SP_FILTER_12                 = 12,    
	};
	
	public:
		CNiftySymbolFilterReader(const char* pcFullFileName, char chSeparator);
		~CNiftySymbolFilterReader();
    
    bool GetNextRow(tagSymbolRangeDetails& stSymbolRangeDetails);
};

class CStrikePriceReader : public CFileReader
{
	enum ENUM_NSEFO_COLUMN_ID : uint16_t
	{
		SYMBOL_FILTER_SYMBOL_NAME          = 0,				
    SYMBOL_FILTER_CURR_MONTH_LIST      = 1,
    SYMBOL_FILTER_NEXT_MONTH_LIST      = 2,
    SYMBOL_FILTER_FAR_MONTH_LIST       = 3,
	};
	
	public:
		CStrikePriceReader(const char* pcFullFileName, char chSeparator);
		~CStrikePriceReader();
    
    bool GetNextRow(std::vector<std::string>& szSplitStrings);
};


#endif
