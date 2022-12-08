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

// Global for all algos
string exchangeSetting = "Centrabit";
string symbolSetting = "LTC/BTC";
string position = "flat";
string runningAlgos = "";
float tradingAmountAtOnce = 0.01;


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

float lastOwnOrderPrice = 0.0;

// boolean stopLoss(float price, float pip)
// {
//   float limit = lastOwnOrderPrice * pip;
//   if (position == "long")
//   {
//     if (lastOwnOderPrice < limit )
//     {
//       return true
//     }    
//   }

// }






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

float bollingerSMA = 0.0;
float bollingerSTDDEV = 0.0;
float bollingerUpperBand = 0.0;
float bollingerLowerBand = 0.0;

float bollingerInputPriceArray[];
integer bollingerTimeStepInMinutes = 0;
integer bollingerTimestampOfLastbar = 0;
float bollingerLastPrice = 0.0;
integer bollingerCounter = 0;  // for test

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
  integer timeStepInMinutes = toInteger(substring(typeStepSymbol, 0, 1)); // 1m, 5m, 15m, 30min, 1h, 4h, 1d, 1w, 1M
  string timeUnit = substring(typeStepSymbol, 1, 1);

  if (timeUnit == "h")
  {
    timeStepInMinutes = timeStepInMinutes * 60;
  }
  if (timeUnit == "d")
  {
    timeStepInMinutes = timeStepInMinutes * 24 * 60;
  }
  if (timeUnit == "w")
  {
    timeStepInMinutes = timeStepInMinutes * 7 * 24 * 60;
  }
  if (timeUnit == "M")
  {
    timeStepInMinutes = timeStepInMinutes * 305 * 24 * 6; // means timeStepInMinutes * 30.5 * 24 * 60
  }

  boolean isSuccess = addTimer(1000);
  print("Timer started result -  " + toString(isSuccess));
}

integer lastTimerTicked;

event onTimedOut(integer interval) 
{
  print("--- ticked !!!!!!!!!!");
  print(toString(getCurrentTime() - lastTimerTicked));
  lastTimerTicked = getCurrentTime();
}

bollingerBands("Centrabit", "LTC/BTC", 20, 2.0, "1m", 0.01);

