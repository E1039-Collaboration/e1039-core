#include <iomanip>
#include <cmath>
#include <unistd.h>
#include "PHTimer2.h"
using namespace std;

PHTimer2::PHTimer2(const std::string& name)
  : m_name    (name)
  , m_state   (STOP)
  , m_time_acc(0)
  , m_n_cycle (0)
{
  ;
}

PHTimer2::~PHTimer2()
{
  ;
}

double PHTimer2::get_time_per_cycle() const
{
  return m_n_cycle > 0  ?  m_time_acc / m_n_cycle  :  .0;
}

void PHTimer2::reset()
{
  m_state = STOP;
  m_time_acc = 0;
  m_n_cycle  = 0;
}

void PHTimer2::stop()
{
  if(m_state == STOP) return;
  m_state = STOP;
  get_clock_time(&m_time_stop);
  m_n_cycle++;
  m_time_acc += elapsed();
}

void PHTimer2::restart()
{
  m_state = RUN;
  get_clock_time(&m_time_start);
}

void PHTimer2::reset_and_start()
{
  reset();
  restart();
}

void PHTimer2::print_stat(std::string header, std::ostream& os) const
{
  os << header << " "   << setw(25) << left << m_name << right
     << " "   << setw(15) << get_time_per_cycle() << " ms"
     << " * " << setw( 8) << m_n_cycle
     << " = " << setw( 8) << (int)round(m_time_acc/1000) << " s" << endl;
}

void PHTimer2::test(unsigned int time_msec, std::ostream& os)
{
  reset();
  restart();
  usleep(1000 * time_msec);
  stop();
  os << "PHTimer2::test(): " << m_time_acc << " for " << time_msec << endl;
}

/// In millisecond.
double PHTimer2::elapsed() const
{
  if (m_state == RUN) return -1; // Not supported

  long diff_sec  = m_time_stop.tv_sec  - m_time_start.tv_sec;
  long diff_nsec = m_time_stop.tv_nsec - m_time_start.tv_nsec;

  return 1000 * diff_sec + 1e-6 * diff_nsec;
}

int PHTimer2::get_clock_time(timespec* tp)
{
  return clock_gettime(CLOCK_MONOTONIC_RAW, tp);
}
