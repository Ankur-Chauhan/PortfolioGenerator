#include <stdint.h>
#include <list>
#include <vector>
#include <fstream>
#include <map>

#include "businessStore.h"


/*********************************************************CSPTokenDetails******************************************************************/

CSPTokenDetails::CSPTokenDetails(const int32_t& nStrikePrice) : m_nStrikePrice(nStrikePrice)
{
  m_nToken1   = 0;
  m_nToken2   = 0;    
  m_nToken3   = 0;            
}

CSPTokenDetails::~CSPTokenDetails()
{

}

void CSPTokenDetails::UpdateToken(const OMSScripInfo& stOMSScripInfo)
{
  switch(stOMSScripInfo.nCustomInstrumentType)
  {
    case INSTRUMENT_FUTURE:
    {
      m_nToken1 = stOMSScripInfo.nToken;
    }
    break;
    
    case INSTRUMENT_PUT:
    {
      m_nToken2 = stOMSScripInfo.nToken;      
    }
    break;
    
    case INSTRUMENT_CALL:
    {
      m_nToken3 = stOMSScripInfo.nToken;          
    }
    break;
  }
  //std::cout << "UpdateToken StrikePrice|"<<m_nStrikePrice<<"|Token1|" << m_nToken1 << "|Token2|"<<m_nToken2 <<"|Token3|"<<m_nToken3<< std::endl;
  return;
}

void CSPTokenDetails::UpdateToken1(const int32_t nToken)
{
  m_nToken1 = nToken;
  //std::cout << "UpdateToken1 StrikePrice|"<<m_nStrikePrice<<"|Token1|" << m_nToken1 << "|Token2|"<<m_nToken2 <<"|Token3|"<<m_nToken3<< std::endl;
}

/*********************************************************CSPTokenDetails******************************************************************/



/*********************************************************CSPTokenDetailStore******************************************************************/

CSPTokenDetailStore::CSPTokenDetailStore(const int32_t nExpiryDate, const std::string& szExpiryMonth)
{
  m_szExpiryMonth = szExpiryMonth;
  m_nExpiryDate   = nExpiryDate;
  m_nFutureToken  = 0;
}

CSPTokenDetailStore::~CSPTokenDetailStore()
{
  for(auto lcItr = m_cSPTokenStore.begin(); lcItr != m_cSPTokenStore.end(); ++lcItr)
  {
    delete lcItr->second;
  }
  m_cSPTokenStore.clear(); 
}

bool CSPTokenDetailStore::AddSymbol(const OMSScripInfo& stOMSScripInfo)
{
  CSPTokenDetails*  lpcSPTokenDetails  = NULL;
  SPTokenStoreITR lcItr = m_cSPTokenStore.find(stOMSScripInfo.stSymbolDetails.nStrikePrice);
  if(lcItr == m_cSPTokenStore.end())
  {
    lpcSPTokenDetails = new CSPTokenDetails(stOMSScripInfo.stSymbolDetails.nStrikePrice);
    std::pair<SPTokenStoreITR, bool>lcRetValue;
    lcRetValue = m_cSPTokenStore.insert(SPTokenStore::value_type(lpcSPTokenDetails->m_nStrikePrice, lpcSPTokenDetails));
    if(lcRetValue.second == false)
    {
      //std::cout << "Failed to added Token|Strike " << stOMSScripInfo.nToken << "|" << lpcSPTokenDetails->m_nStrikePrice << std::endl;
      delete lpcSPTokenDetails;
      return false;
    }
    m_TokenSet.insert(stOMSScripInfo.stSymbolDetails.nStrikePrice);
  }
  else
  {
    lpcSPTokenDetails = lcItr->second;
  }
  
  lpcSPTokenDetails->UpdateToken(stOMSScripInfo);
  
  return true;
}

CSPTokenDetails* CSPTokenDetailStore::GetSPTokenDetails(const int32_t nStrikePrice)
{
  CSPTokenDetails*  lpcSPTokenDetails  = NULL;
  SPTokenStoreITR lcItr = m_cSPTokenStore.find(nStrikePrice);
  if(lcItr != m_cSPTokenStore.end())
  {
    lpcSPTokenDetails = lcItr->second;
  }
  
  return lpcSPTokenDetails;
}

bool CSPTokenDetailStore::UpdateFutureToken(const int32_t nToken)
{
  m_nFutureToken = nToken;
  CSPTokenDetails* lpcSPTokenDetails  = NULL;  
  SPTokenStoreITR lcItr = m_cSPTokenStore.begin();
  for(; lcItr != m_cSPTokenStore.end(); ++lcItr)
  {
    lpcSPTokenDetails = lcItr->second;
    if(NULL != lpcSPTokenDetails)
    {
      lpcSPTokenDetails->m_nToken1 = nToken;
    }
    //std::cout << "Update Token1 " <<  lpcSPTokenDetails->m_nToken1 << " " << lpcSPTokenDetails->m_nStrikePrice  << std::endl;
  }
  
  return true;
}

int32_t CSPTokenDetailStore::GetToken1()
{
  return m_nFutureToken;
}

void CSPTokenDetailStore::UpdateToken(int32_t nStrikePrice, int32_t nToken1)
{
  SPTokenStoreITR lcItr = m_cSPTokenStore.find(nStrikePrice);
  if(lcItr != m_cSPTokenStore.end())
  {
    lcItr->second->UpdateToken1(nToken1);
  }
  return;
}

/*********************************************************CSPTokenDetailStore******************************************************************/


/*********************************************************CExpiryDateStore******************************************************************/


CExpiryDateStore::CExpiryDateStore(const std::string& szSymbol)
{
  m_szSymbol = szSymbol;
}

CExpiryDateStore::~CExpiryDateStore()
{
  for(auto lcItr = m_cFutExpiry2SPInfoStore.begin(); lcItr != m_cFutExpiry2SPInfoStore.end(); ++lcItr)
  {
    delete lcItr->second;
  }
  m_cFutExpiry2SPInfoStore.clear();   
  
  for(auto lcItr = m_cExpiry2SPInfoStore.begin(); lcItr != m_cExpiry2SPInfoStore.end(); ++lcItr)
  {
    delete lcItr->second;
  }
  m_cExpiry2SPInfoStore.clear();   
}
  
