#ifndef __PH_TIMER_2_H__
#define __PH_TIMER_2_H__
#include <iostream>
#include <string>
#include <ctime>

/// Class to measure the time spent by a code block.
/**
 * It is a new (2nd) version of `PHTimer`.
 * `PHTimer` was found to have two problems as of 2022-05-13:
 * - It breaks the pthread mutex (i.e. `pthread_mutex_unlock()` fails),
 *   probably because `PHTimer` uses the assembly call.
 * - The measured time is longer than the actual time by 30-80%,
 *   probably because the CPU frequency is changing.
 */
class PHTimer2
{
  enum State { STOP, RUN };

  std::string  m_name;
  State        m_state;
  timespec     m_time_start;
  timespec     m_time_stop;
  double       m_time_acc;
  unsigned int m_n_cycle;

 public:
  PHTimer2(const std::string& name="Generic Timer");
  virtual ~PHTimer2();

  void        set_name(const std::string& name) { m_name = name; }
  std::string get_name() const { return m_name; }

  double       get_accumulated_time() const { return m_time_acc; }
  unsigned int get_ncycle          () const { return m_n_cycle; }
  double       get_time_per_cycle  () const;

  void reset();
  void stop();
  void restart();
  void reset_and_start();

  void print_stat(std::string header="  Timer2:", std::ostream& os=std::cout) const;

  /// Test this class by waiting for a given time (in ms).
  void test(unsigned int time_msec, std::ostream& os=std::cout);

 protected:  
  double elapsed() const;
  int get_clock_time(timespec* tp);
};

#endif
