script backtest;

import "library.csh";

bollingerBandsBackTest("Centrabit", "LTC/BTC", 100, 2.0, "1m", 1.0, "2022-11-21 00:00:00", "2022-11-25 20:00:00");
stopLoss(0.008);