bool CExpiryDateStore::AddSymbol(const OMSScripInfo& stOMSScripInfo)
{
  switch(stOMSScripInfo.nCustomInstrumentType)
  {
    case INSTRUMENT_FUTURE:
    {
      CSPTokenDetailStore* lpcSPTokenDetailStore = NULL;
      Expiry2SPInfoStoreITR lcItr = m_cFutExpiry2SPInfoStore.find(stOMSScripInfo.stSymbolDetails.nExpiryDate);
      if(lcItr == m_cFutExpiry2SPInfoStore.end())
      {
        int32_t lnExpiryDate = stOMSScripInfo.stSymbolDetails.nExpiryDate + 315532800;         
        std::string lszExpiryMonth = CBusinessStore::GetMonthInStr(lnExpiryDate);        
        lpcSPTokenDetailStore = new CSPTokenDetailStore(stOMSScripInfo.stSymbolDetails.nExpiryDate, lszExpiryMonth);

        std::pair<Expiry2SPInfoStoreITR, bool>lcRetValue;    
        lcRetValue = m_cFutExpiry2SPInfoStore.insert(Expiry2SPInfoStore::value_type(stOMSScripInfo.stSymbolDetails.nExpiryDate, lpcSPTokenDetailStore));
        if(lcRetValue.second == false)
        {
          std::cout << "Failed to added Token|ExpiryDate|StrikePrice|" << stOMSScripInfo.nToken << "|" << stOMSScripInfo.stSymbolDetails.nExpiryDate << "|" << stOMSScripInfo.stSymbolDetails.nStrikePrice << std::endl;
          delete lpcSPTokenDetailStore;
          return false;
        }
        lpcSPTokenDetailStore->m_nFutureToken = stOMSScripInfo.nToken;
        UpdateToken1(stOMSScripInfo);
      }
      else
      {
        //std::cout << "Already Added  Future Symbol|Token|ExpiryDate|StrikePrice|" << stOMSScripInfo.stSymbolDetails.cSymbol << "|" << stOMSScripInfo.nToken << "|" << stOMSScripInfo.stSymbolDetails.nExpiryDate << "|" << stOMSScripInfo.stSymbolDetails.nStrikePrice << std::endl;              
      }
    }
    break;
    
    case INSTRUMENT_CALL:
    case INSTRUMENT_PUT:
    {
      CSPTokenDetailStore* lpcSPTokenDetailStore = NULL;
      Expiry2SPInfoStoreITR lcItr = m_cExpiry2SPInfoStore.find(stOMSScripInfo.stSymbolDetails.nExpiryDate);
      if(lcItr == m_cExpiry2SPInfoStore.end())
      {
        int32_t lnExpiryDate = stOMSScripInfo.stSymbolDetails.nExpiryDate + 315532800; 
        std::string lszExpiryMonth = CBusinessStore::GetMonthInStr(lnExpiryDate);
        lpcSPTokenDetailStore = new CSPTokenDetailStore(stOMSScripInfo.stSymbolDetails.nExpiryDate, lszExpiryMonth);

        std::pair<Expiry2SPInfoStoreITR, bool>lcRetValue;    
        lcRetValue = m_cExpiry2SPInfoStore.insert(Expiry2SPInfoStore::value_type(stOMSScripInfo.stSymbolDetails.nExpiryDate, lpcSPTokenDetailStore));
        if(lcRetValue.second == false)
        {
          std::cout << "Failed to added Token|ExpiryDate|StrikePrice|" << stOMSScripInfo.nToken << "|" << stOMSScripInfo.stSymbolDetails.nExpiryDate << "|" << stOMSScripInfo.stSymbolDetails.nStrikePrice << std::endl;
          delete lpcSPTokenDetailStore;
          return false;
        }
        
        if(!m_cExpiry2SPInfoStore.empty())
        {
          int32_t lnSPPriceCount = m_cExpiry2SPInfoStore.size();
          if(lnSPPriceCount > MAX_PORTFOLIO_PER_TOKEN)
          {
            std::cout << "Strike Price Count is more then " << MAX_PORTFOLIO_PER_TOKEN  << " for Expiry Month " << lszExpiryMonth  << std::endl;          
            return false;
          }
        }
        //std::cout << "Added  Symbol|Token|ExpiryDate|StrikePrice|" << stOMSScripInfo.stSymbolDetails.cSymbol << "|" << stOMSScripInfo.nToken << "|" << stOMSScripInfo.stSymbolDetails.nExpiryDate << "|" << stOMSScripInfo.stSymbolDetails.nStrikePrice << std::endl;    
      }
      else
      {
        lpcSPTokenDetailStore = lcItr->second;
      }  
      lpcSPTokenDetailStore->AddSymbol(stOMSScripInfo);
      lcItr = m_cFutExpiry2SPInfoStore.find(stOMSScripInfo.stSymbolDetails.nExpiryDate);
      if(lcItr != m_cFutExpiry2SPInfoStore.end())
      {
        int32_t lnToken1 = lcItr->second->GetToken1();
        lpcSPTokenDetailStore->UpdateToken(stOMSScripInfo.stSymbolDetails.nStrikePrice, lnToken1);
      }
      else
      {
        for(auto ITR = m_cFutExpiry2SPInfoStore.begin(); ITR != m_cFutExpiry2SPInfoStore.end(); ITR++)
        {
          if(ITR->second->m_szExpiryMonth == CBusinessStore::GetMonthInStr(stOMSScripInfo.stSymbolDetails.nExpiryDate))
          {
            //std::cout<< ITR->second->m_szExpiryMonth << "|"<< CBusinessStore::GetMonthInStr(stOMSScripInfo.stSymbolDetails.nExpiryDate)<<
              //"|"<< stOMSScripInfo.stSymbolDetails.nExpiryDate<< std::endl;
            int32_t lnToken1 = ITR->second->m_nFutureToken;
            lpcSPTokenDetailStore->UpdateToken(stOMSScripInfo.stSymbolDetails.nStrikePrice, lnToken1);
          }
        }
      }
    }
    break;
  }
  
  return true;
}

void CExpiryDateStore::UpdateToken1(const OMSScripInfo& stOMSScripInfo)
{
  CSPTokenDetailStore* lpcSPTokenDetailStore = NULL;
  Expiry2SPInfoStoreITR lcItr = m_cExpiry2SPInfoStore.find(stOMSScripInfo.stSymbolDetails.nExpiryDate);
  if(lcItr != m_cExpiry2SPInfoStore.end())
  {
    //std::cout << "Expiry date " << stOMSScripInfo.stSymbolDetails.nExpiryDate  << " Token " << stOMSScripInfo.nToken << std::endl;
    lpcSPTokenDetailStore = lcItr->second;
    if(NULL != lpcSPTokenDetailStore)
    {
      lpcSPTokenDetailStore->UpdateFutureToken(stOMSScripInfo.nToken);
    }
  }
  else
  {
    for(auto ITR = m_cExpiry2SPInfoStore.begin(); ITR != m_cExpiry2SPInfoStore.end(); ITR++)
    {
      if(ITR->second->m_szExpiryMonth == CBusinessStore::GetMonthInStr(stOMSScripInfo.stSymbolDetails.nExpiryDate))
      {
        //std::cout<< "X:"<< ITR->second->m_szExpiryMonth<< stOMSScripInfo.stSymbolDetails.nExpiryDate<< std::endl;
        lpcSPTokenDetailStore = ITR->second;
        lpcSPTokenDetailStore->UpdateFutureToken(stOMSScripInfo.nToken);
      }
    }
  }
  return;
}

CSPTokenDetailStore* CExpiryDateStore::GetSPTokenDetailStore(const std::string szExpiryMonth)
{
  CSPTokenDetailStore* lpcSPTokenDetailStore = NULL;
  Expiry2SPInfoStoreITR lcItr = m_cExpiry2SPInfoStore.begin();
  for(;lcItr != m_cExpiry2SPInfoStore.end(); ++lcItr)
  {
    lpcSPTokenDetailStore = lcItr->second;
    if(NULL != lpcSPTokenDetailStore)
    {
      if(lpcSPTokenDetailStore->m_szExpiryMonth == szExpiryMonth)
      {
        return lpcSPTokenDetailStore;
      }
    }
  }
  return lpcSPTokenDetailStore;
}

/*********************************************************CExpiryDateStore******************************************************************/      
/*********************************************************CSymbolMaster******************************************************************/      

CSymbolMaster::CSymbolMaster()
{
}

CSymbolMaster::~CSymbolMaster()
{
  for(auto lcItr = m_cSymbol2ExpiryStore.begin(); lcItr != m_cSymbol2ExpiryStore.end(); ++lcItr)
  {
    delete lcItr->second;
  }
  m_cSymbol2ExpiryStore.clear();   
}

bool CSymbolMaster::AddSymbol(const OMSScripInfo& stOMSScripInfo)
{
  CExpiryDateStore* lpcExpiryDateStore = NULL;
  Symbol2ExpiryStoreITR lcItr = m_cSymbol2ExpiryStore.find(stOMSScripInfo.stSymbolDetails.cSymbol);
  if(lcItr == m_cSymbol2ExpiryStore.end())
  {
    lpcExpiryDateStore = new CExpiryDateStore(stOMSScripInfo.stSymbolDetails.cSymbol);
    std::string lszSymbol = stOMSScripInfo.stSymbolDetails.cSymbol;
    std::pair<Symbol2ExpiryStoreITR, bool>lcRetValue;
    lcRetValue = m_cSymbol2ExpiryStore.insert(Symbol2ExpiryStore::value_type(lszSymbol, lpcExpiryDateStore));
    if(lcRetValue.second == false)
    {
      std::cout << "Failed to added Symbol|Token|ExpiryDate|StrikePrice|" << stOMSScripInfo.stSymbolDetails.cSymbol << "|" << stOMSScripInfo.nToken << "|" << stOMSScripInfo.stSymbolDetails.nExpiryDate << "|" << stOMSScripInfo.stSymbolDetails.nStrikePrice << std::endl;
      delete lpcExpiryDateStore;
      return false;
    }
    //std::cout << "Added Symbol|Token|ExpiryDate|StrikePrice|" << stOMSScripInfo.stSymbolDetails.cSymbol << "|" << stOMSScripInfo.nToken << "|" << stOMSScripInfo.stSymbolDetails.nExpiryDate << "|" << stOMSScripInfo.stSymbolDetails.nStrikePrice << std::endl;    
  }
  else
  {
    lpcExpiryDateStore = lcItr->second;
  }
  
  return lpcExpiryDateStore->AddSymbol(stOMSScripInfo);
}

CExpiryDateStore* CSymbolMaster::GetExpiryDateStore(const std::string& szSymbol)
{
  CExpiryDateStore* lpcExpiryDateStore = NULL;
  Symbol2ExpiryStoreITR lcItr = m_cSymbol2ExpiryStore.find(szSymbol);
  if(lcItr != m_cSymbol2ExpiryStore.end())
  {
    lpcExpiryDateStore = lcItr->second;
  }
  return lpcExpiryDateStore;
}

