<p align="center">
<img src="https://cloud.githubusercontent.com/assets/11370278/10808535/02230d46-7dc3-11e5-92d8-da15cae8c6e9.png" width="50%" alt="Blackbird Bitcoin Arbitrage">
</p>

### Introduction
Blackbird Bitcoin Arbitrage is a C++ trading system that does automatic long/short arbitrage between Bitcoin exchanges.

### How It Works
Bitcoin is still a new and inefficient market. Several Bitcoin exchanges exist around the world and the bid/ask prices they propose can be briefly different from an exchange to another. The purpose of Blackbird is to automatically profit from these temporary price differences while being market-neutral.

Here is a real example where an arbitrage opportunity exists between Bitstamp (long) and Bitfinex (short):

<p align="center">
<img src="https://cloud.githubusercontent.com/assets/11370278/11164055/5863e750-8ab3-11e5-86fc-8f7bab6818df.png"  width="60%" alt="Spread Example">
</p>

At the first vertical line, the spread between the exchanges is high so Blackbird buys Bitstamp and short sells Bitfinex. Then, when the spread closes (second vertical line), Blackbird exits the market by selling Bitstamp and buying Bitfinex back.

#### Advantages
Unlike other Bitcoin arbitrage systems, Blackbird doesn't sell but actually _short sells_ Bitcoin on the short exchange. This feature offers two important advantages:

1. The strategy is always market-neutral: the Bitcoin market's moves (up or down) don't impact the strategy returns. This removes a huge risk from the strategy. The Bitcoin market could suddenly lose twice its value that this won't make any difference in the strategy returns.

2. The strategy doesn't need to transfer funds (USD or BTC) between Bitcoin exchanges. The buy/sell and sell/buy trading activities are done in parallel on two different exchanges, independently. Advantage: no need to deal with transfer latency issues.


### Disclaimer

__USE THE SOFTWARE AT YOUR OWN RISK. YOU ARE RESPONSIBLE FOR YOUR OWN MONEY. PAST PERFORMANCE IS NOT NECESSARILY INDICATIVE OF FUTURE RESULTS.__

__THE AUTHORS AND ALL AFFILIATES ASSUME NO RESPONSIBILITY FOR YOUR TRADING RESULTS.__


### Arbitrage Parameters

The two parameters used to control the arbitrage are `SpreadEntry` and `SpreadTarget`.

* `SpreadEntry`: the limit above which we want the long/short trades to be triggered
* `SpreadTarget`: the spread we want to achieve as an arbitrage opportunity. It represents the net profit and takes the exchange fees into account.

If two exchanges have a 0.20% trading fee then we will have 0.80% fees in total for each arbitrage opportunity (entry: 0.20% long + 0.20% short, exit: 0.20% long + 0.20% short).

Let's say we have `SpreadEntry` at 1.00% and trades are generated at that level. If the targeted net profit (`SpreadTarget`) is 0.30%, then Blackbird will set the exit threshold at -0.10% (1.00% spread entry - 0.80% fees - 0.30% target = -0.10% exit threshold).


### Code Information

The trade results are stored in CSV files and the detailed activity is stored in log files. New files are created every time Blackbird is started.

It is possible to automatically stop Blackbird after the next trade has closed by creating, at any time, an empty file named _stop_after_exit_.

By setting `SendEmail=true`, Blackbird will send you an e-mail every time an arbitrage trade is completed, with information such as the names of the exchanges and the trade return.

By setting `UseDatabase=true`, Blackbird will store the bid/ask information of your exchanges to a MySQL database (one table per exchange).

Blackbird uses functions written by <a href="http://www.adp-gmbh.ch/cpp/common/base64.html" target="_target">René Nyffenegger</a> to encode and decode base64.


### How To Test Blackbird

Please make sure that you understand the disclaimer above if you want to test Blackbird with real money, and start with a small amount of money.

__IMPORTANT: all your BTC accounts must be empty before starting Blackbird. Make sure that you only have USD on your accounts and no BTC.__

It is never entirely safe to just tell Blackbird to use, say, $25 per exchange (`CashForTesting=25.00`). You also need to only have $25 available on each of your trading accounts as well as 0 BTC. In this case you are sure that even with a bug with `CashForTesting` your maximum loss on an exchange won't be greater than $25 no matter what.

#### Credentials

As of today the exchanges fully implemented and tested are:
* <a href="https://www.bitfinex.com" target="_blank">__Bitfinex__</a>: _long_ and _short_
* <a href="https://www.okcoin.com" target="_blank">__OKCoin__</a>: _long_ only (__update:__ their API now offers short sellling, <a href="https://www.okcoin.com/about/rest_api.do" target="_blank">link here</a>)
* <a href="https://www.bitstamp.net" target="_blank">__Bitstamp__</a>: _long_ only
* <a href="https://gemini.com" target="_blank">__Gemini__</a>: _long_ only

The following exchanges are fully implemented but haven't been tested:

* <a href="https://www.kraken.com" target="_blank">__Kraken__</a> (validation in progress)
* <a href="https://796.com/?lang=en" target="_blank">__796.com__ (Bitcoin futures)</a> (segfault found, troubleshooting in progress)

The following exchange should be implemented soon:

