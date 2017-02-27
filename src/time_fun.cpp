#include <iostream>
#include <time.h>
#include "time_fun.h"

time_t getTime_t(int y, int m, int d, int h, int n, int s)
{
  tm ttm = {0};
  ttm.tm_year = y - 1900;
  ttm.tm_mon = m - 1;
  ttm.tm_mday = d;
  ttm.tm_hour = h;
  ttm.tm_min = n;
  ttm.tm_sec = s;
  ttm.tm_isdst = -1;
  return mktime(&ttm);
}

template <const char *fmt>
std::string fmtDateTime(const time_t &t)
{
  std::string buff(20, '\0');
  buff.resize(strftime(&buff[0], buff.size(), fmt, localtime(&t)));
  return buff;
}

/*
 * Apparently gcc and msvc have a bug
 * 'auto' deduces a type different from the earlier
 * extern declaration.
 * clang compiles with 'auto' however.
 * Also see SO question:
 *  http://stackoverflow.com/q/28835198/234175
 */
extern const char csvfmt[] = "%Y-%m-%d_%H:%M:%S";
const decltype(&fmtDateTime<csvfmt>) printDateTimeCsv = &fmtDateTime<csvfmt>;

extern const char dbfmt[] = "%Y-%m-%d %H:%M:%S";
const decltype(&fmtDateTime<dbfmt>) printDateTimeDb = &fmtDateTime<dbfmt>;

extern const char filenamefmt[] = "%Y%m%d_%H%M%S";
std::string printDateTimeFileName()
{
  return fmtDateTime<filenamefmt>(time(NULL));
}

extern const char defaultfmt[] = "%m/%d/%Y %H:%M:%S";
std::string printDateTime(time_t t)
{
  return fmtDateTime<defaultfmt>(t);
}

std::string printDateTime()
{
  return printDateTime(time(NULL));
}