bool CSymbolMaster::ValidateTokenList()
{
  bool lbStatus = true;
  Symbol2ExpiryStoreITR lcItr0 = m_cSymbol2ExpiryStore.begin();
  for(; lcItr0 != m_cSymbol2ExpiryStore.end(); ++lcItr0)
  {
    CExpiryDateStore* lpcExpiryDateStore = lcItr0->second;
    Expiry2SPInfoStoreITR lcItr2 = lpcExpiryDateStore->m_cExpiry2SPInfoStore.begin();
    for(; lcItr2 != lpcExpiryDateStore->m_cExpiry2SPInfoStore.end(); ++lcItr2)
    {
      CSPTokenDetailStore* lpcSPTokenDetailStore = lcItr2->second;
      SPTokenStoreITR lcItr3 = lpcSPTokenDetailStore->m_cSPTokenStore.begin();
      for(; lcItr3 != lpcSPTokenDetailStore->m_cSPTokenStore.end(); ++lcItr3)
      {
        CSPTokenDetails* lpcSPTokenDetails = lcItr3->second;
        //std::cout<< lpcSPTokenDetails->m_nStrikePrice<< "|"<< lpcSPTokenDetails->m_nToken1<< "|"<< lpcSPTokenDetails->m_nToken2<< "|"<< lpcSPTokenDetails->m_nToken3<< std::endl;
        if((lpcSPTokenDetails->m_nToken1 == 0 || lpcSPTokenDetails->m_nToken2 == 0 || lpcSPTokenDetails->m_nToken3 == 0))
        {
          std::cout << "Error invalid Token List  StrikePrice|" << lpcSPTokenDetails->m_nStrikePrice<< "|Token1|"<< lpcSPTokenDetails->m_nToken1 << "|Token2|" << lpcSPTokenDetails->m_nToken2 << "|Token3|" << lpcSPTokenDetails->m_nToken3 << std::endl;
          lbStatus = false;
        }
      }
    }
    
  }  
  
  return lbStatus;
}

bool CSymbolMaster::GetConRevPortFolio(int16_t nInstanceId, int32_t nSingleOrderLot, int32_t nMaxPositionLot, int32_t nConSpread, int32_t nRevSpread)
{
  /*
  Symbol2ExpiryStoreITR lcItr0 = m_cSymbol2ExpiryStore.begin();
  for(; lcItr0 != m_cSymbol2ExpiryStore.end(); ++lcItr0)
  {
    CExpiryDateStore* lpcExpiryDateStore = lcItr0->second;
    Expiry2SPInfoStoreITR lcItr1 = lpcExpiryDateStore->m_cExpiry2SPInfoStore.begin();
    for(; lcItr1 != lpcExpiryDateStore->m_cExpiry2SPInfoStore.end(); ++lcItr1)
    {
      Expiry2SPInfoStoreITR lcItr1 = lpcExpiryDateStore->m_cExpiry2SPInfoStore.begin();
      for(; lcItr1 != lpcExpiryDateStore->m_cExpiry2SPInfoStore.end(); ++lcItr1)
      {
        CSPTokenDetailStore* lpcSPTokenDetailStore = lcItr1->second;
        SPTokenStoreITR lcItr2 = lpcSPTokenDetailStore->m_cSPTokenStore.begin();
        for(; lcItr2 != lpcSPTokenDetailStore->m_cSPTokenStore.end(); ++lcItr2)
        {
          CSPTokenDetails* lpcSPTokenDetails = lcItr2->second;
          
          std::cout << nInstanceId                          << "," 
                    << "1"                                  << "," 
                    << lpcSPTokenDetails->m_nToken1         << ","
                    << lpcSPTokenDetails->m_nToken2         << ","
                    << lpcSPTokenDetails->m_nToken3         << ","
                    << nSingleOrderLot                      << ","
                    << nMaxPositionLot                      << ","
                    << nConSpread                           << ","
                    << lpcExpiryDateStore->m_szSymbol       << "," 
                    << lpcSPTokenDetails->m_nStrikePrice    << ","
                    << lpcSPTokenDetailStore->m_nExpiryDate << "," 
                    << std::endl;
            
          std::cout << nInstanceId                          << "," 
                    << "2"                                  << "," 
                    << lpcSPTokenDetails->m_nToken1         << ","
                    << lpcSPTokenDetails->m_nToken2         << ","
                    << lpcSPTokenDetails->m_nToken3         << ","
                    << nSingleOrderLot                      << ","
                    << nMaxPositionLot                      << ","
                    << nRevSpread                           << ","
                    << lpcExpiryDateStore->m_szSymbol       << "," 
                    << lpcSPTokenDetails->m_nStrikePrice    << ","
                    << lpcSPTokenDetailStore->m_nExpiryDate << "," 
                    << std::endl;
        }
      }
    }
  }  
  */
  return true;
}

/*********************************************************CSymbolMaster******************************************************************/          

/*********************************************************CStrikeParamStore******************************************************************/          


CStrikeParamStore::CStrikeParamStore()
{

}

CStrikeParamStore::~CStrikeParamStore()
{

}
    
void CStrikeParamStore::UpdateStrikePrice(int32_t nStrikePrice)
{
  m_cStrikePriceList.push_back(nStrikePrice);
  return;
}

/*********************************************************CStrikeParamStore******************************************************************/          

/*********************************************************CExpiryParam******************************************************************/          

CExpiryParam::CExpiryParam(const std::string& szExpiryDate):m_szExpiryDate(szExpiryDate)
{
  //std::cout<< "m_szExpiryDate:"<< m_szExpiryDate<< std::endl;
}

CExpiryParam::~CExpiryParam()
{
  for(auto lcItr = m_cStrikeParamStore.begin(); lcItr != m_cStrikeParamStore.end(); ++lcItr)
  {
    delete lcItr->second;
  }
  m_cStrikeParamStore.clear(); 
}

bool CExpiryParam::UpdateExpiryNStrikePrices(const tagIndividualSymbolDetails& stIndividualSymbolDetails, std::string& szExpiryStr)
{
  //szExpiryStr = CBusinessStore::GetExpiryMonth(stIndividualSymbolDetails.cExpiryDate);
  CStrikeParamStore* lpcStrikeParamStore = NULL;
  StrikeParamStoreItr lcItr = m_cStrikeParamStore.find(szExpiryStr);
  if(lcItr == m_cStrikeParamStore.end())
  {    
    lpcStrikeParamStore = new CStrikeParamStore();
    lpcStrikeParamStore->m_nWeeklyIdentifier = stIndividualSymbolDetails.bWeeklyExpirySet;
    std::pair<StrikeParamStoreItr, bool>lcRetValue;    
    lcRetValue = m_cStrikeParamStore.insert(StrikeParamStore::value_type(szExpiryStr, lpcStrikeParamStore));
    if(lcRetValue.second == false)
    {
      std::cout << "Failed to added Symbol " << szExpiryStr << std::endl;
      delete lpcStrikeParamStore;
      return false;
    }
  }
  else
  {
    lpcStrikeParamStore = lcItr->second;
  }  

  for(int32_t lnLoop = 0; lnLoop <  MAX_PORTFOLIO_PER_TOKEN; ++lnLoop)
  {
    if(stIndividualSymbolDetails.nStrikePrice[lnLoop] != 0)
    {
      lpcStrikeParamStore->UpdateStrikePrice(stIndividualSymbolDetails.nStrikePrice[lnLoop]);    
    }
  }
  return true;
}

CStrikeParamStore* CExpiryParam::GetStrikeParamStore(std::string& szExpiryStr)
{
  CStrikeParamStore* lpcStrikeParamStore = NULL;
  StrikeParamStoreItr lcItr = m_cStrikeParamStore.find(szExpiryStr);
  if(lcItr != m_cStrikeParamStore.end())
  {
    lpcStrikeParamStore = lcItr->second;
  }
  
  return lpcStrikeParamStore;
}

/*********************************************************CExpiryParam******************************************************************/          

/*********************************************************CSymbolParam******************************************************************/          

CSymbolParam::CSymbolParam()
{

}

CSymbolParam::~CSymbolParam()
{
  for(auto lcItr = m_cSymbol2ExpiryParamStore.begin(); lcItr != m_cSymbol2ExpiryParamStore.end(); ++lcItr)
  {
    delete lcItr->second;
  }
  m_cSymbol2ExpiryParamStore.clear(); 
}


