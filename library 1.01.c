/*
 * Trading strategies template library - Copyright(C) 2022 Centrabit.com
 * 
 *  - Bollinger Bands (from line 78 to )
 *  - MACD
 *  - RSI
 *  - ParabolicSAR
 *  - Stop loss
*/

// QTScript header name definition
script library;

// Dependancies
import IO;
import Math;
import Time;
import Strings;
import Trades;
import Charts;

/* SMA(Simple Moving Average) calculation
  @ prototype
      float calcSMA(float[]) 
  @ params
      prices: float array of prices
  @ return
      sma float value of the prices in a given array */
float calcSMA (float[] prices) 
{
  integer length = sizeof(prices);
  float sum = 0.0;
  float result = 0.0;

  for (integer i = 0; i < length; i++) 
  {
      sum += prices[i];
  }
  result = (sum / toFloat(length));

  return result;
}

/* Standard Deviation calculation
  @ prototype
      float calcStdDev(float, float[]) 
  @ params
      sma: sma float value of the prices in a given array
      prices: array of prices
  @ return
      sma value of the prices in given array */
float calcStdDev (float sma, float[] prices) 
{
  integer length = sizeof(prices);
  float squaredDifferencesSum = 0.0;
  float meanOfSquaredDifferences = 0.0;
  float result = 0.0;

  // find the sum of the squared differences
  squaredDifferencesSum = 0.0;
  for (integer i = 0; i < length; i++)
  {
    squaredDifferencesSum += pow(prices[i] - sma, toFloat(2));
  }
  // take the mean of the squared differences
  meanOfSquaredDifferences = squaredDifferencesSum / toFloat(length);
  result = sqrt(meanOfSquaredDifferences);

  return result;
}

// Global settings for all algos
string exchangeSetting = "Centrabit";
string symbolSetting = "LTC/BTC";
string position = "flat";
string runningAlgos = "";
float tradingAmountAtOnce = 0.01;

// Global trading informations
float balanceLTC = 0.0;
float balanceBTC = 0.0;
float buyTotal = 0.0;
integer buyCount = 0;
float sellTotal = 0.0;
integer sellCount = 0;
float initialBalanceInLTC = 0.0;
float initialBalanceInBTC = 0.0;

float lastOwnOrderPrice = 0.0;
transaction lookbackTransactions[];   // Only used in backtestmode, it keeps the lookback transactions in given period

/* Stop-Loss Ordering algo

  =====================================================================================

  About Stop-Loss Order (Buy or sell instructions according to predetermined limits):
  -----------------------------------------------------------------------------------

    A stop-loss order is a type of order used by traders to limit their loss or lock in a profit on an existing position.
    Traders can control their exposure to risk by placing a stop-loss order.
    Stop-loss orders are orders with instructions to close out a position by buying or selling a security at the market when it reaches a certain price known as the stop price.

      - A stop-loss order instructs that a stock be bought or sold when it reaches a specified price known as the stop price.
      - Once the stop price is met, the stop order becomes a market order and is executed at the next available opportunity.
      - Stop-loss orders are used to limit loss or lock in profit on existing positions.
      - They can protect investors with either long or short positions.
      - A stop-loss order is different from a stop-limit order, the latter of which must execute at a specific price rather than at the market.
    
    Traders or investors may choose to use a stop-loss order to limit their losses and protect their profits.
    By placing a stop-loss order, they can manage risk by exiting a position if the price for their security starts moving in the direction opposite to the position that they've taken.

    Benefits of Stop-Loss Orders are as follows.
      - Stop-loss orders are a smart and easy way to manage the risk of loss on a trade.
      - They can help traders lock in profit.
      - Every investor can make them a part of their investment strategy.
      - They add discipline to an investor's short-term trading efforts.
      - They take emotions out of trading.
      - They eliminate the need to monitor investments on a daily (or hourly) basis.

  Usage :
  -----------------
    Before buy or sell command, please execute like this,
      stopLoss(0.1);  // 0.1 is a pip

  ===================================================================================== */

float stopLossPip = 0.1;

