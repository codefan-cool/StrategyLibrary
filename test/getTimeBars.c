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

print("Catched bars are " + toString(sizeof(initialBars)));

// for (integer i=0; i<sizeof(initialBars); i++)
// {
//   bollingerInputPriceArray >> initialBars[i].closePrice;
//   // print("     " + toString(i) + "st : "+ timeToString(initialBars[k].timestamp, "yyyy-MM-dd hh:mm:ss") + " price : " + toString(initialBars[k].closePrice));
// }