bool CSymbolParam::UpdateSymbolParam(const tagIndividualSymbolDetails& stIndividualSymbolDetails, std::string& szExpiryStr)
{
  std::string lszSymbol = stIndividualSymbolDetails.cSymbol;
  CExpiryParam* lpcExpiryParam = NULL;
  SymbolParamItr lcItr = m_cSymbol2ExpiryParamStore.find(lszSymbol);
  if(lcItr == m_cSymbol2ExpiryParamStore.end())
  {
    lpcExpiryParam = new CExpiryParam(lszSymbol);
    std::pair<SymbolParamItr, bool>lcRetValue;    
    lcRetValue = m_cSymbol2ExpiryParamStore.insert(SymbolParam::value_type(lszSymbol, lpcExpiryParam));
    if(lcRetValue.second == false)
    {
      std::cout << "Failed to added Symbol " << lszSymbol << std::endl;
      delete lpcExpiryParam;
      return false;
    }
    m_cSymbolListParam.push_back(stIndividualSymbolDetails.cSymbol);
  }
  else
  {
    lpcExpiryParam = lcItr->second;
  }  

  lpcExpiryParam->UpdateExpiryNStrikePrices(stIndividualSymbolDetails, szExpiryStr);  
  return true;
}

void CSymbolParam::PrintSymbolList()
{
  for(auto lcItr = m_cSymbolListParam.begin(); lcItr != m_cSymbolListParam.end(); ++lcItr)
  {
    //std::cout << *lcItr << std::endl;
  }
}

CExpiryParam* CSymbolParam::GetExpiryParam(const std::string& szSymbol)
{
  CExpiryParam* lpcExpiryParam = NULL;
  SymbolParamItr lcItr = m_cSymbol2ExpiryParamStore.find(szSymbol);
  if(lcItr != m_cSymbol2ExpiryParamStore.end())
  {
    lpcExpiryParam = lcItr->second;
  }
  return lpcExpiryParam;
}

/*********************************************************CSymbolParam******************************************************************/          

CBusinessStore::CBusinessStore()
{
  m_nMaxHolidayCount = 0;
}

CBusinessStore::~CBusinessStore()
{

}

bool CBusinessStore::StrategyType(int16_t nStrategyType)
{
  m_nStrategyType = nStrategyType;
  return true;
}

bool CBusinessStore::AddIgnoreSymbol(char* pcSymbolName)
{
  std::string lszSymbol = pcSymbolName;
  
  auto lcItr = m_cIgnoreSymbolStore.find(lszSymbol);
  if(lcItr == m_cIgnoreSymbolStore.end())
  {  
    std::pair<SymbolStoreItr, bool>lcRetValue;
    lcRetValue = m_cIgnoreSymbolStore.insert(lszSymbol);
    if(false == lcRetValue.second)
    {
      std::cout << "Failed to add Symbol " << lszSymbol << std::endl;
      return false;
    }
  }  
  return true;
}

bool CBusinessStore::AddOMSScripInfo(const OMSScripInfo& stOMSScripInfo)
{
  const int32_t lnExpiryDate = stOMSScripInfo.stSymbolDetails.nExpiryDate + 315532800;
  switch(m_nStrategyType)
  {
    case MONTHLY_EXPIRY:
    {
      if(!IsMonthlyExpiry(lnExpiryDate, m_nMaxHolidayCount))
      {
        return true;
      }
      
      auto lcItr = m_cSymbolStore.find(stOMSScripInfo.stSymbolDetails.cSymbol);
      if(lcItr == m_cSymbolStore.end())
      {  
        std::pair<SymbolStoreItr, bool>lcRetValue;
        lcRetValue = m_cSymbolStore.insert(stOMSScripInfo.stSymbolDetails.cSymbol);
        if(false == lcRetValue.second)
        {
          std::cout << "Failed to add Symbol " << stOMSScripInfo.stSymbolDetails.cSymbol << std::endl;
          return false;
        }
      }  
      
      ExpiryDateStoreItr  lcItr1 = m_cExpiryDateStore.find(stOMSScripInfo.stSymbolDetails.nExpiryDate);
      if(lcItr1 == m_cExpiryDateStore.end())
      {
        m_cExpiryDateStore.insert(stOMSScripInfo.stSymbolDetails.nExpiryDate);    
      }

      OMSScripInfo* lpstOMSScripInfo = new OMSScripInfo();
      memset(lpstOMSScripInfo, 0, sizeof(OMSScripInfo));

      memcpy(lpstOMSScripInfo, &stOMSScripInfo, sizeof(OMSScripInfo));

      std::pair<OMSScripInfoStoreITR, bool>lcRetValue;
      lcRetValue = m_cOMSScripInfoStore.insert(OMSScripInfoStore::value_type(lpstOMSScripInfo->nToken, lpstOMSScripInfo));
      if(lcRetValue.second == false)
      {
        std::cout << "Failed to added Token " << lpstOMSScripInfo->nToken << std::endl;
        delete lpstOMSScripInfo;
        return false;
      }
      return m_cSymbolMaster.AddSymbol(stOMSScripInfo);
    }
    break;
    
    case WEEKLY_EXPIRY:
    {
      if(IsMonthlyExpiry(lnExpiryDate, m_nMaxHolidayCount) && (stOMSScripInfo.stSymbolDetails.nStrikePrice != -1))
      {
        //std::cout<< "THROWN OUT:"<< stOMSScripInfo.cTradingSymbolName<< "|"<< stOMSScripInfo.stSymbolDetails.nStrikePrice<< std::endl;
        return true;
      }
      //if(strncmp(stOMSScripInfo.cTradingSymbolName, "BANKNIFTY", 9) == 0)
          //std::cout<< "Symbol:"<< stOMSScripInfo.cTradingSymbolName<< "|"<< stOMSScripInfo.stSymbolDetails.nStrikePrice<< std::endl;
  
      auto lcItr = m_cSymbolStore.find(stOMSScripInfo.stSymbolDetails.cSymbol);
      if(lcItr == m_cSymbolStore.end())
      {  
        std::pair<SymbolStoreItr, bool>lcRetValue;
        lcRetValue = m_cSymbolStore.insert(stOMSScripInfo.stSymbolDetails.cSymbol);
        if(false == lcRetValue.second)
        {
          std::cout << "Failed to add Symbol " << stOMSScripInfo.stSymbolDetails.cSymbol << std::endl;
          return false;
        }
      } 
      
      ExpiryDateStoreItr lcItr1 = m_cExpiryDateStore.find(stOMSScripInfo.stSymbolDetails.nExpiryDate);
      if(lcItr1 == m_cExpiryDateStore.end())
      {
        //if(strncmp(stOMSScripInfo.cTradingSymbolName, "BANKNIFTY", 9) == 0)
          //std::cout<< "EXPIRY:"<< stOMSScripInfo.stSymbolDetails.nExpiryDate<< "\t"<< stOMSScripInfo.cTradingSymbolName<< "|stOMSScripInfo:"<< stOMSScripInfo.nCustomInstrumentType<< std::endl;
        m_cExpiryDateStore.insert(stOMSScripInfo.stSymbolDetails.nExpiryDate);    
      }
      
//    if(strncmp(stOMSScripInfo.cTradingSymbolName, "BANKNIFTY", 9) == 0)
//    std::cout<< "EXP:"<< stOMSScripInfo.cTradingSymbolName<< "|"<< stOMSScripInfo.stSymbolDetails.nExpiryDate<< std::endl;
      OMSScripInfo* lnstOMSScripInfo = new OMSScripInfo();
      memset(lnstOMSScripInfo, 0, sizeof(OMSScripInfo));

      memcpy(lnstOMSScripInfo, &stOMSScripInfo, sizeof(OMSScripInfo));

      auto lcRetValue = m_cOMSScripInfoStore.insert(std::make_pair(stOMSScripInfo.nToken, lnstOMSScripInfo));
      if(lcRetValue.second == false)
      {
        std::cout << "Failed to added Token " << lnstOMSScripInfo->nToken << std::endl;
        delete lnstOMSScripInfo;
        return false;
      }
        
//      if(strncmp(stOMSScripInfo.stSymbolDetails.cSymbol, "BANKNIFTY", 10) == 0)
//      std::cout<< "ADD:"<< stOMSScripInfo.cTradingSymbolName<< std::endl;
      return m_cSymbolMaster.AddSymbol(stOMSScripInfo);
    }
    break;
  }
  return true;
}


bool CBusinessStore::IsValidSymbol(const std::string& szSymbol)
{
  auto lcItr = m_cSymbolStore.find(szSymbol);
  if(lcItr != m_cSymbolStore.end())
  {  
    return true;
  }  
  return false;
}

bool CBusinessStore::IsValidIgnoreSymbol(const std::string& szSymbol)
{
  auto lcItr = m_cIgnoreSymbolStore.find(szSymbol);
  if(lcItr != m_cIgnoreSymbolStore.end())
  {  
    return true;
  }  
  return false;
}


