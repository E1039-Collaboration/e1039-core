/*
 * Event.h
 *
 *  Created on: Sep. 5, 2018
 *  Author: yuhw@nmsu.edu
 */

#ifndef _H_Event_H_
#define _H_Event_H_

class Event{

public:

	Event() {}
  virtual ~Event() {}

  virtual int* get_words();
};




#endif /* _H_Event_H_ */
