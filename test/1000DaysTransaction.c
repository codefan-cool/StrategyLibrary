
script BollingerBands;

import Math;
import IO;
import Time;
import Trades;
import Charts;

string exchangeSetting = "Centrabit";
string symbolSetting = "LTC/BTC";

integer periodInDays = 30;

integer timeEnd = getCurrentTime();
integer diffTime = periodInDays * 24 * 60 * 60 * 1000 * 1000;  // periodInDays days
integer timeStart = timeEnd - diffTime;

print("--- first fetching ---");

print("Started at " + timeToString(timeEnd, "yyyy-MM-dd hh:mm:ss"));

transaction lastTransactions[] = getPubTrades(exchangeSetting, symbolSetting, timeStart, timeEnd);
//  generateTimeBars(transaction[] history, integer timestamp, integer period, integer barSize)

integer lastTime = getCurrentTime();
print("Finished at " + timeToString(lastTime, "yyyy-MM-dd hh:mm:ss"));


////////////////////////////////////////////////////////////
print("--- second fetching ---");

timeEnd = getCurrentTime();
diffTime = periodInDays * 24 * 60 * 60 * 1000 * 1000;  // periodInDays days
timeStart = timeEnd - diffTime;

print("Started at " + timeToString(timeEnd, "yyyy-MM-dd hh:mm:ss"));

transaction lastTransactionsAgain[] = getPubTrades(exchangeSetting, symbolSetting, timeStart, timeEnd);
//  generateTimeBars(transaction[] history, integer timestamp, integer period, integer barSize)

lastTime = getCurrentTime();
print("Finished at " + timeToString(lastTime, "yyyy-MM-dd hh:mm:ss"));

print("Fetched transaction count : " + toString(sizeof(lastTransactions)));