void CBusinessStore::UpdateHolidayCount(int16_t nMaxHolidayCount)
{
  m_nMaxHolidayCount = nMaxHolidayCount;
}

OMSScripInfo* CBusinessStore::GetOMSScripInfo(int32_t nTokenNumber)
{
  OMSScripInfo* lpstOMSScripInfo = NULL;
  
  OMSScripInfoStoreITR lcItr = m_cOMSScripInfoStore.find(nTokenNumber); 
  if(lcItr != m_cOMSScripInfoStore.end())
  {
    return lcItr->second;
  }  
  
  return lpstOMSScripInfo;
}

bool CBusinessStore::ValidateTokenList()
{
  return m_cSymbolMaster.ValidateTokenList();
}

void CBusinessStore::GetPortFolioList(std::vector<tagPortfolioDetails>& vPortfolioDetailsStore, const int16_t& nStratType)
{
  const char lcCurrentMonth             = 'C';
  const char lcNextMonth                = 'N';
  const char lcFarMonth                 = 'F';  
  
  CExpiryParam*         lpcExpiryParam          = NULL;
  CStrikeParamStore*    lpcStrikeParamStore     = NULL;  
  
  CExpiryDateStore*     lpcExpiryDateStore      = NULL;
  CSPTokenDetailStore*  lpcSPTokenDetailStore   = NULL;

  std::string lszExpiryMonthStr = CBusinessStore::GetExpiryMonth(lcCurrentMonth, GetLowestExpiryDate());            
  SymbolListParamItr lcItr = m_cSymbolParam.m_cSymbolListParam.begin();
  for(; lcItr != m_cSymbolParam.m_cSymbolListParam.end(); ++lcItr)
  {
    lpcExpiryParam = m_cSymbolParam.GetExpiryParam(*lcItr);
    //std::cout<< *lcItr<< std::endl;
    if(NULL != lpcExpiryParam)
    {
      lpcStrikeParamStore = lpcExpiryParam->GetStrikeParamStore(lszExpiryMonthStr);
    }
    
    if(lpcExpiryParam != NULL && lpcStrikeParamStore != NULL)
    {
      lpcExpiryDateStore = m_cSymbolMaster.GetExpiryDateStore(*lcItr);
      if(NULL != lpcExpiryDateStore)
      {        
        std::set<int32_t> WeeklkyExpSetPerMonth;
        Expiry2SPInfoStoreITR lcItr = lpcExpiryDateStore->m_cExpiry2SPInfoStore.begin();
        for(; lcItr != lpcExpiryDateStore->m_cExpiry2SPInfoStore.end(); ++lcItr)
        {
          if(lszExpiryMonthStr == lcItr->second->m_szExpiryMonth)
          {
            WeeklkyExpSetPerMonth.insert(lcItr->first);
          }
        }
                
        lcItr = lpcExpiryDateStore->m_cExpiry2SPInfoStore.begin();
        for(; lcItr != lpcExpiryDateStore->m_cExpiry2SPInfoStore.end(); ++lcItr)
        {
          lpcSPTokenDetailStore = lcItr->second;
          if(lszExpiryMonthStr == lcItr->second->m_szExpiryMonth)
          {
            if(lpcSPTokenDetailStore)
            {
              std::cout<< lcItr->second->m_nExpiryDate<< std::endl;
              if(IsMonthlyExpiry(lcItr->second->m_nExpiryDate + 315532800, m_nMaxHolidayCount))
              {
                std::cout<< "Monthly "<< lcItr->second->m_nExpiryDate<< std::endl;
                GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
              }
              else
              {
                std::cout<< "Cur - WeeklkyExpSetPerMonth.size():"<< WeeklkyExpSetPerMonth.size()<< "|"<< lpcStrikeParamStore->m_nWeeklyIdentifier.size()<< std::endl;
                if(WeeklkyExpSetPerMonth.size() < int((*lpcStrikeParamStore->m_nWeeklyIdentifier.rbegin())[1]) - 48)
                {
                  std::cout<< "Invalid Input Week : Current Month : "<< *lpcStrikeParamStore->m_nWeeklyIdentifier.rbegin()<< std::endl;
                  exit(0);
                }
                
                for(auto ITR : lpcStrikeParamStore->m_nWeeklyIdentifier) std::cout<< ITR << std::endl;
                if(lcItr->second->m_nExpiryDate == *std::next(WeeklkyExpSetPerMonth.begin(), 0))
                {
                  if(lpcStrikeParamStore->m_nWeeklyIdentifier.find("W1") != lpcStrikeParamStore->m_nWeeklyIdentifier.end())
                  GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
                }
                if(lcItr->second->m_nExpiryDate == *std::next(WeeklkyExpSetPerMonth.begin(), 1))
                {
                  if(lpcStrikeParamStore->m_nWeeklyIdentifier.find("W2") != lpcStrikeParamStore->m_nWeeklyIdentifier.end())
                  GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
                }
                if(lcItr->second->m_nExpiryDate == *std::next(WeeklkyExpSetPerMonth.begin(), 2))
                {
                  if(lpcStrikeParamStore->m_nWeeklyIdentifier.find("W3") != lpcStrikeParamStore->m_nWeeklyIdentifier.end())
                  GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
                }
                if(lcItr->second->m_nExpiryDate == *std::next(WeeklkyExpSetPerMonth.begin(), 3))
                {
                  if(lpcStrikeParamStore->m_nWeeklyIdentifier.find("W4") != lpcStrikeParamStore->m_nWeeklyIdentifier.end())
                  GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
                }
                //GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
              }
            }
          }
        }
      }
    }
  }
  
  lpcExpiryParam = NULL;
  lpcStrikeParamStore = NULL;
  lszExpiryMonthStr = CBusinessStore::GetExpiryMonth(lcNextMonth, GetLowestExpiryDate());              
  lcItr = m_cSymbolParam.m_cSymbolListParam.begin();
  for(; lcItr != m_cSymbolParam.m_cSymbolListParam.end(); ++lcItr)
  {
    lpcExpiryParam = m_cSymbolParam.GetExpiryParam(*lcItr);
    if(NULL != lpcExpiryParam)
    {
      lpcStrikeParamStore = lpcExpiryParam->GetStrikeParamStore(lszExpiryMonthStr);
    }
    
    if(lpcExpiryParam != NULL && lpcStrikeParamStore != NULL)
    {
      lpcExpiryDateStore = m_cSymbolMaster.GetExpiryDateStore(*lcItr);
      if(NULL != lpcExpiryDateStore)
      {
        //handling Weekly
        std::set<int32_t> WeeklkyExpSetPerMonth;
        Expiry2SPInfoStoreITR lcItr = lpcExpiryDateStore->m_cExpiry2SPInfoStore.begin();
        for(; lcItr != lpcExpiryDateStore->m_cExpiry2SPInfoStore.end(); ++lcItr)
        {
          if(lszExpiryMonthStr == lcItr->second->m_szExpiryMonth)
          {
            WeeklkyExpSetPerMonth.insert(lcItr->first);
          }
        }
        
        lcItr = lpcExpiryDateStore->m_cExpiry2SPInfoStore.begin();
        for(; lcItr != lpcExpiryDateStore->m_cExpiry2SPInfoStore.end(); ++lcItr)
        {
          lpcSPTokenDetailStore = lcItr->second;
          if(lszExpiryMonthStr == lcItr->second->m_szExpiryMonth)
          {
            if(lpcSPTokenDetailStore)
            {
              if(IsMonthlyExpiry(lcItr->second->m_nExpiryDate + 315532800, m_nMaxHolidayCount))
              {
                GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
              }
              else
              {
                std::cout<< "Next - WeeklkyExpSetPerMonth.size():"<< WeeklkyExpSetPerMonth.size()<< std::endl;
                //std::cout<< (*lpcStrikeParamStore->m_nWeeklyIdentifier<< std::endl;
                try
                {
                  std::cout<< "Try"<< std::endl;
                  if(!isdigit(int((*lpcStrikeParamStore->m_nWeeklyIdentifier.rbegin())[1])))
                  {
                    std::cout<< "Throw"<< std::endl;
                    throw ;
                  }
                  if(WeeklkyExpSetPerMonth.size() < int((*lpcStrikeParamStore->m_nWeeklyIdentifier.rbegin())[1]) - 48)
                  {
                    std::cout<< "Invalid Input Week : Next Month : "<< *lpcStrikeParamStore->m_nWeeklyIdentifier.rbegin()<< std::endl;
                    exit(0);
                  }
                }
                catch(...)
                {
                  std::cout<< "No Weeks Provided"<< std::endl;
                }
                if(lcItr->second->m_nExpiryDate == *std::next(WeeklkyExpSetPerMonth.begin(), 0))
                {
                  if(lpcStrikeParamStore->m_nWeeklyIdentifier.find("W1") != lpcStrikeParamStore->m_nWeeklyIdentifier.end())
                  GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
                }
                if(lcItr->second->m_nExpiryDate == *std::next(WeeklkyExpSetPerMonth.begin(), 1))
                {
                  if(lpcStrikeParamStore->m_nWeeklyIdentifier.find("W2") != lpcStrikeParamStore->m_nWeeklyIdentifier.end())
                  GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
                }
                if(lcItr->second->m_nExpiryDate == *std::next(WeeklkyExpSetPerMonth.begin(), 2))
                {
                  if(lpcStrikeParamStore->m_nWeeklyIdentifier.find("W3") != lpcStrikeParamStore->m_nWeeklyIdentifier.end())
                  GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
                }
                if(lcItr->second->m_nExpiryDate == *std::next(WeeklkyExpSetPerMonth.begin(), 3))
                {
                  if(lpcStrikeParamStore->m_nWeeklyIdentifier.find("W4") != lpcStrikeParamStore->m_nWeeklyIdentifier.end())
                  GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
                }
              }
            }
          }
        }
        /*lpcSPTokenDetailStore = lpcExpiryDateStore->GetSPTokenDetailStore(lszExpiryStr);
          if(lpcSPTokenDetailStore)
        {
          GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
        }*/
      }
    }
  }

  lpcExpiryParam = NULL;
  lpcStrikeParamStore = NULL;
  lszExpiryMonthStr = CBusinessStore::GetExpiryMonth(lcFarMonth, GetLowestExpiryDate()); 
  
  lcItr = m_cSymbolParam.m_cSymbolListParam.begin();
  for(; lcItr != m_cSymbolParam.m_cSymbolListParam.end(); ++lcItr)
  {
    lpcExpiryParam = m_cSymbolParam.GetExpiryParam(*lcItr);
    if(NULL != lpcExpiryParam)
    {
      lpcStrikeParamStore = lpcExpiryParam->GetStrikeParamStore(lszExpiryMonthStr);
    }
    
    if(lpcExpiryParam != NULL && lpcStrikeParamStore != NULL)
    {
      lpcExpiryDateStore = m_cSymbolMaster.GetExpiryDateStore(*lcItr);
      if(NULL != lpcExpiryDateStore)
      {
        //handling Weekly
        std::set<int32_t> WeeklkyExpSetPerMonth;
        Expiry2SPInfoStoreITR lcItr = lpcExpiryDateStore->m_cExpiry2SPInfoStore.begin();
        for(; lcItr != lpcExpiryDateStore->m_cExpiry2SPInfoStore.end(); ++lcItr)
        {
          if(lszExpiryMonthStr == lcItr->second->m_szExpiryMonth)
          {
            WeeklkyExpSetPerMonth.insert(lcItr->first);
          }
        }
        
        lcItr = lpcExpiryDateStore->m_cExpiry2SPInfoStore.begin();
        for(; lcItr != lpcExpiryDateStore->m_cExpiry2SPInfoStore.end(); ++lcItr)
        {
          lpcSPTokenDetailStore = lcItr->second;
          if(lszExpiryMonthStr == lcItr->second->m_szExpiryMonth)
          {
            if(lpcSPTokenDetailStore)
            {
              if(IsMonthlyExpiry(lcItr->second->m_nExpiryDate + 315532800, m_nMaxHolidayCount))
              {
                GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
              }
              else
              {
                std::cout<< "Far - WeeklkyExpSetPerMonth.size():"<< WeeklkyExpSetPerMonth.size()<< std::endl;
                if(WeeklkyExpSetPerMonth.size() < int((*lpcStrikeParamStore->m_nWeeklyIdentifier.rbegin())[1]) - 48)
                {
                  std::cout<< "Invalid Input Week : Far Month : "<< *lpcStrikeParamStore->m_nWeeklyIdentifier.rbegin()<< std::endl;
                  exit(0);
                }
                if(lcItr->second->m_nExpiryDate == *std::next(WeeklkyExpSetPerMonth.begin(), 0))
                {
                  if(lpcStrikeParamStore->m_nWeeklyIdentifier.find("W1") != lpcStrikeParamStore->m_nWeeklyIdentifier.end())
                  GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
                }
                if(lcItr->second->m_nExpiryDate == *std::next(WeeklkyExpSetPerMonth.begin(), 1))
                {
                  if(lpcStrikeParamStore->m_nWeeklyIdentifier.find("W2") != lpcStrikeParamStore->m_nWeeklyIdentifier.end())
                  GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
                }
                if(lcItr->second->m_nExpiryDate == *std::next(WeeklkyExpSetPerMonth.begin(), 2))
                {
                  if(lpcStrikeParamStore->m_nWeeklyIdentifier.find("W3") != lpcStrikeParamStore->m_nWeeklyIdentifier.end())
                  GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
                }
                if(lcItr->second->m_nExpiryDate == *std::next(WeeklkyExpSetPerMonth.begin(), 3))
                {
                  if(lpcStrikeParamStore->m_nWeeklyIdentifier.find("W4") != lpcStrikeParamStore->m_nWeeklyIdentifier.end())
                  GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
                }
              }
            }
          }
        }
        /*lpcSPTokenDetailStore = lpcExpiryDateStore->GetSPTokenDetailStore(lszExpiryStr);
        if(lpcSPTokenDetailStore)
        {        
          GetPortFolioList1(vPortfolioDetailsStore, lpcExpiryDateStore, lpcSPTokenDetailStore, lpcStrikeParamStore);
        }*/
      }
    }
  }  
}

