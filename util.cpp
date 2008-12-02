#include "config.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#if !defined(HAVE_SYS_TIME_H) || defined(TIME_WITH_SYS_TIME)
#include <time.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "util.h"

#ifndef HAVE_RANDOM
#include "compat.h"
#endif


void RandomInit(void)
{
  unsigned long seed;
#ifdef HAVE_GETTIMEOFDAY
  struct timeval tv;
  gettimeofday(&tv, (struct timezone *)0);
  seed = tv.tv_usec + tv.tv_sec + getpid();
#else
  seed = (unsigned long)time((time_t *)0);
#endif
  srandom(seed);
}


unsigned long RandBits(int nbits)
{
  static int remain = 0, maxbits = 8;
  static unsigned long rndbits;
  unsigned long result, tmpbits;

  if( remain < nbits ){
    rndbits = random();
    if( rndbits & ~((1UL << maxbits) - 1) ){
      tmpbits = rndbits >> maxbits;
      while( tmpbits ){
        maxbits++;
        tmpbits >>= 1;
      }
    }
    remain = maxbits;
  }
  result = rndbits & ((1 << nbits) - 1);
  rndbits >>= nbits;
  remain -= nbits;
  return result;
}


double PreciseTime()
{
#ifdef HAVE_CLOCK_GETTIME
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif
  struct timespec nowspec;
  clock_gettime(CLOCK_REALTIME, &nowspec);
  return nowspec.tv_sec + (double)nowspec.tv_nsec/1000000000;
#elif defined(HAVE_GETTIMEOFDAY)
  struct timeval tv;
  gettimeofday(&tv, (struct timezone *)0);
  return tv.tv_sec + (double)tv.tv_usec/1000000;
#else
#error No suitable precision timing functions appear to be available!
#error Please report this problem and identify your system platform.
#endif
}


char *hexencode(const unsigned char *data, size_t length, char *dstbuf)
{
  static char hexdigit[17] = "0123456789abcdef";
  const unsigned char *src, *end;
  char *dst;

  if( 0==length ) length = strlen((char *)data);
  end = data + length;
  if( !dstbuf ) dstbuf = new char[length * 2 + 1];
  dst = dstbuf;

  if( dst ){
    for( src = data; src < end; src++ ){
      *dst++ = hexdigit[*src >> 4];
      *dst++ = hexdigit[*src & 0x0f];
    }
    *dst = '\0';
  }
  return dstbuf;
}


char *hexencode(const char *data, size_t length, char *dstbuf)
{
  return hexencode((unsigned char *)data, length, dstbuf);
}


unsigned char *hexdecode(const char *data, size_t length, unsigned char *dstbuf)
{
  const char *src, *end;
  unsigned char c, *dst;

  if( 0==length ) length = strlen(data);
  end = data + length;
  if( !dstbuf ) dstbuf = new unsigned char[length / 2 + 1];
  dst = dstbuf;

  if( dst ){
    for( src = data; src < end; src += 2, dst++ ){
      c = *src;
      if( c >= '0' && c <= '9' ) *dst = (c - '0') << 4;
      else if( c >= 'a' && c <= 'f' ) *dst = (c - 'a' + 10) << 4;
      else *dst = 0;
      c = *(src + 1);
      if( c >= '0' && c <= '9' ) *dst |= (c - '0');
      else if( c >= 'a' && c <= 'f' ) *dst |= (c - 'a' + 10);
    }
    *dst = '\0';
  }
  return dstbuf;
}

