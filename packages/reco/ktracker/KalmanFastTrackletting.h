#ifndef _KALMAN_FAST_TRACKLETTING_H
#define _KALMAN_FAST_TRACKLETTING_H
#include "KalmanFastTracking.h"

class KalmanFastTrackletting : public KalmanFastTracking
{
  double TX_MAX;
  double TY_MAX;
  double X0_MAX;
  double Y0_MAX;

public:
  explicit KalmanFastTrackletting(const PHField* field, const TGeoManager *geom, bool flag=true, const int verb=0);
    virtual ~KalmanFastTrackletting();

    virtual int setRawEvent(SRawEvent* event_input);

    virtual void buildTrackletsInStation(int stationID, int listID, double* pos_exp = nullptr, double* window = nullptr);
};

#endif // _KALMAN_FAST_TRACKLETTING_H