void CBusinessStore::GetPortFolioList1(std::vector<tagPortfolioDetails>& vPortfolioDetailsStore, CExpiryDateStore* pcExpiryDateStore, CSPTokenDetailStore* pcSPTokenDetailStore, CStrikeParamStore* pcStrikeParamStore) 
{
  std::cout<< pcStrikeParamStore->m_cStrikePriceList.front()<< std::endl;
  int32_t lnFirstStrikePrice = pcStrikeParamStore->m_cStrikePriceList.front();
  if(-1 == lnFirstStrikePrice)
  {
    SPTokenStoreITR lcItr2 = pcSPTokenDetailStore->m_cSPTokenStore.begin();
    for(; lcItr2 != pcSPTokenDetailStore->m_cSPTokenStore.end(); ++lcItr2)
    {
      CSPTokenDetails* lpcSPTokenDetails = lcItr2->second;
      if(NULL != lpcSPTokenDetails)
      {
        GetPortFolioInVector(vPortfolioDetailsStore, pcExpiryDateStore, pcSPTokenDetailStore, lpcSPTokenDetails);
      }
    }    
  }
  else
  {
    StrikePriceListItr lcItr = pcStrikeParamStore->m_cStrikePriceList.begin();
    for(;lcItr != pcStrikeParamStore->m_cStrikePriceList.end(); ++lcItr)
    {
      CSPTokenDetails* lpcSPTokenDetails = pcSPTokenDetailStore->GetSPTokenDetails(*lcItr);
      if(lpcSPTokenDetails == NULL )
        continue;

      GetPortFolioInVector(vPortfolioDetailsStore, pcExpiryDateStore, pcSPTokenDetailStore, lpcSPTokenDetails);
    }
  }
  return;
}


void CBusinessStore::GetPortFolioInVector(std::vector<tagPortfolioDetails>& vPortfolioDetailsStore, CExpiryDateStore* pcExpiryDateStore, CSPTokenDetailStore* pcSPTokenDetailStore, CSPTokenDetails* pcSPTokenDetails)
{
  tagPortfolioDetails lstPortfolioDetails;
  memset(&lstPortfolioDetails, 0, sizeof(tagPortfolioDetails));
  lstPortfolioDetails.m_nToken1       = pcSPTokenDetails->m_nToken1;
  lstPortfolioDetails.m_nToken2       = pcSPTokenDetails->m_nToken2;
  lstPortfolioDetails.m_nToken3       = pcSPTokenDetails->m_nToken3;    
  lstPortfolioDetails.m_nStrikePrice  = pcSPTokenDetails->m_nStrikePrice;
  lstPortfolioDetails.m_nExpiryDate   = pcSPTokenDetailStore->m_nExpiryDate;
  strncpy(lstPortfolioDetails.cSymbol, pcExpiryDateStore->m_szSymbol.c_str(), SCRIP_SYMBOL_LEN);
  vPortfolioDetailsStore.push_back(lstPortfolioDetails);
}

bool CBusinessStore::UpdateExpiryNStrikePrice(const tagIndividualSymbolDetails& stIndividualSymbolDetails)
{
  std::string lszExpiryStr = CBusinessStore::GetExpiryMonth(stIndividualSymbolDetails.cExpiryDate, GetLowestExpiryDate()); 
  return m_cSymbolParam.UpdateSymbolParam(stIndividualSymbolDetails, lszExpiryStr);
}

