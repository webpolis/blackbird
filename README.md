<p align="center">
<img src="https://cloud.githubusercontent.com/assets/11370278/10808535/02230d46-7dc3-11e5-92d8-da15cae8c6e9.png" width="50%" alt="Blackbird Bitcoin Arbitrage">
</p>

### Introduction
Blackbird Bitcoin Arbitrage is a C++ trading system that does automatic long/short arbitrage between Bitcoin exchanges.

### How It Works
Bitcoin is still a new and inefficient market. Several Bitcoin exchanges exist around the world and the proposed prices (_bid_ and _ask_) can be briefly different from an exchange to another. The purpose of Blackbird is to automatically profit from these temporary price differences.

Here is an example with real data. Blackbird analyzes the bid/ask information from two Bitcoin exchanges, Bitfinex and Bitstamp, every few seconds. At some point the spread between Bitfinex and Bitstamp prices is higher than an `ENTRY` threshold (first vertical line): an arbitrage opportunity exists and Blackbird buys Bitstamp and short sells Bitfinex.

Then, about 4.5 hours later the spread decreases below an `EXIT` threshold (second vertical line) so Blackbird exits the market by selling Bitstamp and buying Bitfinex back.

<p align="center">
<img src="https://cloud.githubusercontent.com/assets/11370278/11164055/5863e750-8ab3-11e5-86fc-8f7bab6818df.png"  width="60%" alt="Spread Example">
</p>


### Disclaimer

__USE THE SOFTWARE AT YOUR OWN RISK. YOU ARE RESPONSIBLE FOR YOUR OWN MONEY. PAST PERFORMANCE IS NOT NECESSARILY INDICATIVE OF FUTURE RESULTS. THE AUTHORS AND ALL AFFILIATES ASSUME NO RESPONSIBILITY FOR YOUR TRADING RESULTS.__


### Arbitrage Parameters

The two parameters used to control the arbitrage are `SpreadEntry` and `SpreadTarget`.

* `SpreadEntry`: the limit above which we want the long/short trades to be triggered
* `SpreadTarget`: the spread we want to achieve as an arbitrage opportunity. It represents the net profit and takes the exchange fees into account.

If two exchanges have a 0.20% fees for every trade then we will have for each arbitrage opportunity:

0.20% entry long + 0.20% entry short + 0.20% exit long + 0.20% exit short = 0.80% total fees

Now let's say we have `SpreadEntry` at 1.00% and trades are generated at that level. If the net profit we target (`SpreadTarget`) is 0.30%, then Blackbird will set the exit threshold for these trades at -0.10% (1.00% spread entry - 0.80% total fees - 0.30% target = -0.10% exit threshold).

Defining smaller spreads will generates more trades but with less profit each.


### Code Information

Blackbird uses the base64 functions written by <a href="http://www.adp-gmbh.ch/cpp/common/base64.html" target="_target">René Nyffenegger</a> (thanks to him) to encode and decode base64.

The trade results are stored in CSV files. A new CSV file and a new log file are created every time Blackbird is started.

It is possible to properly stop Blackbird after the next trade has closed. While Blackbird is running just create an empty file named _stop_after_exit_ in the working directory. This will make Blackbird stop after the next trade closes.


### How To Test Blackbird

#### Note

Please make sure that you understand the disclaimer above if you want to test Blackbird with real money. You can start by testing with a limited amount of money, like $25 per exchange.

__IMPORTANT: all your BTC accounts must be empty before starting Blackbird. Make sure that you only have USD on your accounts and no BTC.__

Note that it is never entirely safe to just tell Blackbird to use only $25 per exchange (parameter `CashForTesting`). You also need to only have $25 available on each of your trading accounts as well as 0 BTC. In this case you are sure that even with a bug with `CashForTesting` your maximum loss on an exchange won't be greater than $25 no matter what.

#### Credentials

As of today the exchanges fully implemented are:
* <a href="https://www.bitfinex.com" target="_blank">Bitfinex</a>
* <a href="https://www.okcoin.com" target="_blank">OKCoin</a>
* <a href="https://www.bitstamp.net" target="_blank">Bitstamp</a>
* <a href="https://gemini.com" target="_blank">Gemini</a>
* <a href="https://www.kraken.com" target="_blank">Kraken</a>
* <a href="https://796.com/?lang=en" target="_blank">796.com (Bitcoin futures)</a>

For each of your exchange accounts you need to create API authentication keys. This is usually done in the _Settings_ section of your accounts.