void stopLoss(float pip)
{
  stopLossPip = pip;
  if (runningAlgos == "")
  {
    runningAlgos = "StopLoss";
  }
  else
  {
    strinsert(runningAlgos, strlength(runningAlgos)-1, "-StopLoss");
  }
}

boolean stopLossTick(float price)
{
  if (position == "flat")
    return false;
  float limitPrice;
  if (position == "long")
  {
    limitPrice = lastOwnOrderPrice * (1.0 - stopLossPip);
    if (price < limitPrice)
    {
      sellMarket(exchangeSetting, symbolSetting, tradingAmountAtOnce, 0);
      drawPoint(getCurrentTime(), price, true, "sell");
      position = "flat";
      print("");
      print("      !!! Long position closed : "+ toString(tradingAmountAtOnce) + "( price- " + toString(price) + ", time- " + timeToString(getCurrentTime(), "yyyy-MM-dd hh:mm:ss") + " )");
      print("");
      return true;
    }
  }
  if (position == "short")
  {
    limitPrice = lastOwnOrderPrice * (1.0 + stopLossPip);
    if (price > limitPrice)
    {
      buyMarket(exchangeSetting, symbolSetting, tradingAmountAtOnce, 0);
      drawPoint(getCurrentTime(), price, false, "sell");
      position = "flat";
      print("");
      print("      !!! Short position closed : "+ toString(tradingAmountAtOnce) + "( price- " + toString(price) + ", time- " + timeToString(getCurrentTime(), "yyyy-MM-dd hh:mm:ss") + " )");
      print("");
      return true;
    }
  }
  return false;
}







/* Bollinger Bands trading strategy

  =====================================================================================

  About Bollinger Bands Indicator:
  --------------------------------

    Bollinger Bands are a popular technical indicator which use standard deviation to establish where a band of likely support and resistance levels might be found. 
    A volatility channel plots lines above and below a central measure of price. 
    These lines, also known as envelopes or bands, widen or contract according to how volatile or non-volatile a market is. 
    Bollinger Bands measure market volatility and provide lots of useful information, including:

      - Trend continuation or reversal
      - Periods of market consolidation
      - Periods of upcoming large volatility breakouts
      - Possible market tops or bottoms and potential price targets
    
    Bollinger Bands consist of three lines, the middle of which is a Simple Moving Average (SMA) with the default value of 20. 
    The upper and lower band are found two standard deviations either side of the SMA.

    The most basic interpretation of Bollinger Bands is that the channels represent a measure of 'highness' and 'lowness'. 
    Let's sum up three key points about Bollinger bands:

      - The upper band shows a level that is statistically high or expensive
      - The lower band shows a level that is statistically low or cheap
      - The Bollinger Band width correlates to the volatility of the market
          - In a more volatile market, Bollinger Bands widen
          - In a less volatile market, the bands narrow

    The Bollinger Bands contain a default setting in Forex trading as (20,2) - 
    where 20 is the value for the SMA and 2 refers to the number of standard deviations the upper and lower band are either side of the SMA.

    The default Bollinger Bands® formula consists of:

    - An N-period moving average (MA)
    - An upper band at K times and an N-period standard deviation above the moving average (MA + Kσ)
    - A lower band at K times and an N-period standard deviation below the moving average (MA − Kσ)

  Usage :
  -----------------


    backtest:
      backtestBollingerBands("Centrabit", "LTC/BTC", 20, 2.0, "1d", 1000, 0.01);
      It tests Bollinger Bands trading strategy with last 1000 days data.
      20 means SMA period, 2.0 is deviation, "1d" represents the duration of a bar.
      Last parameter 0.01 is the amount to sell or buy at once.

  ===================================================================================== */

// Global values for bollinger bands 
integer bollingerSettingPeriod = 20;
float bollingerSettingDeviation = 2.0;
integer bollingerBarTimeLengthInMinutes = 0;

float bollingerSMA = 0.0;
float bollingerSTDDEV = 0.0;
float bollingerUpperBand = 0.0;
float bollingerLowerBand = 0.0;

