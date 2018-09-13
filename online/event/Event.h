/*
 * Event.h
 *
 *  Created on: Sep. 5, 2018
 *  Author: yuhw@nmsu.edu
 */

#ifndef _H_Event_H_
#define _H_Event_H_

#include <stdio.h>

class Event{

public:

	Event() {}
  virtual ~Event() {}

  virtual int* get_words() = 0;
};




#endif /* _H_Event_H_ */
