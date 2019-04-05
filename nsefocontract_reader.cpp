#include "nsefocontract_reader.h"

CNSEFOContractFileReader::CNSEFOContractFileReader(const char* pcFullFileName, char chSeparator) :
							CFileReader(pcFullFileName)							
{
  std::vector<uint16_t> lszInputCoumunNo = {CONTRACT_TOKEN,       CONTRACT_ASSETTOKEN,      CONTRACT_INSTRUMENT_NAME,   CONTRACT_SYMBOL, 
                                            CONTRACT_SERIES,      CONTRACT_EXPIRY_DATE,     CONTRACT_STRIKE_PRICE,      CONTRACT_OPTION_TYPE,
                                            CONTRACT_CA_LEVEL,    CONTRACT_MIN_LOT_QTY,     CONTRACT_BROAD_LOT_QTY,     CONTRACT_TICK_SIZE,
                                            CONTRACT_NAME,        CONTRACT_DELETE_FLAG
                                            };
  m_cStringSplit.UpdateSplitparam(chSeparator, lszInputCoumunNo);
}

CNSEFOContractFileReader::~CNSEFOContractFileReader()
{

}


bool CNSEFOContractFileReader::GetNextRow(OMSScripInfo& stOMSScripInfo)
{
  int32_t lnCount = 0;
  int32_t lnStringCount = 0;  
  
  std::vector<std::string> lszSplitStrings;  
	if(GetNextLine())
	{
    if(m_cStringSplit.GetData(m_cRowRecords, lszSplitStrings))
    {
      lnCount = -1;
      if(!lszSplitStrings.empty())
      {
        lnCount = 0;
        lnStringCount = lszSplitStrings.size();
      }        
      
      auto lcItr = m_cStringSplit.GetColumnList().begin();
      for(; lcItr != m_cStringSplit.GetColumnList().end() && lnCount < lnStringCount; ++lcItr)
      {
        switch(*lcItr)
        {
          case CONTRACT_TOKEN :
          {
            stOMSScripInfo.nToken = atol(lszSplitStrings[lnCount++].c_str());				
          }
          break;

          case CONTRACT_ASSETTOKEN:
          {
            stOMSScripInfo.nAssetToken = atol(lszSplitStrings[lnCount++].c_str());									
          }
          break;

          case CONTRACT_INSTRUMENT_NAME:
          {
            strncpy(stOMSScripInfo.stSymbolDetails.cInstrumentName, lszSplitStrings[lnCount++].c_str(), SCRIP_INSTRUMENT_NAME_LEN);
          }
          break;

          case CONTRACT_SYMBOL:
          {
            strncpy(stOMSScripInfo.stSymbolDetails.cSymbol, lszSplitStrings[lnCount++].c_str(), SCRIP_SYMBOL_LEN);
          }
          break;

          case CONTRACT_SERIES:
          {
            strncpy(stOMSScripInfo.stSymbolDetails.cSeries, lszSplitStrings[lnCount++].c_str(), SCRIP_SERIES_LEN);
          }
          break;

          case CONTRACT_EXPIRY_DATE:
          {
            stOMSScripInfo.stSymbolDetails.nExpiryDate = atol(lszSplitStrings[lnCount++].c_str());
          }
          break;

          case CONTRACT_STRIKE_PRICE:
          {
            stOMSScripInfo.stSymbolDetails.nStrikePrice = atoi(lszSplitStrings[lnCount++].c_str());
            if(-1 == stOMSScripInfo.stSymbolDetails.nStrikePrice)
            {
              stOMSScripInfo.nCustomInstrumentType = INSTRUMENT_FUTURE;  
            }
          }
          break;
				
          case CONTRACT_OPTION_TYPE:
          {
            strncpy(stOMSScripInfo.stSymbolDetails.cOptionType, lszSplitStrings[lnCount++].c_str(), SCRIP_OPTION_TYPE_LEN);					
            
            if(0 == strncmp(stOMSScripInfo.stSymbolDetails.cOptionType, "CE", 2))
            {
              stOMSScripInfo.nCustomInstrumentType = INSTRUMENT_CALL;  
            }
            
            if(0 == strncmp(stOMSScripInfo.stSymbolDetails.cOptionType, "PE", 2))
            {
              stOMSScripInfo.nCustomInstrumentType = INSTRUMENT_PUT;  
            }
            
          }
          break;
				
          case CONTRACT_CA_LEVEL:
          {
            stOMSScripInfo.stSymbolDetails.nCALevel = atoi(lszSplitStrings[lnCount++].c_str());
          }
          break;
				
          case CONTRACT_MIN_LOT_QTY:
          {
            stOMSScripInfo.nMinLotQty = atol(lszSplitStrings[lnCount++].c_str());
          }
          break;

          case CONTRACT_BROAD_LOT_QTY:
          {
            stOMSScripInfo.nBoardLotQty = atol(lszSplitStrings[lnCount++].c_str());
          }
          break;

          case CONTRACT_TICK_SIZE:
          {
            stOMSScripInfo.nTickSize = atol(lszSplitStrings[lnCount++].c_str());
          }
          break;

          case CONTRACT_NAME:
          {
            strncpy(stOMSScripInfo.cTradingSymbolName, lszSplitStrings[lnCount++].c_str(), TRADING_SYMBOL_NAME_LEN);				
          }
          break;

          case CONTRACT_DELETE_FLAG:
          {
            stOMSScripInfo.cDeleteFlag = *lszSplitStrings[lnCount++].c_str();
          }
          break;
        }
      }
    }
	}
	else
	{
		return false;
	}
  
  return true;
}

