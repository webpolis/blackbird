### Changelog

##### February 2016

* Spread volatility can be displayed for each pair
* Minor fixes and improvements

##### January 2016

* Removed `AggressiveVolume` parameter as it is covered by `OrderBookFactor`
* More details are written to the log file when an error with cURL or Jansson occurs
* Minor fixes and improvements

##### December 2015

* More user-friendly config file (_blackbird.conf_)
* Bid/ask information can be stored in a MySQL database
* Minor fixes and improvements

##### November 2015

* Trailing spread implemented
* Replaced `SpreadExit` by `SpreadTarget`
* _Demo mode_ implemented
* Blackbird output is now sent to a log file
* Kraken and 796.com fully implemented (__to be tested__)
* Safety measure: Blackbird won't start if one of the BTC accounts is not empty
* Minor fixes and improvements

##### October 2015

* Gemini fully implemented
* No need to have accounts on all the exchanges anymore
* Minor fixes and improvements

##### September 2015

* General performance and stability improvements (merge from _julianmi:performance_improvements_)
* Minor fixes and improvements

##### July 2015

* Bitstamp fully implemented
* Improved JSON and cURL exceptions management
* Added the milliseconds to the nonce used for exchange authentification
* Minor fixes and improvements

##### May 2015

* First release

