## Blackbird Bitcoin Arbitrage

### Introduction
Blackbird Bitcoin Arbitrage is a trading system that does automatic long/short arbitrage between Bitcoin exchanges. The software is written in C++ and runs on Linux.

### How It Works
Bitcoin is still a new and inefficient market. Several Bitcoin exchanges exist and the proposed prices (_bid_ and _ask_) can be briefly different from an exchange to another. The purpose of this software is to automatically exploit these differences that occur between Bitcoin exchanges. 

Here is an example with real data. Blackbird analyzes the bid/ask information from two Bitcoin exchanges, Bitfinex and Bitstamp, every five seconds. At some point the spread between Bitfinex and Bitstamp prices is higher than an `ENTRY` threshold (first vertical line): an arbitrage opportunity exists and Blackbird buys Bitsamp and short sells Bitfinex. Then about 4.5 hours later the spread decreases below an `EXIT` threshold (second vertical line) so Blackbird exits the market by selling Bitstamp and buying Bitfinex back.

<p align="center">
<img src="https://cloud.githubusercontent.com/assets/11370278/6548498/64764de6-c5d1-11e4-855b-c2eebb3b782b.png"  width="75%" alt="Spread Example">
</p>


### Disclaimer

__USE THE SOFTWARE AT YOUR OWN RISK. YOU ARE RESPONSIBLE FOR YOUR OWN MONEY. PAST PERFORMANCE IS NOT NECESSARILY INDICATIVE OF FUTURE RESULTS. THE AUTHORS AND ALL AFFILIATES ASSUME NO RESPONSIBILITY FOR YOUR TRADING RESULTS.__


### Arbitrage Parameters

The two parameters used to control the arbitrage are `SpreadEntry` and `SpreadExit`.

* `SpreadEntry`: the limit above which a long/short trade is triggered
* `SpreadExit`: the limit below which the long/short trade is closed

`SpreadEntry` is actually the limit _after_ the exchange fees which means that `SpreadEntry` represents the net profit. If two exchanges have a 0.20% fees for every trade then we will have in total:

* 0.20% entry long + 0.20% entry short + 0.20% exit long + 0.20% exit short = 0.80% total fees

Note: the actual total will be slightly different since it is a percentage of each individual trade.
Now if the profit we target is 0.30% (`"SpreadEntry": 0.0030`) then Blackbird will set the entry threshold at 1.10% (0.80% total fees + 0.30% target).

A smaller spread will generates more trades but with less profit each.


### Code Information

Blackbird uses the base64 functions written by <a href="http://www.adp-gmbh.ch/cpp/common/base64.html" target="_target">René Nyffenegger</a> to encode and decode base64 in C++. Thank you René your code works well with Blackbird.

The trade results are stored in CSV files. A new CSV file is created every time Blackbird is started.

It is possible to properly stop Blackbird after the next trade has closed. While Blackbird is running just create an empty file called _stop_after_exit_ in the working directory. This will make Blackbird automatically stop after the next trade closes.


### How To Test Blackbird

#### Note

Please make sure you understand the disclaimer above if you are going to test the program with real money. You can start by testing with a limited amount of money like $20 per exchange. Note that it is never entirely safe to just tell the system to use only $20 per exchange (parameter `CashForTesting`). You also need to only have $20 available on each of your trading accounts so you will be sure than even with a bug with the `CashForTesting` parameter your maximum loss on an exchange won't be greater than $20 no matter what.

#### Credentials

As of today you need to have accounts on Bitfinex, OKCoin and Bitstamp. Then with each account you need to create API authentication keys. This is usually in the _Settings_ section of your exchange accounts. In v0.0.3 the bid/ask information of Kraken and ItBit are displayed but the functions to send orders haven't been implemented yet.

The file that contains all the parameters is _config.json_. __Never__ share this file as it will contain your exchange credentials!

First add your credentials to _config.json_:

```json
"BitfinexApiKey"
"BitfinexSecretKey"
"OKCoinApiKey"
"OKCoinSecretKey"
"BitstampClientId"
"BitstampApiKey"
"BitstampSecretKey"
```


#### Strategy parameters

Modify the stategy parameters to match your trading style (few trades with high spreads or many trades with low spreads):
```json
"SpreadEntry": 0.0030,
"SpreadExit": -0.0040,
```

#### Risk parameters

If you set `UseFullCash` at `true` then Blackbird will use the minimum cash on the accounts of your two trades, minus a small percentage defined by `UntouchedCash`.
For example, if you have:

```json
"UseFullCash": true,
"UntouchedCash": 0.01,
"CashForTesting": 20.00,
```

And let's say you have $1,000 on your Bitfinex trading account and $1,100 on your OKCoin trading account. The system will then use $990 on each exchange (i.e. $1,000 - 1%). So in this example your total exposure will be $1,980. Now if you change `UseFullCash` to `false` then the system will use $20 per exchange (total exposure $40).


### E-mail parameters (optional)