* <a href="https://poloniex.com" target="_blank">__Poloniex__</a>: _long_ and _short_
* <a href="https://btc-e.com" target="_blank">__BTC-e__</a>: _long_ only
* <a href="https://www.itbit.com" target="_blank">__itBit__</a>: _long_ only

For each of your exchange accounts you need to create the API authentication keys. This is usually done in the _Settings_ section of your accounts.

Then, you need to add your API keys into the file _blackbird.conf_. You need at least two exchanges and one of them should allow short selling. __Never__ share this file as it will contain your personal exchange credentials!

#### Demo mode

It is possible to run Blackbird without any credentials by setting the parameter `DemoMode=true`. Blackbird in demo mode will show you the bid/ask information, the spreads and the arbitrage opportunities but won't generate any trades.

#### Strategy parameters

Modify the strategy parameters to match your trading style:
```javascript
SpreadEntry=0.0080
SpreadTarget=0.0020
```

Small values will generate more trades but with lower return each.

#### Risk parameters

If you set `UseFullCash=true` Blackbird will use the minimum cash on the two accounts of your trades, minus 1.00% as a small margin.
For example, if you have:

```javascript
UseFullCash=true
CashForTesting=25.00
MaxExposure=25000.00
```

And you have $1,000 on your first trading account and $1,100 on your second one, Blackbird will use $990 on each exchange, i.e. $1,000 - (1.00% * $1,000). So your total exposure will be $1,980.

Conversely, if you set `UseFullCash=false` Blackbird will use $25 per exchange (total exposure $50).

`MaxExposure` defines the maximum exposure on each exchange ($25,000 per exchange in the example above).

### Run the software

You need the following libraries: <a href="https://www.openssl.org/docs/crypto/crypto.html" target="_blank">Crypto</a>, <a href="http://www.digip.org/jansson" target="_blank">Jansson</a>, <a href="http://curl.haxx.se" target="_blank">cURL</a>, <a href="http://dev.mysql.com/doc" target="_blank">MySQL Client</a> and <a href="http://caspian.dotconf.net/menu/Software/SendEmail" target="_blank">sendEmail</a>. Usually this is what you need to install:

```
libssl-dev
libjansson-dev
libcurl4-openssl-dev
libmysqlclient-dev
sendemail
```

__Note:__ you need Jansson version 2.7 minimum otherwise you will get the following compilation error:

`'json_boolean_value' was not declared in this scope`


Build Blackbird by typing:

`make`

Then start it by typing:

`./blackbird`

### Tasks And Issues

Please check the <a href="https://github.com/butor/blackbird/issues" target="_blank">issues page</a> for the current tasks and issues. If you face any problems with Blackbird please open a new issue on that page.

### Links

* <a href="https://bitcointalk.org/index.php?topic=985660.0" target="_blank">Discussion about Blackbird on BitcoinTalk</a>

### License

* <a href="https://en.wikipedia.org/wiki/MIT_License" target="_blank">MIT</a>

### Contact

* If you found a bug, please open a new <a href="https://github.com/butor/blackbird/issues" target="_blank">issue</a> with the label _bug_
* If you have a general question or have troubles running Blackbird, you can open a new  <a href="https://github.com/butor/blackbird/issues" target="_blank">issue</a> with the label _question_ or _help wanted_
* For anything else you can contact me at julien.hamilton@gmail.com or on <a href="https://www.linkedin.com/in/julienhamilton" target="_blank">LinkedIn</a>.

### Changelog

* Here is the <a href="https://github.com/butor/blackbird/blob/master/CHANGELOG.md" target="_blank">changelog</a>

### Log Output Example

This is what the log file looks like when Blackbird is started:


```
Blackbird Bitcoin Arbitrage
DISCLAIMER: USE THE SOFTWARE AT YOUR OWN RISK.

[ Targets ]
   Spread Entry:  0.80%
   Spread Target: 0.30%

[ Current balances ]
   Bitfinex:    1,857.79 USD    0.000000 BTC
   OKCoin:      1,801.38 USD    0.000436 BTC
   Bitstamp:    1,694.15 USD    0.000000 BTC
   Gemini:      1,720.38 USD    0.000000 BTC

[ Cash exposure ]
   FULL cash used!

[ 10/31/2015 08:32:45 ]
   Bitfinex:    325.21 / 325.58
   OKCoin:      326.04 / 326.10
   Bitstamp:    325.37 / 325.82
   Gemini:      325.50 / 328.74
   ----------------------------
   OKCoin/Bitfinex:     -0.27% [target  0.80%, min -0.27%, max -0.27%]
   Bitstamp/Bitfinex:   -0.19% [target  0.80%, min -0.19%, max -0.19%]
   Gemini/Bitfinex:     -1.07% [target  0.80%, min -1.07%, max -1.07%]

[ 10/31/2015 08:32:48 ]
   Bitfinex:    325.21 / 325.58
   OKCoin:      326.04 / 326.10
   Bitstamp:    325.39 / 325.68
   Gemini:      325.50 / 328.67
   ----------------------------
   OKCoin/Bitfinex:     -0.27% [target  0.80%, min -0.27%, max -0.27%]
   Bitstamp/Bitfinex:   -0.14% [target  0.80%, min -0.19%, max -0.14%]
   Gemini/Bitfinex:     -1.05% [target  0.80%, min -1.07%, max -1.05%]
```