CIgnoreSymbolReader::CIgnoreSymbolReader(const char* pcFullFileName, char chSeparator):
                      CFileReader(pcFullFileName)							
{
  std::vector<uint16_t> lszInputCoumunNo = {
                                              IGNORE_SYMBOL_NAME,
                                           };
  m_cStringSplit.UpdateSplitparam(chSeparator, lszInputCoumunNo);
}

CIgnoreSymbolReader::~CIgnoreSymbolReader()
{

}

bool CIgnoreSymbolReader::GetNextRow(char* pcSymbol)
{
  int32_t lnCount = 0;
  int32_t lnStringCount = 0;  
  
  std::vector<std::string> lszSplitStrings;  
	if(GetNextLine())
	{
    if(m_cStringSplit.GetData(m_cRowRecords, lszSplitStrings))
    {
      lnCount = -1;
      if(!lszSplitStrings.empty()) 
      {
        lnCount = 0;
        lnStringCount = lszSplitStrings.size();
      }
      
      auto lcItr = m_cStringSplit.GetColumnList().begin();
      for(; lcItr != m_cStringSplit.GetColumnList().end() && lnCount < lnStringCount; ++lcItr)
      {
        switch(*lcItr)
        {
          case IGNORE_SYMBOL_NAME:
          {
            strncpy(pcSymbol, lszSplitStrings[lnCount++].c_str(), SCRIP_SYMBOL_LEN);				
          }
          break;
        }
      }
    }
  }
  else
	{
		return false;
	}  
  return true;
}


CSymbolFilterReader::CSymbolFilterReader(const char* pcFullFileName, char chSeparator):
                      CFileReader(pcFullFileName)
{
  std::vector<uint16_t> lszInputCoumunNo = {SYMBOL_FILTER_SYMBOL_NAME,       SYMBOL_FILTER_CURR_MONTH_LIST,      SYMBOL_FILTER_NEXT_MONTH_LIST,   
                                            SYMBOL_FILTER_FAR_MONTH_LIST,    SYMBOL_FILTER_X_MONTH_LIST,    
                                           };
  m_cStringSplit.UpdateSplitparam(chSeparator, lszInputCoumunNo);
}

CSymbolFilterReader::~CSymbolFilterReader()
{

}

