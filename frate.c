#define __RAWKS_FRATE__
#define DBG 0

#include "defines.h"
#include <sys/time.h>
#include "protos.h"
#include "globals.h"

#define	DEFAULT_FRAME_RATE	15

static int frame_rate = DEFAULT_FRAME_RATE;
static float frame_time = 1. / DEFAULT_FRAME_RATE;
static struct timeval frame_start, frame_end;

void set_frame_rate(int new_rate);
void copy_time(struct timeval *dst,struct timeval *src);
void set_time(struct timeval *dst,struct timeval *src,long secs,long usecs);
void add_to_time(struct timeval *dst,long secs,long usecs);
float pace_frame(struct timeval *buf);
void start_time(struct timeval *buf);
float end_time(struct timeval *bufThen,struct timeval *bufNow);
long elapsed_sec_tenths(struct timeval *bufThen,struct timeval *bufNow);

void set_frame_rate(int new_rate)
{
  if (new_rate > 0) {
    frame_rate = new_rate;
    frame_time = 1. / frame_rate;
  }
  start_time(&frame_start);
}

void copy_time(struct timeval *dst,struct timeval *src)
{
  bcopy(src, dst, sizeof(struct timeval));
}

void set_time(struct timeval *dst,struct timeval *src,long secs,long usecs)
{
  copy_time(dst, src);
  add_to_time(dst, secs, usecs);
}

void add_to_time(struct timeval *dst,long secs,long usecs)
{
  dst->tv_usec += usecs;
  dst->tv_sec += secs + (dst->tv_usec / 1000000);
  dst->tv_usec %= 1000000;
}

float pace_frame(struct timeval *buf)
{
  float elapsed_time;

  do {
    elapsed_time = end_time(&frame_start, &frame_end);
  } while (elapsed_time < frame_time);
  start_time(&frame_start);

  if (buf != NULL) {
    copy_time(buf, &frame_start);
  }
  return (elapsed_time);
}

void start_time(struct timeval *buf)
{
  struct timezone tzp;

  gettimeofday(buf, &tzp);
}

float end_time(struct timeval *bufThen,struct timeval *bufNow)
{
  long secs;
  long msecs;
  struct timeval tv;
  struct timeval *now;
  struct timeval *then = (struct timeval *)bufThen;
  struct timezone tzp;

  if (bufNow != NULL)
    now = (struct timeval *)bufNow;
  else
    now = &tv;

  gettimeofday(now, &tzp);

  return ((float)((now->tv_sec - then->tv_sec) +
		  (now->tv_usec - then->tv_usec) * 0.000001 ));
}

long elapsed_sec_tenths(struct timeval *bufThen,struct timeval *bufNow)
{
  struct timeval tv;
  struct timeval *now;
  struct timeval *then = bufThen;
  struct timezone tzp;

  if (bufNow != NULL) {
    now = bufNow;
  } else {
    now = &tv;
    gettimeofday(now, &tzp);
  }

  return (((now->tv_sec - then->tv_sec) * 10 +
	   (now->tv_usec - then->tv_usec) / 100000));
}