Then, you need to add your API keys in the file _blackbird.conf_. __Never__ share this file as it will contain your personal exchange credentials! If you don't have an account for one of the exchanges just leave it blank. However, you need at least two exchanges and one of them should allow short selling.

##### Demo mode

It is possible to run Blackbird without any credentials by setting the parameter `DemoMode=true`. Blackbird in demo mode will shows you the bid/ask information, the spreads and the arbitrage opportunities but no actual trades will be generated.

#### Strategy parameters

Modify the strategy parameters to match your trading style (few trades with high spreads or many trades with low spreads):
```javascript
SpreadEntry=0.0080
SpreadTarget=0.0020
```

#### Risk parameters

If you set `UseFullCash=true` then Blackbird will use the minimum cash on the accounts of your two trades, minus a small percentage defined by `UntouchedCash`.
For example, if you have:

```javascript
UseFullCash=true
UntouchedCash=0.01
CashForTesting=25.00
MaxExposure=25000.00
```

And let's say you have $1,000 on your Bitfinex trading account and $1,100 on your OKCoin trading account. Blackbird will then use $990 on each exchange, i.e. $1,000 - (1.00% * $1,000) so your total exposure will be $1,980.

Now if you set `UseFullCash=false` then Blackbird will use $25 per exchange (total exposure $50).

`MaxExposure` defines the maximum cash exposure on each exchange ($25,000 per exchange in the example above).


#### E-mail parameters (optional)

Blackbird can send you an e-mail every time an arbitrage trade is completed, with information such as the names of the traded exchanges and the trade return. If you let  `SendEmail=false` then you don't need to fill the other e-mail parameters.

### Run the software

You need the following libraries: <a href="https://www.openssl.org/docs/crypto/crypto.html" target="_blank">Crypto</a>, <a href="http://www.digip.org/jansson" target="_blank">Jansson</a>, <a href="http://curl.haxx.se" target="_blank">cURL</a> and <a href="http://caspian.dotconf.net/menu/Software/SendEmail" target="_blank">sendEmail</a>.

__Note:__ you need Jansson version 2.7 minimum otherwise you will get the following compilation error:

`'json_boolean_value' was not declared in this scope`


Build Blackbird by typing:

`make`

Then start it by typing:

`./blackbird`

### Tasks And Issues

Please check the <a href="https://github.com/butor/blackbird/issues" target="_blank">issues page</a> for the current tasks and issues. If you face any problems with Blackbird please open a new issue on that page.

### Changelog

##### May 2015

* First release

##### July 2015

* Bitstamp exchange added
* Kraken exchange added (bid/ask info only)
* Improved JSON and cURL exceptions management
* Added the milliseconds to the nonce used for exchange authentification
* Minor fixes and improvements

##### September 2015

* General performance and stability improvements (merge from _julianmi:performance_improvements_)
* ItBit exchange added (bid/ask info only)
* Minor fixes and improvements

##### October 2015

* Gemini exchange added and fully implemented
* No need to have accounts on all the exchanges anymore
* Bug <a href="https://github.com/butor/blackbird/issues/16" target="_blank">#16</a> fixed
* Bug <a href="https://github.com/butor/blackbird/issues/19" target="_blank">#19</a> fixed
* Minor fixes and improvements

##### November 2015

* Trailing spread implemented
* Replaced `SpreadExit` by `SpreadTarget`
* _Demo mode_ implemented
* Blackbird output is now sent to a log file
* Kraken fully implemented (__to be tested__)
* 796.com fully implemented (__to be tested__)
* BTC-e exchange added (bid/ask info only)
* Safety measure: Blackbird won't start if one of the BTC accounts is not empty
* Minor fixes and improvements


##### December 2015

* More user-friendly config file (_blackbird.conf_)

### Links

* <a href="https://bitcointalk.org/index.php?topic=985660.0" target="_blank">Discussion about Blackbird on BitcoinTalk</a>

### License

* <a href="https://en.wikipedia.org/wiki/MIT_License" target="_blank">MIT</a>

### Contact

* If you found a bug, please open a new <a href="https://github.com/butor/blackbird/issues" target="_blank">issue</a> with the label _bug_
* If you have a general question or have troubles running Blackbird, you can open a new  <a href="https://github.com/butor/blackbird/issues" target="_blank">issue</a> with the label _question_ or _help wanted_
* For anything else you can directly contact me by email: julien.hamilton@gmail.com

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