float bollingerInputPriceArray[];   // In realtime mode it will be used to store all transaction and bar datas, whereas in backtest mode it will only be used for lookback bars.
float bollingerLastPrice = 0.0;
integer bollingerBackTestTickCounter = 0;  // Counting for backtest tick


/* Bollinger upper bands calculation
  @ prototype
      float calcBollingerUpperBand (float sma, float stdev, float k) 
  @ params
      sma: sma float value of the prices in a given array
      stdev: standard deviation caculated from a given array
      k: represents the number of standard deviations applied to the Bollinger Bands indicator.
  @ return
      bollinger upper band value */
float calcBollingerUpperBand (float sma, float stdev, float k) 
{
  return (sma + (k*stdev));
}

/* Bollinger upper bands calculation
  @ prototype
      float calcBollingerLowerBand (float sma, float stdev, float k) 
  @ params
      sma: sma float value of the prices in a given array
      stdev: standard deviation caculated from a given array
      k: represents the number of standard deviations applied to the Bollinger Bands indicator.
  @ return
      bollinger upper band value */
float calcBollingerLowerBand (float sma, float stdev, float k) 
{
  return (sma - (k*stdev));
}

/* Bollinger Bands strategy process
  @ prototype
      string bollingerBands(string exchange, string symbol, float price, integer period, float deviation, string typeStepSymbol)
  @ params
      exchange: exchange string
      symbol: symbol string
      price: new price
      period: period used to calculate SMA
      deviation: deviation float number
      typeStepSymbol: symbol string to represent time step - format : "number" + "expression letter" (ex: "1m", "3m", "5m", "15m", "1d", "3d", ... "1M", "2M"...)
  @ return
      signal string ("buy" or "sell") */
void bollingerBands(string exchange, string symbol, integer period, float deviation, string typeStepSymbol, float tradeAmount)
{  
  bollingerBarTimeLengthInMinutes = toInteger(substring(typeStepSymbol, 0, 1)); // 1m, 5m, 15m, 30min, 1h, 4h, 1d, 1w, 1M
  string timeUnit = substring(typeStepSymbol, 1, 1);

  if (timeUnit == "h")
  {
    bollingerBarTimeLengthInMinutes = bollingerBarTimeLengthInMinutes * 60;
  }
  if (timeUnit == "d")
  {
    bollingerBarTimeLengthInMinutes = bollingerBarTimeLengthInMinutes * 24 * 60;
  }
  if (timeUnit == "w")
  {
    bollingerBarTimeLengthInMinutes = bollingerBarTimeLengthInMinutes * 7 * 24 * 60;
  }
  if (timeUnit == "M")
  {
    bollingerBarTimeLengthInMinutes = bollingerBarTimeLengthInMinutes * 305 * 24 * 6; // means bollingerBarTimeLengthInMinutes * 30.5 * 24 * 60
  }

  bar lookbackBars[] = getTimeBars(exchange, symbol, 0, period, bollingerBarTimeLengthInMinutes * 60 * 1000 * 1000);
  for (integer i=0; i<sizeof(lookbackBars); i++)
  {
    bollingerInputPriceArray >> lookbackBars[i].closePrice;
  }
  setChartsExchange(exchange);
  setChartsSymbol(symbol);
  clearCharts();
  print("SMA period is " + toString(sizeof(lookbackBars)));

  bollingerSMA = calcSMA(bollingerInputPriceArray);
  bollingerSTDDEV = calcStdDev(bollingerSMA, bollingerInputPriceArray);
  bollingerUpperBand = calcBollingerUpperBand(bollingerSMA, bollingerSTDDEV, deviation);
  bollingerLowerBand = calcBollingerLowerBand(bollingerSMA, bollingerSTDDEV, deviation);

  print("Initial SMA :" + toString(bollingerSMA));
  print("Initial bollingerSTDDEV :" + toString(bollingerSTDDEV));
  print("Initial bollingerUpperBand :" + toString(bollingerUpperBand));
  print("Initial bollingerLowerBand :" + toString(bollingerLowerBand));

  exchangeSetting = exchange;
  symbolSetting = symbol;

  bollingerSettingPeriod = period;
  bollingerSettingDeviation = deviation;
  tradingAmountAtOnce = tradeAmount;
  bollingerLastPrice = lookbackBars[sizeof(lookbackBars)-1].closePrice;

  if (runningAlgos == "")
  {
    runningAlgos = "BollingerBandsRealtime";
  }
  else
  {
    strinsert(runningAlgos, strlength(runningAlgos)-1, "-BollingerBandsRealtime");
  }

  addTimer(bollingerBarTimeLengthInMinutes * 60 * 1000);
}

