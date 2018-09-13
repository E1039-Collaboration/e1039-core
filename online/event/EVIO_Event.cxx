/*
 * SQHit_v1.C
 *
 *  Created on: Sep. 5, 2018
 *  Author: yuhw@nmsu.edu
 */

#include "EVIO_Event.h"

EVIO_Event::EVIO_Event() {
	words = NULL;
}

EVIO_Event::EVIO_Event(int* data) {
	words = data;
}

EVIO_Event::~EVIO_Event() {
  delete [] words;
}
