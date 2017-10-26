#!/usr/bin/env python

import sys
import os

detectorName2ID = {'D0U': 1, 'D0Up': 2, 'D0X': 3, 'D0Xp': 4, 'D0V': 5, 'D0Vp': 6,
                   'D1V': 7, 'D1Vp': 8, 'D1X': 9, 'D1Xp': 10, 'D1U': 11, 'D1Up': 12,
                   'D2V': 13, 'D2Vp': 14, 'D2Xp': 15, 'D2X': 16, 'D2U': 17, 'D2Up': 18,
                   'D3pVp': 19, 'D3pV': 20, 'D3pXp': 21, 'D3pX': 22, 'D3pUp': 23, 'D3pU': 24,
                   'D3mVp': 25, 'D3mV': 26, 'D3mXp': 27, 'D3mX': 28, 'D3mUp': 29, 'D3mU': 30,
                   'P1Y1': 47, 'P1Y2': 48, 'P1X1': 49, 'P1X2': 50, 'P2X1': 51, 'P2X2': 52, 'P2Y1': 53, 'P2Y2': 54}
detectorGroup = {'D0' : ['D0U', 'D0Up', 'D0X', 'D0Xp', 'D0V', 'D0Vp'], 'D1' : ['D1V', 'D1Vp', 'D1X', 'D1Xp', 'D1U', 'D1Up'],
                 'D2' : ['D2V', 'D2Vp', 'D2Xp', 'D2X', 'D2U', 'D2Up'], 'D3p' : ['D3pVp', 'D3pV', 'D3pXp', 'D3pX', 'D3pUp', 'D3pU'],
                 'D3m' : ['D3mVp', 'D3mV', 'D3mXp', 'D3mX', 'D3mUp', 'D3mU'], 'P1H' : ['P1Y1', 'P1Y2'], 'P1V' : ['P1X1', 'P1X2'],
                 'P2V' : ['P2X1', 'P2X2'], 'P2H' : ['P2Y1', 'P2Y2'],
                 'P' : ['P1Y1', 'P1Y2', 'P1X1', 'P1X2', 'P2X1', 'P2X2', 'P2Y1', 'P2Y2']}

# invert the ID to name
detectorID2Name = {}
for (key, val) in detectorName2ID.iteritems():
    detectorID2Name[val] = key
print detectorID2Name

# read t0, tmin, and tmax
tmin = {}
tmax = {}
chamberInfo = [line.strip() for line in open(sys.argv[1]).readlines()[1:]]
for line in chamberInfo:
    vals = line.split()
    if vals[0][0] == 'P':
        tmax[detectorGroup[vals[0][:3]][0]] = int(vals[5])
        tmin[detectorGroup[vals[0][:3]][0]] = int(vals[5]) - int(vals[7])
        tmax[detectorGroup[vals[0][:3]][1]] = int(vals[5])
        tmin[detectorGroup[vals[0][:3]][1]] = int(vals[5]) - int(vals[7])
    else:
        tmax[vals[0]] = int(vals[5])
        tmin[vals[0]] = int(vals[5]) - int(vals[7])

for i in range(1, 31) + range(47, 55):
    print i, detectorID2Name[i], tmin[detectorID2Name[i]], tmax[detectorID2Name[i]]

# read the R-T for detector group
rtinfo = [line.strip() for line in open(sys.argv[2]).readlines()[1:]]
RT = {}
for detector in detectorGroup[sys.argv[3]]:
    RT[detector] = []

for line in rtinfo:
    vals = line.split()
    if vals[0][0] == 'P':
        RT[detectorGroup[vals[0][:3]][0]].append((tmax[detectorGroup[vals[0][:3]][0]] - float(vals[4]), float(vals[5])))
        RT[detectorGroup[vals[0][:3]][1]].append((tmax[detectorGroup[vals[0][:3]][1]] - float(vals[4]), float(vals[5])))
    else:
        RT[vals[0]].append((tmax[vals[0]] - float(vals[3]), float(vals[4])))

mode = 'w'
if os.path.exists(sys.argv[4]):
    mode = 'a'

fout = open(sys.argv[4], mode)
for i in range(1, 31) + range(47, 55):
    if detectorID2Name[i] not in RT:
        continue

    rt_detector = sorted(RT[detectorID2Name[i]], key = lambda d: d[0])
    fout.write('%d %d %.1f %.1f %s\n' % (i, len(rt_detector), tmin[detectorID2Name[i]], tmax[detectorID2Name[i]], detectorID2Name[i]))
    for j, entry in enumerate(rt_detector):
        fout.write('%d %.1f  %.6f\n' % (j, entry[0], entry[1]))

fout.close()
