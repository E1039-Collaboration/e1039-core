#!/usr/bin/env python

import os
import sys

targetPos = [1, 3, 5, 6, 7]
optsfile = sys.argv[1]
mcFile = sys.argv[2]
nim3filePrefix = sys.argv[3]
outputdir = sys.argv[4]
outputprefix = sys.argv[5]
xfmin = [0.,  0.3,  0.41, 0.50, 0.60, 0.67]
xfmax = [0.3, 0.41, 0.50, 0.60, 0.67, 1.0]

for target in targetPos:
    for i in range(len(xfmin)):
        cmd = "./eventMixer %s %s %s_%d.root %s/clean_%s_%d_Bin%d.root %s/messy_%s_%d_Bin%d.root 'xF > %.2f && xF < %.2f'" % (optsfile, mcFile, nim3filePrefix, target, outputdir, outputprefix, target, i, outputdir, outputprefix, target, i, xfmin[i], xfmax[i])
        print cmd
        os.system(cmd)
