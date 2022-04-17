#ifndef _KALMAN_FAST_TRACKLETTING_H
#define _KALMAN_FAST_TRACKLETTING_H
#include "KalmanFastTracking.h"

class KalmanFastTrackletting : public KalmanFastTracking
{
public:
    explicit KalmanFastTrackletting(const PHField* field, const TGeoManager *geom, bool flag = true);
    virtual ~KalmanFastTrackletting();

    virtual int setRawEvent(SRawEvent* event_input);

    virtual void buildTrackletsInStation(int stationID, int listID, double* pos_exp = nullptr, double* window = nullptr);
};

#endif // _KALMAN_FAST_TRACKLETTING_H
