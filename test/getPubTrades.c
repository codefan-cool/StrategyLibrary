
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

/* Bollinger Bands strategy backtest
  @ prototype
      string backtestBollingerBands(string exchange, string symbol, float price, integer period, float deviation, string typeStepSymbol, integer lookbackPeriod, float bidAmount)
  @ params
      exchange: exchange string
      symbol: symbol string
      period: period used to calculate SMA
      deviation: deviation float number
      typeStepSymbol: symbol string to represent time step - format : "number" + "expression letter" (ex: "1m", "3m", "5m", "15min", "1d", "3d", ... "1M", "2M"...)
      lookbackPeriod: lookback bar count
      tradeAmount: amount of trading(buy or sell) at once
  @ return
      none */
void bollingerBandsBackTest(string exchange, string symbol, integer period, float deviation, string typeStepSymbol, float tradeAmount, string startDateTime, string endDateTime)
{  
  integer timeStart = stringToTime(startDateTime, "yyyy-MM-dd hh:mm:ss");
  integer timeEnd = stringToTime(endDateTime, "yyyy-MM-dd hh:mm:ss");

  // Showing the translating result
  print("Translated start date : " + toString(timeStart));
  print("Translated end date : " + toString(timeEnd));
  // Confirm the result
  print("Retranslated start date : " + timeToString(timeStart, "yyyy-MM-dd hh:mm:ss"));
  print("Retranslated end date : " + timeToString(timeEnd, "yyyy-MM-dd hh:mm:ss"));

  print("Fetching transactions from " + startDateTime + " to " + endDateTime + "...");

  transaction lookbackTransactions[] = getPubTrades(exchangeSetting, symbolSetting, timeStart, timeEnd);

  print("   Result : " + toString(sizeof(lookbackTransactions)) + " of transactions are fetched.");

  // Checking the datetimes for the first and last value in the array.
  print("Started at : " + timeToString(lookbackTransactions[0].tradeTime, "yyyy-MM-dd hh:mm:ss"));
  print("Ended at : " + timeToString(lookbackTransactions[sizeof(lookbackTransactions)-1].tradeTime, "yyyy-MM-dd hh:mm:ss"));
}

bollingerBandsBackTest("Centrabit", "LTC/BTC", 20, 2.0, "1m", 0.01, "2022-10-01 00:00:00", "2022-10-02 00:00:00");
