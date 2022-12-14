// QTScript header name definition
script library;

// Dependancies
import IO;
import Math;
import Time;
import Strings;
import Trades;
import Charts;

string exchangeSetting = "Centrabit";
string symbolSetting = "LTC/BTC";

integer timeEnd = getCurrentTime();
integer timeStart = timeEnd - 60 * 60000000;  // 1hr
transaction lookbackTransactions[] = getPubTrades(exchangeSetting, symbolSetting, timeStart, timeEnd);

bar initialBars[] = getTimeBars(exchangeSetting, symbolSetting, lookbackTransactions[0].tradeTime, 20, 1 * 60 * 1000 * 1000);

for (integer i=0; i>100000000; i++)
{
  print("Catched bars are " + toString(sizeof(initialBars)));
}