void updateBollingerBands()
{
    // print("time differ is : " + toString(timeDiffer));
    // print("bar time length : " + toString(barTimeLength));
    print("----------------------------------------");
    print("SMA input added : " + toString(bollingerLastPrice) + "  Time:" + timeToString(getCurrentTime(), "yyyy-MM-dd hh:mm:ss"));
    print("Old SMA: " + toString(bollingerSMA));
    bollingerInputPriceArray >> bollingerLastPrice;
    delete bollingerInputPriceArray[0];

    bollingerSMA = calcSMA(bollingerInputPriceArray);
    bollingerSTDDEV = calcStdDev(bollingerSMA, bollingerInputPriceArray);
    bollingerUpperBand = calcBollingerUpperBand(bollingerSMA, bollingerSTDDEV, bollingerSettingDeviation);
    bollingerLowerBand = calcBollingerLowerBand(bollingerSMA, bollingerSTDDEV, bollingerSettingDeviation);

    print("New SMA :" + toString(bollingerSMA));
    // print("bollingerSTDDEV :" + toString(bollingerSTDDEV));
    // print("bollingerUpperBand :" + toString(bollingerUpperBand));
    // print("bollingerLowerBand :" + toString(bollingerLowerBand));
}

void bollingerBandsTick(float price)
{
  bollingerLastPrice = price;

  if (position == "long" || position == "flat")
  {
    if (price > bollingerUpperBand)
    {
      sellMarket(exchangeSetting, symbolSetting, tradingAmountAtOnce, 0);
      drawPoint(getCurrentTime(), price, true, "sell");
      print("--- Market sell ordered : "+ toString(tradingAmountAtOnce) + "( price- " + toString(price) + ", time- " + timeToString(getCurrentTime(), "yyyy-MM-dd hh:mm:ss") + " )");
      lastOwnOrderPrice = price;
      position = "short";
      return;
    }
  }
  if (position == "short" || position == "flat")
  {
    if (price < bollingerLowerBand)
    {
      buyMarket(exchangeSetting, symbolSetting, tradingAmountAtOnce, 0);
      drawPoint(getCurrentTime(), price, false, "buy");
      print("--- Market buy ordered : "+ toString(tradingAmountAtOnce) + "( price- " + toString(price) + ", time- " + timeToString(getCurrentTime(), "yyyy-MM-dd hh:mm:ss") + " )");
      lastOwnOrderPrice = price;
      position = "long";
      return;
    }
  }
  // print("SMA :" + toString(bollingerSMA));
  // print("bollingerUpperBand :" + toString(bollingerUpperBand));
  // print("bollingerLowerBand :" + toString(bollingerLowerBand));

    setLineName("middle");
    setLineColor("yellow");
    drawLine(getCurrentTime(), bollingerSMA);

    setLineName("uppper");
    setLineColor("green");
    drawLine(getCurrentTime(), bollingerUpperBand);

    setLineName("lower");
    setLineColor("red");
    drawLine(getCurrentTime(), bollingerLowerBand);

  // setLineName("price");
  // setLineColor("pink");
  // drawLine(getCurrentTime(), price);
}

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
  bollingerBarTimeLengthInMinutes = toInteger(substring(typeStepSymbol, 0, 1)); // 1m, 5m, 15m, 30min, 1h, 4h, 1d, 1w, 1M
  string timeUnit = substring(typeStepSymbol, 1, 1);

  if (timeUnit == "h")
  {
    bollingerBarTimeLengthInMinutes = bollingerBarTimeLengthInMinutes * 60;
  }
  if (timeUnit == "d")
  {
    bollingerBarTimeLengthInMinutes = bollingerBarTimeLengthInMinutes * 24 * 60;
  }
  if (timeUnit == "w")
  {
    bollingerBarTimeLengthInMinutes = bollingerBarTimeLengthInMinutes * 7 * 24 * 60;
  }
  if (timeUnit == "M")
  {
    bollingerBarTimeLengthInMinutes = bollingerBarTimeLengthInMinutes * 305 * 24 * 6; // means bollingerBarTimeLengthInMinutes * 30.5 * 24 * 60
  }

  // strinsert(strDateTime, strlength(strDateTime)-1, " 00:00:00");
  // strinsert(strEndTime, strlength(strEndTime)-1, " 23:59:59");

  integer timeStart = stringToTime(startDateTime, "yyyy-MM-dd hh:mm:ss");
  integer timeEnd = stringToTime(endDateTime, "yyyy-MM-dd hh:mm:ss");
  // print("Translated start date : " + toString(timeStart));
  // print("Translated end date : " + toString(timeEnd));
  // print("Retranslated start date : " + timeToString(timeStart, "yyyy-MM-dd hh:mm:ss"));
  // print("Retranslated end date : " + timeToString(timeEnd, "yyyy-MM-dd hh:mm:ss"));

  print("Fetching transactions from " + startDateTime + " to " + endDateTime + "...");
  lookbackTransactions = getPubTrades(exchangeSetting, symbolSetting, timeStart, timeEnd);
  // print("   Result : " + toString(sizeof(lookbackTransactions)) + " of transactions are fetched.");
  // print("Started at : " + timeToString(lookbackTransactions[0].tradeTime, "yyyy-MM-dd hh:mm:ss"));
  // print("Ended at : " + timeToString(lookbackTransactions[sizeof(lookbackTransactions)-1].tradeTime, "yyyy-MM-dd hh:mm:ss"));

  // init lookback bar generating
  print("Preparing lookback bars...");
  timeEnd = timeStart;
  timeStart -= (period * bollingerBarTimeLengthInMinutes * 60 * 1000 * 1000);
  transaction tempTransactions[] = getPubTrades(exchangeSetting, symbolSetting, timeStart, timeEnd);
  // print("   Result : " + toString(sizeof(tempTransactions)) + " of transactions are fetched.");
  // print("     lookback started at : " + timeToString(tempTransactions[0].tradeTime, "yyyy-MM-dd hh:mm:ss"));
  // print("     loookback ended at : " + timeToString(tempTransactions[sizeof(tempTransactions)-1].tradeTime, "yyyy-MM-dd hh:mm:ss"));

  integer step = bollingerBarTimeLengthInMinutes * 2;    // one step is 30s in fetched transactions
  integer k=0;
  for (integer i=0; i<period; i++)
  {
    bollingerInputPriceArray >> tempTransactions[k].price;
    // print("     " + toString(i) + "st : "+ timeToString(tempTransactions[k].tradeTime, "yyyy-MM-dd hh:mm:ss") + " price : " + toString(tempTransactions[k].price));
    k+= step;
  }

  setChartsExchange(exchange);
  setChartsSymbol(symbol);
  clearCharts();
  print("SMA period is " + toString(period));

  bollingerSMA = calcSMA(bollingerInputPriceArray);
  bollingerSTDDEV = calcStdDev(bollingerSMA, bollingerInputPriceArray);
  bollingerUpperBand = calcBollingerUpperBand(bollingerSMA, bollingerSTDDEV, deviation);
  bollingerLowerBand = calcBollingerLowerBand(bollingerSMA, bollingerSTDDEV, deviation);

  print("Initial SMA :" + toString(bollingerSMA));
  print("Initial bollingerSTDDEV :" + toString(bollingerSTDDEV));
  print("Initial bollingerUpperBand :" + toString(bollingerUpperBand));
  print("Initial bollingerLowerBand :" + toString(bollingerLowerBand));

  exchangeSetting = exchange;
  symbolSetting = symbol;
  balanceLTC = 1000.0;
  balanceBTC = 0.01;
  initialBalanceInLTC = balanceLTC + (balanceBTC / lookbackTransactions[0].price);
  initialBalanceInBTC = balanceBTC + (balanceLTC * lookbackTransactions[0].price);

  bollingerSettingPeriod = period;
  bollingerSettingDeviation = deviation;
  tradingAmountAtOnce = tradeAmount;
  bollingerLastPrice = bollingerInputPriceArray[sizeof(bollingerInputPriceArray)-1];

  if (runningAlgos == "")
  {
    runningAlgos = "BollingerBandsBackTest";
  }
  else
  {
    strinsert(runningAlgos, strlength(runningAlgos)-1, "-BollingerBandsBackTest");
  }

  print("--------------   Running   -------------------");
  
  addTimer(30);
}

