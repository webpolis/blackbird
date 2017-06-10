#ifndef GETPID_H
#define GETPID_H

#if defined(_MSC_VER)
  #include <windows.h>
  static DWORD getpid() { return GetCurrentProcessId(); }
#else
  #include <unistd.h>
#endif

#endif