Blackbird can send you an automatic e-mail every time an arbitrage trade is done. The e-mails contain information like the trade performance. Here are the parameters:

```json
"SendEmail": false,
"SenderAddress": "john@example.com",
"SenderUsername": "john",
"SenderPassword": "pa33w0rd",
"SmtpServerAddress": "smtp.example.com:587",
"ReceiverAddress": "mark@example.com"
```

This feature is optional. If you let `SendEmail` to `false` you don't need to fill the other e-mail parameters.

#### Run the software

You need the following libraries: <a href="https://www.openssl.org/docs/crypto/crypto.html" target="_blank">Crypto</a>, <a href="http://www.digip.org/jansson" target="_blank">Jansson</a> (__v2.7 minimum__), <a href="http://curl.haxx.se" target="_blank">cURL</a> and <a href="http://caspian.dotconf.net/menu/Software/SendEmail" target="_blank">sendEmail</a>.

For instance on Ubuntu you need to install the following libaries:
```
libssl-dev
libjansson-dev
libcurl4-openssl-dev
sendemail
```

Build the software by typing:
```
make
```

Then start the software:
```
./blackbird
```

Alternatively, you may want to start it in the background:

```
./blackbird > output.txt &
```

### Tasks And Issues

Please check the <a href="https://github.com/butor/blackbird/issues" target="_blank">issues page</a> for the current tasks and issues.

### Changelog

##### May 2015

* First release

##### July 2015

 * Bitstamp exchange added
 * Kraken exchange added (bid/ask information only, other functions to be implemented)
 * Improved JSON and cURL exceptions management
 * Added the milliseconds to the nonce used for exchange authentification
 * JSON memory leak fixed
 * Minor fixes and improvements

##### September 2015

 * General performance and stability improvements (merge from _julianmi:performance_improvements_)
 * ItBit exchange added (bid/ask information only, other functions to be implemented)
 * Other minor fixes and improvements


### Links

* <a href="https://bitcointalk.org/index.php?topic=985660.0" target="_blank">Discussion on BitcoinTalk</a>

### License

MIT

### Console Output Example

This is what Blackbird looks like when it is started:


```
Blackbird Bitcoin Arbitrage
DISCLAIMER: USE THE SOFTWARE AT YOUR OWN RISK.

[ Targets ]
   Spread to enter: 0.30%
   Spread to exit: -0.40%

[ Current balances ]
   Bitfinex:    1,434.66 USD    0.000000 BTC
   OKCoin:      1,428.71 USD    0.000262 BTC
   Bitstamp:    1,417.31 USD    0.000000 BTC
   Kraken:      0.00 USD        0.000000 BTC
   ItBit:       0.00 USD        0.000000 BTC

[ Cash exposure ]
   TEST cash used
   Value: $100.00

[ 09/19/2015 09:31:40 ]
   Bitfinex:    231.86 / 231.87
   OKCoin:      231.24 / 231.32
   Bitstamp:    230.51 / 231.20
   Kraken:      230.12 / 232.03
   ItBit:       231.00 / 231.26
   ----------------------------
   Bitfinex/Kraken:     -0.75% [target  1.20%, min -0.75%, max -0.75%]
   OKCoin/Bitfinex:      0.23% [target  1.10%, min  0.23%, max  0.23%]
   OKCoin/Kraken:       -0.52% [target  1.20%, min -0.52%, max -0.52%]
   Bitstamp/Bitfinex:    0.29% [target  1.20%, min  0.29%, max  0.29%]
   Bitstamp/Kraken:     -0.47% [target  1.30%, min -0.47%, max -0.47%]
   Kraken/Bitfinex:     -0.07% [target  1.20%, min -0.07%, max -0.07%]
   ItBit/Bitfinex:       0.26% [target  1.70%, min  0.26%, max  0.26%]
   ItBit/Kraken:        -0.49% [target  1.80%, min -0.49%, max -0.49%]

[ 09/19/2015 09:31:45 ]
   Bitfinex:    231.86 / 231.87
   OKCoin:      231.31 / 231.32
   Bitstamp:    230.51 / 231.20
   Kraken:      230.12 / 232.03
   ItBit:       231.00 / 231.26
   ----------------------------
   Bitfinex/Kraken:     -0.75% [target  1.20%, min -0.75%, max -0.75%]
   OKCoin/Bitfinex:      0.23% [target  1.10%, min  0.23%, max  0.23%]
   OKCoin/Kraken:       -0.52% [target  1.20%, min -0.52%, max -0.52%]
   Bitstamp/Bitfinex:    0.29% [target  1.20%, min  0.29%, max  0.29%]
   Bitstamp/Kraken:     -0.47% [target  1.30%, min -0.47%, max -0.47%]
   Kraken/Bitfinex:     -0.07% [target  1.20%, min -0.07%, max -0.07%]
   ItBit/Bitfinex:       0.26% [target  1.70%, min  0.26%, max  0.26%]
   ItBit/Kraken:        -0.49% [target  1.80%, min -0.49%, max -0.49%]
```
