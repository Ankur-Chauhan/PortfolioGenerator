/* 
 * File:   TrianglePF.cpp
 * Author: AnkurChauhan
 *
 * Created on November 9, 2017, 11:04 AM
 */


//#include "Triangle_Class.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string.h>
#include <string>

#include <algorithm>
#include <functional>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <set>
#include <fstream>

#include "struct_char.h"
#include "nsefocontract_reader.h"
#include "ConfigReader.h"
#include "businessStore.h"


//#define FILTER_FILENAME "TriangleSymbol.ini"
//#define CONFIG_FILENAME "config.ini"

const int32_t NSEFO_CONTRACT_FIELD_COUNT = 68;

int main(int argc, char** argv)
{
  char lcMsgString[512 + 1] = {0};
  struct stat sd;
  if (-1  == stat(argv[1],  &sd))
  {
    snprintf(lcMsgString, sizeof(lcMsgString), "Invalid Configuration file path = %s", argv[1]);
    return EXIT_SUCCESS;
  }

  ConfigReader lcConfigReader(argv[1]);
  const std::string lszContractFilePath = lcConfigReader.getProperty("CONTRACT_FILE_PATH");
  const std::string lszFilterSymbolPath = lcConfigReader.getProperty("FILTER_SYMBOL_PATH");  
  const std::string lszIgnoreSymbolPath = lcConfigReader.getProperty("IGNORE_SYMBOL_PATH");      
  
  if (-1  == stat(lszContractFilePath.c_str(),  &sd))
  {
    snprintf(lcMsgString, sizeof(lcMsgString), "Invalid contract file path: %s", lszContractFilePath.c_str());
    return EXIT_SUCCESS;
  }

  if (-1  == stat(lszFilterSymbolPath.c_str(),  &sd))
  {
    snprintf(lcMsgString, sizeof(lcMsgString), "Invalid Filter Symbol Path: %s", lszFilterSymbolPath.c_str());
    return EXIT_SUCCESS;
  }

  if (-1  == stat(lszIgnoreSymbolPath.c_str(),  &sd))
  {
    snprintf(lcMsgString, sizeof(lcMsgString), "Invalid Ignore symbol path: %s", lszIgnoreSymbolPath.c_str());
    return EXIT_SUCCESS;
  }  

  const int16_t lnInstanceId            = atoi(lcConfigReader.getProperty("INSTANCE_ID").c_str());
  const int16_t lnPortfolioStrategy     = atoi(lcConfigReader.getProperty("STRATEGY_TYPE").c_str());  
  const int16_t lnPricePriceRange       = atoi(lcConfigReader.getProperty("PRICE_RANGE").c_str());  
  const int64_t lnFutGrosssExpore       = static_cast<int64_t>(atoll(lcConfigReader.getProperty("FUT_GROSS_EXPOSURE").c_str()));    
  const int64_t lnOptGrosssExpore       = static_cast<int64_t>(atoll(lcConfigReader.getProperty("OPT_GROSS_EXPOSURE").c_str()));
  const int16_t lnSingleOrderLot        = atoi(lcConfigReader.getProperty("SINGLE_ORDER_LOT").c_str());
  const int16_t lnMaxPositionLot        = atoi(lcConfigReader.getProperty("MAX_POSITION_LOT").c_str());  
  const int16_t lnMaxTokenLimit         = atoi(lcConfigReader.getProperty("MAX_TOKEN_LIMIT").c_str());    
  const int16_t lnMaxHolidayCount       = atoi(lcConfigReader.getProperty("MAX_HOLIDAY_COUNT").c_str());      
  const int16_t lnPerFuturePFCount      = atoi(lcConfigReader.getProperty("PER_FUTURE_TOKENCOUNT").c_str());
  
  CBusinessStore  lcBusinessStore;
  lcBusinessStore.StrategyType(lnPortfolioStrategy);
  lcBusinessStore.UpdateHolidayCount(lnMaxHolidayCount);
  
  char lchSeparator = '\n';
  CIgnoreSymbolReader lcIgnoreSymbolReader(lszIgnoreSymbolPath.c_str(), lchSeparator);
	if(!lcIgnoreSymbolReader.Open(0))
	{
		return EXIT_SUCCESS;			
	}
  
  char lcSymbol[SCRIP_SYMBOL_LEN + 1] = {0};
	while(lcIgnoreSymbolReader.GetNextRow(lcSymbol))
	{    
    if(!lcBusinessStore.AddIgnoreSymbol(lcSymbol))
    {
      std::cout << "Duplicate Ignore symbol : " << lcSymbol << std::endl;
      return EXIT_SUCCESS;			      
    }
    memset(lcSymbol, 0, SCRIP_SYMBOL_LEN + 1);    
  }  
  
  lchSeparator = '|';
  CNSEFOContractFileReader lcNSEFOContractFileReader(lszContractFilePath.c_str(), lchSeparator);
	if(!lcNSEFOContractFileReader.Open(1))
	{
		return EXIT_SUCCESS;			
	}
      
  OMSScripInfo lstOMSScripInfo;
  memset(&lstOMSScripInfo, 0, sizeof(OMSScripInfo));
	while(lcNSEFOContractFileReader.GetNextRow(lstOMSScripInfo))
	{
    if('N' == lstOMSScripInfo.cDeleteFlag && !lcBusinessStore.IsValidIgnoreSymbol(lstOMSScripInfo.stSymbolDetails.cSymbol))
    {
      if(!lcBusinessStore.AddOMSScripInfo(lstOMSScripInfo))
      {
        std::cout << "Failed to add contract details for " <<lstOMSScripInfo.stSymbolDetails.cInstrumentName << "|"
                                                           <<lstOMSScripInfo.stSymbolDetails.cSymbol         << "|"
                                                           <<lstOMSScripInfo.stSymbolDetails.nExpiryDate     << "|" 
                                                           <<lstOMSScripInfo.stSymbolDetails.nStrikePrice    << "|" 
                                                           <<lstOMSScripInfo.stSymbolDetails.cOptionType     << "|" 
                                                           <<lstOMSScripInfo.nToken                          << std::endl;
        return EXIT_SUCCESS;
      }
    }
    memset(&lstOMSScripInfo, 0, sizeof(OMSScripInfo));
  }
  
  //lcBusinessStore.PrintWeekly();
  lchSeparator = ',';
  CSymbolFilterReader lcSymbolFilterReader(lszFilterSymbolPath.c_str(), lchSeparator);
	if(!lcSymbolFilterReader.Open(0))
	{
		return EXIT_SUCCESS;
	}
  
  //tagSymbolRangeDetails lstSymbolRangeDetails;
  //memset(&lstSymbolRangeDetails, 0, sizeof(tagSymbolRangeDetails));
  
  tagIndividualSymbolDetails lstIndividualSymbolDetails;
  memset(&lstIndividualSymbolDetails, 0, sizeof(tagIndividualSymbolDetails));
  
  char lcStrikePriceList1[STRIKE_PRICE_LIST + 1] = {0};
  char lcStrikePriceList2[STRIKE_PRICE_LIST + 1] = {0};
  char lcStrikePriceList3[STRIKE_PRICE_LIST + 1] = {0};
  
  std::vector<uint16_t> lszInputCoumunNo;
  for(int16_t lnStrikePrice = 0;  lnStrikePrice < MAX_PORTFOLIO_PER_TOKEN; ++lnStrikePrice)
  {
    lszInputCoumunNo.push_back(lnStrikePrice);
  }
  
  SymbolStore lcSymbolStoreTemp;  
  char lchSPSeparator1 = '/';  
  char lchSPSeparator2 = '-';  
  StringSplit lcStringSplit(lchSPSeparator1, lchSPSeparator2, lszInputCoumunNo);
	while(lcSymbolFilterReader.GetNextRow(lstIndividualSymbolDetails,  lcStrikePriceList1, lcStrikePriceList2, lcStrikePriceList3))
	{ 
    if(lcBusinessStore.ValidateSymbolParam(lstIndividualSymbolDetails, lcSymbolStoreTemp, lcStringSplit, lcStrikePriceList1, lcStrikePriceList2, lcStrikePriceList3))
    {
      lcBusinessStore.BuildSymbolParam(lstIndividualSymbolDetails, lcStringSplit, lcStrikePriceList1, lcStrikePriceList2, lcStrikePriceList3);
    }
    else
    {
      std::cout << "Invalid Input Symbol data for : " << lstIndividualSymbolDetails.cSymbol << std::endl;
      return EXIT_SUCCESS;
    }
  }
  
  std::vector<tagPortfolioDetails>  lvPortfolioDetailsStore;
  int32_t lnConSpread = -1;
  int32_t lnRevSpread = 2;
  if(lcBusinessStore.ValidateTokenList())
  {
    lcBusinessStore.GetPortFolioList(lvPortfolioDetailsStore, lnPortfolioStrategy);
  }
  
  typedef std::set<int32_t>TokenSet;  
  //typedef TokenSet::iterator TokenSetItr;
  
  TokenSet lcTokenStore;
  OMSScripInfo* lpcOMSScripInfo = NULL;
  for(auto& lstPortfolioDetails :lvPortfolioDetailsStore)
  {
    lcTokenStore.insert(lstPortfolioDetails.m_nToken1);
    lcTokenStore.insert(lstPortfolioDetails.m_nToken2);
    lcTokenStore.insert(lstPortfolioDetails.m_nToken3);
    lpcOMSScripInfo = lcBusinessStore.GetOMSScripInfo(lstPortfolioDetails.m_nToken1);
    if(NULL != lpcOMSScripInfo && (0 == strncmp(lpcOMSScripInfo->stSymbolDetails.cInstrumentName, "FUTSTK", SCRIP_INSTRUMENT_NAME_LEN)))
    {
      lcTokenStore.insert(lpcOMSScripInfo->nAssetToken);    
    }
  }
  
  lcBusinessStore.PrintMapExp();
  int32_t lnTokenCount = 0;   
  if(!lcTokenStore.empty()) lnTokenCount = lcTokenStore.size();
  
  if(lnTokenCount > lnMaxTokenLimit)
  {
    std::cout<< "TokenCount " << lnTokenCount << " is more than " << lnMaxTokenLimit << std::endl;;
    return EXIT_FAILURE;
  }  
  
  std::string lnPortfoliofileName = "INSTANCE_" + std::to_string(lnInstanceId) + "_TokenPortfolio.csv";
  std::fstream lcPortfoliofileOpen(lnPortfoliofileName, std::ios::out);
  if(!lcPortfoliofileOpen)
  {
    std::cout<< "Unable To Create Portfolio File " << lnPortfoliofileName << std::endl;
    return EXIT_FAILURE;
  }
  
  int32_t lnPortfolioCount = 0, lnPreviousFutureToken = 0, lnPerFutPortfolioCount = 0;
  char lcPortfolioName[MAX_PORTFOLIO_NAME_LEN + 1] = {0};
  memset(lcPortfolioName, 0, sizeof(lcPortfolioName));
  lcPortfoliofileOpen<<"Instance,StratType,Token1,Token2,Token3,SingleOrderLot,MaxPositionLot,Spread,Reserved1,Reserved2,Reserved3,Reserved4,Reserved5,Reserved6,Reserved7,Reserved8"<< std::endl;
  for(auto& lstPortfolioDetails :lvPortfolioDetailsStore)
  {
    memset(lcPortfolioName, 0, sizeof(lcPortfolioName));    

    int32_t lnExpiryDate = lstPortfolioDetails.m_nExpiryDate + 315532800;                 
    time_t lnTimeVal = lnExpiryDate;  
    tm lstTimeStruct;              
    memset(&lstTimeStruct, 0, sizeof(tm));            
    localtime_r(&lnTimeVal, &lstTimeStruct);    
    
    if(lnPortfolioStrategy == 1)
    {
      snprintf(lcPortfolioName, sizeof(lcPortfolioName), "%s%hd%s-%d(C)", lstPortfolioDetails.cSymbol, (lstTimeStruct.tm_year + 1900)%100, MONTH[lstTimeStruct.tm_mon].c_str(), lstPortfolioDetails.m_nStrikePrice/100);
      lstPortfolioDetails.m_StrategyType = 1;
    }
    
    if(lnPortfolioStrategy == 2)
    {
      snprintf(lcPortfolioName, sizeof(lcPortfolioName), "%s%hd%s%d-%d(C)", lstPortfolioDetails.cSymbol, (lstTimeStruct.tm_year + 1900)%100, MONTH[lstTimeStruct.tm_mon].c_str(), lstTimeStruct.tm_mday, lstPortfolioDetails.m_nStrikePrice/100);
      lstPortfolioDetails.m_StrategyType = 7;
    }
    
    //Per Future Token Count Should be lass than 100
    if(lnPreviousFutureToken != lstPortfolioDetails.m_nToken1)
      lnPerFutPortfolioCount = 0;
    lnPerFutPortfolioCount++;
    if(lnPerFutPortfolioCount > lnPerFuturePFCount)
    {
      std::cout<< "Per Future Portfolio Count:"<< lnPerFutPortfolioCount<< std::endl;
      std::cout<< "Per Future Portfolio Count Exceeded:"<< lnPerFuturePFCount<< std::endl;
      return EXIT_FAILURE;
    }
        
    lnPreviousFutureToken = lstPortfolioDetails.m_nToken1;
      
    lcPortfoliofileOpen << lnInstanceId                          << "," 
                        << lstPortfolioDetails.m_StrategyType    << "," 
                        << lstPortfolioDetails.m_nToken1         << ","
                        << lstPortfolioDetails.m_nToken2         << ","
                        << lstPortfolioDetails.m_nToken3         << ","
                        << lnSingleOrderLot                      << ","
                        << lnMaxPositionLot                      << ","
                        << lnConSpread                           << ","
                         << "0,0,0,0,0,0,0,0,"                    << ","
                        << lstPortfolioDetails.cSymbol           << "," 
                        << lstPortfolioDetails.m_nStrikePrice    << ","
                        << lcPortfolioName                       << std::endl;

    memset(lcPortfolioName, 0, sizeof(lcPortfolioName));        
    if(lnPortfolioStrategy == 1)
    snprintf(lcPortfolioName, sizeof(lcPortfolioName), "%s%hd%s-%d(R)", lstPortfolioDetails.cSymbol, (lstTimeStruct.tm_year + 1900)%100, MONTH[lstTimeStruct.tm_mon].c_str(), lstPortfolioDetails.m_nStrikePrice/100);
    
    if(lnPortfolioStrategy == 2)
    snprintf(lcPortfolioName, sizeof(lcPortfolioName), "%s%hd%s%d-%d(R)", lstPortfolioDetails.cSymbol, (lstTimeStruct.tm_year + 1900)%100, MONTH[lstTimeStruct.tm_mon].c_str(), lstTimeStruct.tm_mday, lstPortfolioDetails.m_nStrikePrice/100);
      
    lcPortfoliofileOpen << lnInstanceId                          << "," 
                        << lstPortfolioDetails.m_StrategyType + 1<< "," 
                        << lstPortfolioDetails.m_nToken1         << ","
                        << lstPortfolioDetails.m_nToken2         << ","
                        << lstPortfolioDetails.m_nToken3         << ","
                        << lnSingleOrderLot                      << ","
                        << lnMaxPositionLot                      << ","
                        << lnRevSpread                           << ","
                        << "0,0,0,0,0,0,0,0,"                    << ","
                        << lstPortfolioDetails.cSymbol           << "," 
                        << lstPortfolioDetails.m_nStrikePrice    << ","
                        << lcPortfolioName                       << std::endl;
    
    lnPortfolioCount += 2;                        
  }
  
  std::string lnSymbollimitfileName = "INSTANCE_" + std::to_string(lnInstanceId) + "_SymbolLimit.txt";
  std::fstream lcSymbollimitFileOpen(lnSymbollimitfileName, std::ios::out);
  if(!lcSymbollimitFileOpen)
  {
    std::cout<< "Unable To Create Symbol Limit File " << lnSymbollimitfileName << std::endl;
    return EXIT_FAILURE;
  }
  lcSymbollimitFileOpen<< "InstanceID|Token|PriceRange|GrossExposure" << std::endl;
  
  std::string lnTokenListfileName = "INSTANCE_" + std::to_string(lnInstanceId) + "_TokenList.txt";
  std::fstream lcTokenListfileOpen(lnTokenListfileName, std::ios::out);
  if(!lcTokenListfileOpen)
  {
    std::cout<< "Unable To Create Token File";
    return EXIT_FAILURE;
  }
  lcTokenListfileOpen<< lnInstanceId << "=";
  
  int32_t lnTotalTokenCount = 0;
  for(auto& lnToken : lcTokenStore)
  {
    lpcOMSScripInfo = lcBusinessStore.GetOMSScripInfo(lnToken);    
    if(NULL != lpcOMSScripInfo)
    {
      if(lpcOMSScripInfo->nCustomInstrumentType == INSTRUMENT_FUTURE)
      {
        lcSymbollimitFileOpen << lnInstanceId       << "|" 
                              << lnToken            << "|" 
                              << lnPricePriceRange  << "|" 
                              << lnFutGrosssExpore  << std::endl;
      }
      else
      {
        lcSymbollimitFileOpen << lnInstanceId       << "|" 
                              << lnToken            << "|" 
                              << lnPricePriceRange  << "|" 
                              << lnOptGrosssExpore  << std::endl;      
      }
    }
    else
    {
      lcSymbollimitFileOpen   << lnInstanceId       << "|" 
                              << lnToken            << "|" 
                              << lnPricePriceRange  << "|" 
                              << lnOptGrosssExpore  << std::endl;          
    }
    lnTotalTokenCount++;
    lcTokenListfileOpen <<lnToken << ",";
  }
  
  std::cout<< "Total Token:"<< lnTotalTokenCount << "Total Portfolio:" << lnPortfolioCount << std::endl;
  
  lcPortfoliofileOpen.close();
  lcSymbollimitFileOpen.close();
  lcTokenListfileOpen.close();
  return 0;
}
