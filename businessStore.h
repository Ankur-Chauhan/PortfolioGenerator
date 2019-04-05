#ifndef _BUSINESS_STORE_H_
#define _BUSINESS_STORE_H_

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <list>

#include "stringsplit.h"
#include "struct_char.h"

typedef std::unordered_map<int32_t, OMSScripInfo*> OMSScripInfoStore;
typedef OMSScripInfoStore::iterator OMSScripInfoStoreITR;

const std::string MONTH[] = {"JAN","FEB","MAR","APR","MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

struct tagPortfolioDetails
{
  int16_t             m_StrategyType;
  int32_t             m_nToken1;
  int32_t             m_nToken2;    
  int32_t             m_nToken3;        
  int32_t             m_nStrikePrice;            
  int32_t             m_nExpiryDate;     
  char                cSymbol[SCRIP_SYMBOL_LEN + 1];	    
};

class CSPTokenDetails
{
  public:
    CSPTokenDetails(const int32_t& nStrikePrice);
    ~CSPTokenDetails();
    void UpdateToken(const OMSScripInfo& stOMSScripInfo);
    void UpdateToken1(const int32_t nToken);    
    
  public:
    int32_t             m_nToken1;
    int32_t             m_nToken2;    
    int32_t             m_nToken3;        
    int32_t             m_nStrikePrice;            
};

typedef std::map<int32_t, CSPTokenDetails*>SPTokenStore;
typedef SPTokenStore::iterator SPTokenStoreITR;
typedef std::set<int32_t> TokenSet;

class CSPTokenDetailStore
{
  public:
    CSPTokenDetailStore(const int32_t nExpiryDate, const std::string& szExpiryMonth);
    ~CSPTokenDetailStore();
      
  public:
    bool AddSymbol(const OMSScripInfo& stOMSScripInfo);
    CSPTokenDetails* GetSPTokenDetails(const int32_t nStrikePrice);    
    bool UpdateFutureToken(const int32_t nToken);
    int32_t GetToken1();
    
    void UpdateToken(int32_t nStrikePrice, int32_t nToken1);

  public:    
    SPTokenStore          m_cSPTokenStore;
    TokenSet              m_TokenSet;
    int32_t               m_nFutureToken;
    int32_t               m_nExpiryDate;    
    std::string           m_szExpiryMonth;
};

typedef std::map<int32_t, CSPTokenDetailStore*> Expiry2SPInfoStore;
typedef Expiry2SPInfoStore::iterator Expiry2SPInfoStoreITR;

class CExpiryDateStore
{
  public:
    CExpiryDateStore(const std::string& szSymbol);
    ~CExpiryDateStore();
  
  public:
    bool AddSymbol(const OMSScripInfo& stOMSScripInfo);
    void UpdateToken1(const OMSScripInfo& stOMSScripInfo);
    CSPTokenDetailStore* GetSPTokenDetailStore(const std::string szExpiryMonth);
    
  public:
    std::string               m_szSymbol;
    Expiry2SPInfoStore        m_cExpiry2SPInfoStore;
    Expiry2SPInfoStore        m_cFutExpiry2SPInfoStore;    
};

typedef std::map<std::string, CExpiryDateStore*>Symbol2ExpiryStore;
typedef Symbol2ExpiryStore::iterator Symbol2ExpiryStoreITR;

class CSymbolMaster
{
  public:
    CSymbolMaster();
    ~CSymbolMaster();
    
  public:
    bool AddSymbol(const OMSScripInfo& stOMSScripInfo);    
    CExpiryDateStore* GetExpiryDateStore(const std::string& szSymbol);
    bool GetConRevPortFolio(int16_t nInstanceId, int32_t nSingleOrderLot, int32_t nMaxPositionLot, int32_t nConSpread, int32_t nRevSpread);
    bool ValidateTokenList();
    
  public:
    Symbol2ExpiryStore          m_cSymbol2ExpiryStore;
};

typedef std::list<int32_t>                                  StrikePriceList;
typedef StrikePriceList::iterator                           StrikePriceListItr;


class CStrikeParamStore
{
  public:
    CStrikeParamStore();
    ~CStrikeParamStore();
    
  public:
    void UpdateStrikePrice(int32_t nStrikePrice);
    
  public:
    char                        m_cExpiryDate;
    StrikePriceList             m_cStrikePriceList;
    std::set<std::string>       m_nWeeklyIdentifier;    
};

typedef std::unordered_map<std::string, CStrikeParamStore*> StrikeParamStore;
typedef StrikeParamStore::iterator                          StrikeParamStoreItr;

class CExpiryParam
{
  public:
    CExpiryParam(const std::string& szExpiryDate);
    ~CExpiryParam();
  
  public:
    bool UpdateExpiryNStrikePrices(const tagIndividualSymbolDetails& stIndividualSymbolDetails, std::string& szExpiryStr);    
    CStrikeParamStore* GetStrikeParamStore(std::string& szExpiryStr);
    
  public:
    StrikeParamStore            m_cStrikeParamStore;
    std::string                 m_szExpiryDate;    
};

typedef std::unordered_map<std::string, CExpiryParam*>      SymbolParam;
typedef SymbolParam::iterator                               SymbolParamItr;

typedef std::list<std::string>                              SymbolListParam;
typedef SymbolListParam::iterator                           SymbolListParamItr;

class CSymbolParam
{
  public:
    CSymbolParam();
    ~CSymbolParam();
    
  public:
    bool UpdateSymbolParam(const tagIndividualSymbolDetails& stIndividualSymbolDetails, std::string& szExpiryStr);
    void PrintSymbolList();
    CExpiryParam* GetExpiryParam(const std::string& szSymbol);
    
  public:
    SymbolParam               m_cSymbol2ExpiryParamStore;
    SymbolListParam           m_cSymbolListParam;
};

typedef std::unordered_set<std::string>SymbolStore;
typedef SymbolStore::iterator SymbolStoreItr;

typedef std::set<int32_t>ExpiryDateStore;
typedef ExpiryDateStore::iterator ExpiryDateStoreItr;

typedef std::vector<std::string>SymbolStoreList;
typedef SymbolStoreList::iterator SymbolStoreListItr;


class CBusinessStore
{
  public:
    CBusinessStore();
    ~CBusinessStore();
    
  public:
    enum EXPIRY_TYPE : uint16_t
    {
      MONTHLY_EXPIRY = 1,
      WEEKLY_EXPIRY = 2,
    };
    
  public:
    bool AddOMSScripInfo(const OMSScripInfo& stOMSScripInfo);
    OMSScripInfo* GetOMSScripInfo(int32_t nTokenNumber);
    bool IsValidSymbol(const std::string& szSymbol);
    
    bool StrategyType(int16_t nStrategyType);
    void UpdateHolidayCount(int16_t nMaxHolidayCount);    
    
    bool AddIgnoreSymbol(char* pcSymbolName);
    bool IsValidIgnoreSymbol(const std::string& szSymbol);    
    
    void GetPortFolioList(std::vector<tagPortfolioDetails>& vPortfolioDetailsStore, const int16_t& nStratType);    
    void GetPortFolioList1(std::vector<tagPortfolioDetails>& vPortfolioDetailsStore, CExpiryDateStore* pcExpiryDateStore, CSPTokenDetailStore* pcSPTokenDetailStore, CStrikeParamStore*  lpcStrikeParamStore);    
    void GetPortFolioInVector(std::vector<tagPortfolioDetails>& vPortfolioDetailsStore, CExpiryDateStore* pcExpiryDateStore, CSPTokenDetailStore* pcSPTokenDetailStore, CSPTokenDetails* pcSPTokenDetails);
    bool ValidateTokenList();
    
    bool AddSymbolParam(const std::string& szSymbol);        
    bool UpdateExpiryNStrikePrice(const tagIndividualSymbolDetails& stIndividualSymbolDetails);    
    bool PrintSymbolList();
    
    bool ValidateSymbolParam(tagIndividualSymbolDetails& stIndividualSymbolDetails, SymbolStore& cSymbolStoreTemp, StringSplit& cStringSplit, char* pcStrikePriceList1, char* pcStrikePriceList2, char* pcStrikePriceList3);
    bool BuildSymbolParam(tagIndividualSymbolDetails& lstIndividualSymbolDetails, StringSplit& cStringSplit, char* pcStrikePriceList1, char* pcStrikePriceList2, char* pcStrikePriceList3);
    bool WeeklyExpiryStore(const std::string szString, tagIndividualSymbolDetails& lstIndividualSymbolDetails, char chSeparator);
    bool PrintMapExp();
    
    time_t GetLowestExpiryDate();
    //3155
    static std::string GetMonthInStr(int32_t nExpiryDate);
    static int GetMonthInNumber(int32_t nExpiryDate);    
    
    static int GetMonthInterval(int32_t nDate, int16_t nMonthInterval);
    static std::string GetMonthInStrInterval(int32_t nDate, int16_t nMonthInterval);    
    static bool IsMonthlyExpiry(int32_t nExpiryDate,  int16_t nMaxHoliday);        
    
    static std::string GetExpiryMonth(const char cExpiryIdentifer, const time_t& nExpiryDate);
    
  private:
    OMSScripInfoStore         m_cOMSScripInfoStore; 
    CSymbolMaster             m_cSymbolMaster;
    CSymbolParam              m_cSymbolParam;
    ExpiryDateStore           m_cExpiryDateStore;
    SymbolStore               m_cSymbolStore;    
    SymbolStore               m_cIgnoreSymbolStore;  
    int16_t                   m_nStrategyType;
    int16_t                   m_nMaxHolidayCount;
};

#endif