bool CBusinessStore::PrintSymbolList()
{ 
  m_cSymbolParam.PrintSymbolList();
  return true;
}

bool CBusinessStore::ValidateSymbolParam(tagIndividualSymbolDetails& lstIndividualSymbolDetails, SymbolStore& cSymbolStoreTemp, StringSplit& cStringSplit, char* pcStrikePriceList1, char* pcStrikePriceList2, char* pcStrikePriceList3)
{
  std::vector<std::string>lszSplitStrings;
  char* lpcLineData = NULL;
  memset(&lstIndividualSymbolDetails.nStrikePrice, 0, sizeof(int32_t)*MAX_PORTFOLIO_PER_TOKEN);        
  lszSplitStrings.clear();    
  std::cout<< "pcStrikePriceList1[0] "<< pcStrikePriceList1[0]<< "\npcStrikePriceList2[0] "<< pcStrikePriceList2[0]<< "\npcStrikePriceList3[0] "
    << pcStrikePriceList3[0]<< std::endl;
  if((pcStrikePriceList1[0] != 'C' && pcStrikePriceList1[0] != '\0') && (pcStrikePriceList2[0] != 'N' && pcStrikePriceList2[0] != '\0')
      && (pcStrikePriceList3[0] != 'F' && pcStrikePriceList2[0] != '\0'))
  {
    std::cout << "Invalid Expiry month identifier " << pcStrikePriceList1[0] << " " << pcStrikePriceList2[0] << " " << pcStrikePriceList3[0] << std::endl;
    return false;   
  }
  
  const std::string lszSymbol = lstIndividualSymbolDetails.cSymbol;
  if(false == IsValidSymbol(lszSymbol))
  {
    std::cout << "Invalid Symbol " << lszSymbol << std::endl;
    return false;     
  }

  auto lcItr = cSymbolStoreTemp.find(lszSymbol);
  if(lcItr != cSymbolStoreTemp.end())
  {
    std::cout << "Duplicate Symbol in input files : " << lszSymbol << std::endl;
    return false;       
  }
  cSymbolStoreTemp.insert(lszSymbol);  
  
  if(pcStrikePriceList1[0] == 'C')
  {
    lstIndividualSymbolDetails.cExpiryDate = 'C';
    if(pcStrikePriceList1[1] == '-')
    {
      lpcLineData = pcStrikePriceList1; ++lpcLineData;++lpcLineData;
      cStringSplit.GetData(lpcLineData, lszSplitStrings);

      int32_t lnCount = 0;
      if(!lszSplitStrings.empty()) lnCount = lszSplitStrings.size();
      if(lnCount == 0) 
      {
        std::cout << "Strike Price list is empty" << std::endl;
        return false;         
      }      
      
      if(lnCount > MAX_PORTFOLIO_PER_TOKEN)
      {
        std::cout << "Portfolio per Token is more then " << MAX_PORTFOLIO_PER_TOKEN << std::endl;
        return false;   
      }        
    }
    else if(pcStrikePriceList1[1] != '\0')
    {
      std::cout << "Invalid format in next month filter" << std::endl;
      return false;               
    }        
  }
  else if(pcStrikePriceList1[0] != '\0')    
  {
    std::cout << "Invalid Month identifier " << pcStrikePriceList1[0] << std::endl;
    return false;                 
  }

  memset(&lstIndividualSymbolDetails.nStrikePrice, 0, sizeof(int32_t)*MAX_PORTFOLIO_PER_TOKEN);
  lszSplitStrings.clear();
  if(pcStrikePriceList2[0] == 'N')
  {    
    lstIndividualSymbolDetails.cExpiryDate = 'N';      
    if(pcStrikePriceList2[1] == '-')
    {
      lpcLineData = pcStrikePriceList2; ++lpcLineData;++lpcLineData;      
      cStringSplit.GetData(lpcLineData, lszSplitStrings);

      int32_t lnCount = 0;
      if(!lszSplitStrings.empty()) lnCount = lszSplitStrings.size();
      if(lnCount == 0) 
      {
        std::cout << "Strike Price list is empty" << std::endl;
        return false;         
      }      
      
      if(lnCount > MAX_PORTFOLIO_PER_TOKEN)
      {
        std::cout << "Portfolio per Token is more then " << MAX_PORTFOLIO_PER_TOKEN << std::endl;
        return false;   
      }        
    }
    else if(pcStrikePriceList2[1] != '\0')
    {
      std::cout << "Invalid format in next month filter" << std::endl;
      return false;                     
    }    
  }
  else if(pcStrikePriceList2[0] != '\0')    
  {
    std::cout << "Invalid Month identifier " << pcStrikePriceList2[0] << std::endl;
    return false;                 
  }  

  memset(&lstIndividualSymbolDetails.nStrikePrice, 0, sizeof(int32_t)*MAX_PORTFOLIO_PER_TOKEN);
  lszSplitStrings.clear();    
  if(pcStrikePriceList3[0] == 'F')
  {        
    lstIndividualSymbolDetails.cExpiryDate = 'F';            
    if(pcStrikePriceList3[1] == '-')
    {
      lpcLineData = pcStrikePriceList3; ++lpcLineData;++lpcLineData;            
      cStringSplit.GetData(lpcLineData, lszSplitStrings);

      int32_t lnCount = 0;
      if(!lszSplitStrings.empty()) lnCount = lszSplitStrings.size();
      
      if(lnCount == 0) 
      {
        std::cout << "Strike Price list is empty" << std::endl;
        return false;         
      }
      
      if(lnCount > MAX_PORTFOLIO_PER_TOKEN)
      {
        std::cout << "Portfolio per Token is more then " << MAX_PORTFOLIO_PER_TOKEN << std::endl;
        return false;   
      }        
    }    
    else if(pcStrikePriceList3[1] != '\0')
    {
      std::cout << "Invalid format in far month filter" << std::endl;
      return false;                     
    }
  }
  else if(pcStrikePriceList3[0] != '\0')    
  {
    std::cout << "Invalid Month identifier " << pcStrikePriceList3[0] << std::endl;
    return false;                 
  }
  return true;
}

bool CBusinessStore::PrintMapExp()
{
  return true;
  for(auto ITR = m_cSymbolMaster.m_cSymbol2ExpiryStore.begin(); ITR != m_cSymbolMaster.m_cSymbol2ExpiryStore.end(); ++ITR)
  {
    std::cout<< ITR->second->m_szSymbol<< std::endl;
    for(auto ITER = ITR->second->m_cExpiry2SPInfoStore.begin(); ITER != ITR->second->m_cExpiry2SPInfoStore.end(); ++ITER)
    {
      std::cout<< '\t'<< ITER->second->m_nExpiryDate<< "|"<< ITER->second->m_szExpiryMonth<< "|"<< ITER->second->m_cSPTokenStore.size()<< std::endl;
    }
  }
  return true;
}

time_t CBusinessStore::GetLowestExpiryDate()
{
  if(!m_cExpiryDateStore.empty())
  {
    return
      +*m_cExpiryDateStore.begin();
  }
  time_t lnCurrentTime = time(NULL);    
  
  return lnCurrentTime;
}

bool CBusinessStore::WeeklyExpiryStore(const std::string szString, tagIndividualSymbolDetails& stIndividualSymbolDetails, char chSeparator)
{
  size_t startPoint = 0, endPoint = 0;
  std::string lszWeek;
  while(endPoint != std::string::npos)
  {
    endPoint = szString.find(chSeparator, startPoint);
    lszWeek = szString.substr(startPoint, endPoint - startPoint);
    stIndividualSymbolDetails.bWeeklyExpirySet.insert(lszWeek);    
    startPoint = endPoint + 1;
  }
}