bool CSymbolFilterReader::GetNextRow(tagIndividualSymbolDetails& stIndividualSymbolDetails, char* pcStrikePriceList1, char* pcStrikePriceList2, char* pcStrikePriceList3)
{
  int32_t lnCount = 0;
  int32_t lnStringCount = 0;
  
  std::vector<std::string> lszSplitStrings;
	if(GetNextLine())
	{
    if(m_cStringSplit.GetData(m_cRowRecords, lszSplitStrings))
    {
      lnCount = -1;      
      if(!lszSplitStrings.empty()) 
      {
        lnCount = 0;
        lnStringCount = lszSplitStrings.size();
      }        
        
      auto lcItr = m_cStringSplit.GetColumnList().begin();
      for(; lcItr != m_cStringSplit.GetColumnList().end() && lnCount <= lnStringCount; ++lcItr)
      {
        switch(*lcItr)
        {
          case SYMBOL_FILTER_SYMBOL_NAME :
          {
            strncpy(stIndividualSymbolDetails.cSymbol, lszSplitStrings[lnCount++].c_str(), SCRIP_SYMBOL_LEN);				
          }
          break;

          case SYMBOL_FILTER_CURR_MONTH_LIST:
          {
            strncpy(pcStrikePriceList1, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;

          case SYMBOL_FILTER_NEXT_MONTH_LIST:
          {
            strncpy(pcStrikePriceList2, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;

          case SYMBOL_FILTER_FAR_MONTH_LIST:
          {
            strncpy(pcStrikePriceList3, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;
        }
      }
    }
	}
	else
	{
		return false;
	}
  
  return true;
}

CNiftySymbolFilterReader::CNiftySymbolFilterReader(const char* pcFullFileName, char chSeparator):
                          CFileReader(pcFullFileName)							

{
  std::vector<uint16_t> lszInputCoumunNo = {SYMBOL_FILTER_SYMBOL_NAME,       NIFTY_SP_FILTER_1,      NIFTY_SP_FILTER_2,   
                                            NIFTY_SP_FILTER_3,               NIFTY_SP_FILTER_4,      NIFTY_SP_FILTER_5,
                                            NIFTY_SP_FILTER_6,               NIFTY_SP_FILTER_7,      NIFTY_SP_FILTER_8,
                                            NIFTY_SP_FILTER_9,               NIFTY_SP_FILTER_10,     NIFTY_SP_FILTER_11,
                                            NIFTY_SP_FILTER_12
                                           };
  m_cStringSplit.UpdateSplitparam(chSeparator, lszInputCoumunNo);
}

CNiftySymbolFilterReader::~CNiftySymbolFilterReader()
{

}

/*bool CNiftySymbolFilterReader::GetNextRow(std::vector<std::string>& szSplitStrings)
{
	if(GetNextLine())
	{
    if(m_cStringSplit.GetData(m_cRowRecords, szSplitStrings))
    {      
      lnCount = -1;      
      if(szSplitStrings.empty()) 
      {
        lnCount = 0;
        lnStringCount = szSplitStrings.size();
      }        

      auto lcItr = m_cStringSplit.GetColumnList().begin();
      for(; lcItr != m_cStringSplit.GetColumnList().end() && lnCount <= lnStringCount; ++lcItr)
      {
        switch(*lcItr)
        {
          case SYMBOL_FILTER_SYMBOL_NAME :
          {
            strncpy(stSymbolRangeDetails.cSymbol, lszSplitStrings[lnCount++].c_str(), SCRIP_SYMBOL_LEN);				
          }
          break;

          case NIFTY_SP_FILTER_1:
          {
            strncpy(pcStrikePriceList1, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;
          
          case NIFTY_SP_FILTER_2:
          {
            strncpy(pcStrikePriceList1, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;
          
          case NIFTY_SP_FILTER_3:
          {
            strncpy(pcStrikePriceList1, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;
          
          case NIFTY_SP_FILTER_4:
          {
            strncpy(pcStrikePriceList1, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;
          
          case NIFTY_SP_FILTER_5:
          {
            strncpy(pcStrikePriceList1, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;
          
          case NIFTY_SP_FILTER_6:
          {
            strncpy(pcStrikePriceList1, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;
          
          case NIFTY_SP_FILTER_7:
          {
            strncpy(pcStrikePriceList1, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;
          
          case NIFTY_SP_FILTER_8:
          {
            strncpy(pcStrikePriceList1, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;          
          
          case NIFTY_SP_FILTER_9:
          {
            strncpy(pcStrikePriceList1, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;                    
          
          case NIFTY_SP_FILTER_10:
          {
            strncpy(pcStrikePriceList1, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;                              
          
          case NIFTY_SP_FILTER_11:
          {
            strncpy(pcStrikePriceList1, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;                                        
          
          case NIFTY_SP_FILTER_11:
          {
            strncpy(pcStrikePriceList1, lszSplitStrings[lnCount++].c_str(), STRIKE_PRICE_LIST);				            
          }
          break;                                        
        }
      }   
    }
	}
	else
	{
		return false;
	}
  
  return true;
}
*/   
