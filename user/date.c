#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"

typedef struct {
  int year;
  int month;
  int day;
  int day_of_week;
  int hour;
  int minute;
  int second;
  int millisecond;
} DateTime;

static const long long NANOS_PER_SECOND = 1000000000LL;
static const long long SECONDS_PER_DAY = 1LL * 24 * 60 * 60;
static const long long DAYS_FROM_0000_03_01_TO_1970_01_01 = 719468LL;
static const long long DAYS_PER_400_YEARS = 1LL * 365 * 400 + 97;
static const long long DAYS_PER_100_YEARS = 1LL * 365 * 100 + 24;
static const long long DAYS_PER_4_YEARS = 1LL * 365 * 4 + 1;

static DateTime timestamp_to_datetime(long long nanoseconds_since_epoch) {
  DateTime dt;

  long long seconds_since_epoch = nanoseconds_since_epoch / NANOS_PER_SECOND;
  int nanos_within_second = (int) (nanoseconds_since_epoch % NANOS_PER_SECOND);

  if (nanos_within_second < 0) {
    nanos_within_second += (int) NANOS_PER_SECOND;
    seconds_since_epoch -= 1;
  }
  dt.millisecond = nanos_within_second / 1000000;

  long long total_days = seconds_since_epoch / SECONDS_PER_DAY;
  int seconds_within_day = (int) (seconds_since_epoch % SECONDS_PER_DAY);

  if (seconds_within_day < 0) {
    seconds_within_day += (int) SECONDS_PER_DAY;
    total_days -= 1;
  }

  dt.hour = seconds_within_day / 3600;
  dt.minute = (seconds_within_day % 3600) / 60;
  dt.second = seconds_within_day % 60;

  // These weird formulas are taken from this article: https://howardhinnant.github.io/date_algorithms.html
  long long days_from_0000_03_01 = total_days + DAYS_FROM_0000_03_01_TO_1970_01_01;
  const long long era = (days_from_0000_03_01 >= 0
                           ? days_from_0000_03_01
                           : days_from_0000_03_01 - (DAYS_PER_400_YEARS - 1)) / DAYS_PER_400_YEARS;
  const uint32 doe = (uint32)(days_from_0000_03_01 - era * DAYS_PER_400_YEARS);
  const uint32 yoe = (doe - doe / DAYS_PER_4_YEARS + doe / DAYS_PER_100_YEARS - doe / (DAYS_PER_400_YEARS - 1)) / 365;
  const int y = (int) (yoe) + (int) (era * 400);
  const uint32 doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const uint32 mp = (5 * doy + 2) / 153;
  const uint32 d = doy - (153 * mp + 2) / 5 + 1;
  const uint32 m = mp + (mp < 10 ? 3 : -9);

  dt.year = y + (m <= 2);
  dt.month = (int) m;
  dt.day = (int) d;

  dt.day_of_week = (int) ((total_days % 7 + 3 + 7) % 7 + 1);

  return dt;
}

static const char *days[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
static const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static void itoa_n(int num, const uint32 n, char *buf) {
  for (int i = n - 1; i >= 0; --i) {
    buf[i] = num % 10 + '0';
    num /= 10;
  }
  buf[n] = '\0';
}

static void print_time(const DateTime *rt) {
  char hour[3], minute[3], second[3], millisecond[4];
  itoa_n(rt->hour, 2, hour);
  itoa_n(rt->minute, 2, minute);
  itoa_n(rt->second, 2, second);
  itoa_n(rt->millisecond, 3, millisecond);
  printf("%s %d %s %d %s:%s:%s.%s\n",
         days[rt->day_of_week - 1], rt->day,
         months[rt->month - 1],
         rt->year,
         hour, minute, second, millisecond);
}

int main() {
  uint64 timestamp;
  if (time(&timestamp) == -1) {
    fprintf(2, "date: failed to get timestamp\n");
    return -1;
  }
  DateTime date_time = timestamp_to_datetime(timestamp);
  print_time(&date_time);
  return 0;
}