void bollingerBandsBackTestTick()
{
  bollingerBackTestTickCounter ++;

  // if all transactions are tested, finish the backtest
  if (bollingerBackTestTickCounter >= sizeof(lookbackTransactions))
  {
    removeTimer(30);
    // print(toString(bollingerBackTestTickCounter) + "times ticked");
    print("--------------   Result   -------------------");
    print("Total buy : " + toString(buyTotal) + " in " + toString(buyCount) );
    print("Total sell : " + toString(sellTotal) + " in " + toString(sellCount) );
    print("Total profit : " + toString(sellTotal-buyTotal));
    print("BTC balance : " + toString(balanceBTC));
    print("LTC balance : " + toString(balanceLTC));
    print("... caculating profit");
    print("Initial balance in LTC : " + toString(initialBalanceInLTC));
    print("Initial balance in BTC : " + toString(initialBalanceInBTC));
    float lastBalanceInLTC = balanceLTC + (balanceBTC / bollingerLastPrice);
    float lastBalanceInBTC = balanceBTC + (balanceLTC * bollingerLastPrice);
    print("Last balance in LTC : " + toString(lastBalanceInLTC));
    print("Last balance in BTC : " + toString(lastBalanceInBTC));
    print("Profit balance in LTC : " + toString(lastBalanceInLTC - initialBalanceInLTC));
    print("Profit balance in BTC : " + toString(lastBalanceInBTC - initialBalanceInBTC));
    return;
  }

  bollingerLastPrice = lookbackTransactions[bollingerBackTestTickCounter].price;

  // Update bollinger bands when the step time is reached
  integer step = bollingerBarTimeLengthInMinutes * 2;
  if ((bollingerBackTestTickCounter % step) == 0)   // Update bollinger bands
  {
    // print("----------------------------------------");
    // print("SMA input added : " + toString(lookbackTransactions[bollingerBackTestTickCounter].price) + "  Time:" + timeToString(getCurrentTime(), "yyyy-MM-dd hh:mm:ss"));
    // print("Old SMA: " + toString(bollingerSMA));
    bollingerInputPriceArray >> lookbackTransactions[bollingerBackTestTickCounter].price;
    delete bollingerInputPriceArray[0];

    bollingerSMA = calcSMA(bollingerInputPriceArray);
    bollingerSTDDEV = calcStdDev(bollingerSMA, bollingerInputPriceArray);
    bollingerUpperBand = calcBollingerUpperBand(bollingerSMA, bollingerSTDDEV, bollingerSettingDeviation);
    bollingerLowerBand = calcBollingerLowerBand(bollingerSMA, bollingerSTDDEV, bollingerSettingDeviation);

    // print("New SMA :" + toString(bollingerSMA));

    // drawing new bollinger band's points
    setLineName("middle");
    setLineColor("yellow");
    drawLine(lookbackTransactions[bollingerBackTestTickCounter].tradeTime, bollingerSMA);

    setLineName("uppper");
    setLineColor("green");
    drawLine(lookbackTransactions[bollingerBackTestTickCounter].tradeTime, bollingerUpperBand);

    setLineName("lower");
    setLineColor("red");
    drawLine(lookbackTransactions[bollingerBackTestTickCounter].tradeTime, bollingerLowerBand);
  }
  
  // Stop-loss algo stepping
  if (strfind(runningAlgos, "StopLoss") >= 0)
  {
    print("Stop loss running");
    if (stopLossTick(lookbackTransactions[bollingerBackTestTickCounter].price) == true)
    { 
      return;
    }
  }    

  // print(timeToString(lookbackTransactions[bollingerBackTestTickCounter].tradeTime, "yyyy-MM-dd hh:mm:ss"));

  if (position == "long" || position == "flat")
  {
    if (lookbackTransactions[bollingerBackTestTickCounter].price > bollingerUpperBand)
    {
      sellMarket(exchangeSetting, symbolSetting, tradingAmountAtOnce, 0);
      drawPoint(lookbackTransactions[bollingerBackTestTickCounter].tradeTime, lookbackTransactions[bollingerBackTestTickCounter].price, true, "sell");
      print("--- Market sell ordered : "+ toString(tradingAmountAtOnce) + "( price- " + toString(lookbackTransactions[bollingerBackTestTickCounter].price) + ", time- " + timeToString(lookbackTransactions[bollingerBackTestTickCounter].tradeTime, "yyyy-MM-dd hh:mm:ss") + " )");
      lastOwnOrderPrice = lookbackTransactions[bollingerBackTestTickCounter].price;
      position = "short";
      balanceBTC -= lookbackTransactions[bollingerBackTestTickCounter].price * tradingAmountAtOnce;
      balanceLTC += (tradingAmountAtOnce/lookbackTransactions[bollingerBackTestTickCounter].price);
      sellTotal += (lookbackTransactions[bollingerBackTestTickCounter].price * tradingAmountAtOnce);
      sellCount ++;
      return;
    }
  }
  if (position == "short" || position == "flat")
  {
    if (lookbackTransactions[bollingerBackTestTickCounter].price < bollingerLowerBand)
    {
      buyMarket(exchangeSetting, symbolSetting, tradingAmountAtOnce, 0);
      drawPoint(lookbackTransactions[bollingerBackTestTickCounter].tradeTime, lookbackTransactions[bollingerBackTestTickCounter].price, false, "buy");
      print("--- Market buy ordered : "+ toString(tradingAmountAtOnce) + "( price- " + toString(lookbackTransactions[bollingerBackTestTickCounter].price) + ", time- " + timeToString(lookbackTransactions[bollingerBackTestTickCounter].tradeTime, "yyyy-MM-dd hh:mm:ss") + " )");
      lastOwnOrderPrice = lookbackTransactions[bollingerBackTestTickCounter].price;
      position = "long";
      balanceBTC += lookbackTransactions[bollingerBackTestTickCounter].price * tradingAmountAtOnce;
      balanceLTC -= (tradingAmountAtOnce/lookbackTransactions[bollingerBackTestTickCounter-1].price);
      buyTotal += (lookbackTransactions[bollingerBackTestTickCounter].price * tradingAmountAtOnce);
      buyCount ++;  
      return;
    }
  }
  // print("SMA :" + toString(bollingerSMA));
  // print("bollingerUpperBand :" + toString(bollingerUpperBand));
  // print("bollingerLowerBand :" + toString(bollingerLowerBand));
}


/* When the price changed detected
 *
*/
event onLastPriceChanged(string exchange, string symbol, float amount)
{
  if (runningAlgos == "")
  {
    return;
  }
  // Bollinger bands algo stepping
  if (strfind(runningAlgos, "BollingerBandsRealtime") >= 0)
  {
    bollingerBandsTick(amount);
  }
  // Stop-loss algo stepping
  if (strfind(runningAlgos, "StopLoss") >= 0)
  {
    stopLossTick(amount);
  }    
}

event onTimedOut(integer interval) 
{
  if (strfind(runningAlgos, "BollingerBandsRealtime") >= 0)
  {
    updateBollingerBands();
  }
  if (strfind(runningAlgos, "BollingerBandsBackTest") >= 0)
  {
    bollingerBandsBackTestTick();
  }
}

// bollingerBands("Centrabit", "LTC/BTC", 20, 2.0, "1m", 0.01);
bollingerBandsBackTest("Centrabit", "LTC/BTC", 20, 2.0, "1m", 0.01, "2022-12-08 00:00:00", "2022-12-08 04:00:00");
stopLoss(0.001);