bool CBusinessStore::BuildSymbolParam(tagIndividualSymbolDetails& lstIndividualSymbolDetails, StringSplit& cStringSplit, char* pcStrikePriceList1, char* pcStrikePriceList2, char* pcStrikePriceList3)
{
  std::vector<std::string> lszSplitStrings;
  char* lpcLineData = NULL;
  //memset(&lstIndividualSymbolDetails.nStrikePrice, 0, sizeof(int32_t)*MAX_PORTFOLIO_PER_TOKEN); 
  lstIndividualSymbolDetails.clear();
  lszSplitStrings.clear();
  
  if(pcStrikePriceList1[0] == 'C')
  {
    lstIndividualSymbolDetails.cExpiryDate = 'C';
    if(pcStrikePriceList1[1] == '-')
    {
      lpcLineData = pcStrikePriceList1; ++lpcLineData;++lpcLineData;
      cStringSplit.GetData(lpcLineData, lszSplitStrings);
      
      auto lcItr = lszSplitStrings.begin();
      if(lszSplitStrings.begin()[0][0] == 'W')
      {
        // Check for Weekly Specification
        char chSeparator = '|';
        WeeklyExpiryStore(lszSplitStrings.begin()[0], lstIndividualSymbolDetails, chSeparator);
        lcItr++;
      }
      
      int32_t lnCount = 0;
      for(auto lcItr = lszSplitStrings.begin(); lcItr != lszSplitStrings.end(); ++lcItr)
      {
        lstIndividualSymbolDetails.nStrikePrice[lnCount++] = atoi(lcItr->c_str())*100;
      }
      if(lszSplitStrings.begin()[0][0] == 'W' && lszSplitStrings.size() == 1) //Weekly With No Filter Tokens;
      { 
        lstIndividualSymbolDetails.nStrikePrice[0] = -1;
      }
      
    }
    else
    {
      lstIndividualSymbolDetails.nStrikePrice[0] = -1;        
    }
    UpdateExpiryNStrikePrice(lstIndividualSymbolDetails);      
  }

  //memset(&lstIndividualSymbolDetails.nStrikePrice, 0, sizeof(int32_t)*MAX_PORTFOLIO_PER_TOKEN);
  lstIndividualSymbolDetails.clear();
  lszSplitStrings.clear(); 
  
  if(pcStrikePriceList2[0] == 'N')
  {    
    lstIndividualSymbolDetails.cExpiryDate = 'N';      
    if(pcStrikePriceList2[1] == '-')
    {
      lpcLineData = pcStrikePriceList2; ++lpcLineData;++lpcLineData;      
      cStringSplit.GetData(lpcLineData, lszSplitStrings);
      
      auto lcItr = lszSplitStrings.begin();
      if(lszSplitStrings.begin()[0][0] == 'W')
      {
        // Check for Weekly Specification
        char chSeparator = '|';
        WeeklyExpiryStore(lszSplitStrings.begin()[0], lstIndividualSymbolDetails, chSeparator);
        lcItr++;
      }
      
      int32_t lnCount = 0;        
      for(; lcItr != lszSplitStrings.end(); ++lcItr)
      {
        lstIndividualSymbolDetails.nStrikePrice[lnCount++] = atoi(lcItr->c_str())*100;
      }
      if(lszSplitStrings.begin()[0][0] == 'W' && lszSplitStrings.size() == 1) //Weekly With No Filter Tokens;
      { 
        lstIndividualSymbolDetails.nStrikePrice[0] = -1;
      }
    }
    else
    {
      lstIndividualSymbolDetails.nStrikePrice[0] = -1;        
    }      
    UpdateExpiryNStrikePrice(lstIndividualSymbolDetails);            
  }

  //memset(&lstIndividualSymbolDetails.nStrikePrice, 0, sizeof(int32_t)*MAX_PORTFOLIO_PER_TOKEN);
  lstIndividualSymbolDetails.clear();
  lszSplitStrings.clear();
  
  if(pcStrikePriceList3[0] == 'F')
  {        
    lstIndividualSymbolDetails.cExpiryDate = 'F';            
    if(pcStrikePriceList3[1] == '-')
    {
      lpcLineData = pcStrikePriceList3; ++lpcLineData;++lpcLineData;            
      cStringSplit.GetData(lpcLineData, lszSplitStrings);

      auto lcItr = lszSplitStrings.begin();
      if(lszSplitStrings.begin()[0][0] == 'W')
      {
        // Check for Weekly Specification
        char chSeparator = '|';
        WeeklyExpiryStore(lszSplitStrings.begin()[0], lstIndividualSymbolDetails, chSeparator);
        lcItr++;
      }
      
      int32_t lnCount = 0;        
      for(auto lcItr = lszSplitStrings.begin(); lcItr != lszSplitStrings.end(); ++lcItr)
      {
        lstIndividualSymbolDetails.nStrikePrice[lnCount++] = atoi(lcItr->c_str())*100;
      } 
      if(lszSplitStrings.begin()[0][0] == 'W' && lszSplitStrings.size() == 1) //Weekly With No Filter Tokens;
      { 
        lstIndividualSymbolDetails.nStrikePrice[0] = -1;
      }
    }    
    else
    {
      lstIndividualSymbolDetails.nStrikePrice[0] = -1;        
    }
    UpdateExpiryNStrikePrice(lstIndividualSymbolDetails);      
  }
  return true;
}

std::string CBusinessStore::GetMonthInStr(int32_t nExpiryDate)
{
  time_t lnTimeVal = 0;  
  tm lstTimeStruct;              
  memset(&lstTimeStruct, 0, sizeof(tm));            
  //lnTimeVal = nExpiryDate + 315532800;            
  lnTimeVal = nExpiryDate;            
  localtime_r(&lnTimeVal, &lstTimeStruct);    
  return MONTH[lstTimeStruct.tm_mon].c_str();
}

int CBusinessStore::GetMonthInNumber(int32_t nExpiryDate) 
{
  time_t lnTimeVal = 0;  
  tm lstTimeStruct;              
  memset(&lstTimeStruct, 0, sizeof(tm));            
  //lnTimeVal = nExpiryDate + 315532800;            
  lnTimeVal = nExpiryDate;
  localtime_r(&lnTimeVal, &lstTimeStruct);    
  return lstTimeStruct.tm_mon;
}

int CBusinessStore::GetMonthInterval(int32_t nDate, int16_t nMonthInterval)
{
  time_t lnTimeVal = 0;  
  tm lstTimeStruct;              
  memset(&lstTimeStruct, 0, sizeof(tm));            
  lnTimeVal = nDate;            
  localtime_r(&lnTimeVal, &lstTimeStruct);    
  
  int lnNextMonth = lstTimeStruct.tm_mon;
  lnNextMonth += nMonthInterval;
  return lnNextMonth%12;
}

std::string CBusinessStore::GetMonthInStrInterval(int32_t nDate, int16_t nMonthInterval)
{
  time_t lnTimeVal = 0;  
  tm lstTimeStruct;              
  memset(&lstTimeStruct, 0, sizeof(tm));            
  lnTimeVal = nDate;            
  localtime_r(&lnTimeVal, &lstTimeStruct);    
  
  int lnNextMonth = lstTimeStruct.tm_mon;
  lnNextMonth += nMonthInterval;
  return MONTH[lnNextMonth%12].c_str();
}

bool CBusinessStore::IsMonthlyExpiry(int32_t nExpiryDate, int16_t nMaxHoliday)
{
  time_t lnTimeVal = 0;  
  tm lstTimeStruct;              
  memset(&lstTimeStruct, 0, sizeof(tm));            
  lnTimeVal = nExpiryDate;            
  localtime_r(&lnTimeVal, &lstTimeStruct);    
  
  time_t lnTimeVal1 = nExpiryDate + 86400*7;  
  tm lstTimeStruct1;              
  memset(&lstTimeStruct1, 0, sizeof(tm));            
  localtime_r(&lnTimeVal1, &lstTimeStruct1);      
  
  if(lstTimeStruct.tm_wday == 4 && lstTimeStruct.tm_mon != lstTimeStruct1.tm_mon)
  {
    return true;
  }
  else
  {
    time_t lnTimeVal = 0;  
    tm lstTimeStruct;              
    memset(&lstTimeStruct, 0, sizeof(tm));            
    for(int16_t lnLoop = 0; lnLoop < nMaxHoliday; ++lnLoop)
    {
      lnTimeVal = nExpiryDate + 86400*lnLoop;            
      localtime_r(&lnTimeVal, &lstTimeStruct);   
      
      if(lstTimeStruct.tm_wday == 4 && lstTimeStruct.tm_mon != lstTimeStruct1.tm_mon)
      {
        return true;
      }
    }
  }
  return false;
}

std::string CBusinessStore::GetExpiryMonth(const char cExpiryIdentifer, const time_t& nExpiryDate)
{
  std::string lszExpiryStr = "";
  time_t lnCurrentTime = nExpiryDate;
  switch(cExpiryIdentifer)
  {
    case 'C':
    {
      lszExpiryStr = CBusinessStore::GetMonthInStrInterval(lnCurrentTime, 0);
    }
    break;
    
    case 'N':
    {
      lszExpiryStr = CBusinessStore::GetMonthInStrInterval(lnCurrentTime, 1);
    }
    break;
    
    case 'F':
    {
      lszExpiryStr = CBusinessStore::GetMonthInStrInterval(lnCurrentTime, 2);
    }
    break;
  }
  
  return lszExpiryStr;
}
