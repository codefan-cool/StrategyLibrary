// QTScript header name definition
script bollingerbands;

// Dependancies
import "library.csh";

// bollingerBands("Centrabit", "LTC/BTC", 100, 2.0, "1m", 0.01);
bollingerBandsBackTest("Centrabit", "LTC/BTC", 100, 2.0, "2m", 1.0, "2022-11-25 00:00:00", "2022-11-26 20:00:00");
stopLoss(0.008);