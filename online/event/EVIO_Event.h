/*
 * EVIO_Event.h
 *
 *  Created on: Sep. 5, 2018
 *  Author: yuhw@nmsu.edu
 */

#ifndef _H_EVIO_Event_H_
#define _H_EVIO_Event_H_

#include "Event.h"

class EVIO_Event : public Event {

public:

  EVIO_Event();

  EVIO_Event(int * data);

  virtual ~EVIO_Event();

  int *get_words(){return words;}

private:
  int *words;
};


#endif /* _H_EVIO_Event_H_ */
