// QTScript header name definition
script bollingerbands;

// Dependancies
import "library.csh";

// bollingerBands("Centrabit", "LTC/BTC", 20, 2.0, "1m", 0.01);
bollingerBandsBackTest("Centrabit", "LTC/BTC", 20, 2.0, "1m", 0.01, "2022-12-08 00:00:00", "2022-12-08 04:00:00");
stopLoss(0.001);