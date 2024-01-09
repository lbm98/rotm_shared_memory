```c++
static void crash_handler(int sig)
{
  FILE*     f = stderr;
  time_t    lnTime;
  struct tm stTime;
  char      strdate[32];
```