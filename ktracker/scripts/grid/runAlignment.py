#!/usr/bin/env python

import os
import sys
import time
import subprocess
from datetime import datetime
from optparse import OptionParser

import GridUtil as GU

def runCmd(cmd):
    print '>', cmd
    os.system(cmd)

def checkOverflow(output):
    if not os.path.exists(output):
        return True

    for line in open(output).readlines():
        vals = [float(val) for val in line.strip().split()]
        for val in vals:
            if abs(val) > 1.:
                return False
    return True

def prepareConf(log_prev, conf, fudge = 1.):
    print 'Generating conf file for millepede according to', log_prev, 'with fudge factor of', fudge

    if os.path.isfile(conf):
        print 'Conf file exists, will continue'
        return

    sigma = [[0.5, 0.05, 0.005] for i in range(24)]
    if os.path.isfile(log_prev):
        for index, line in enumerate(open(log_prev).readlines()):
            delta = abs(float(line.strip().split()[3]))
            if delta < sigma[index][2]:
                factor = delta/sigma[index][2]
                for i in range(3):
                    sigma[index][i] = sigma[index][i]*factor

    fout = open(conf, 'w')
    for index, line in enumerate(sigma):
        fout.write('%d      %f       %f         %f\n' % (index+1, fudge*line[0], fudge*line[1], fudge*line[2]))
    fout.close()


# parse all the commandline controls
parser = OptionParser('Usage: %prog [options]')
parser.add_option('-r', type = 'int', dest = 'run', help = 'runID used for alignment', default = 0)
parser.add_option('-c', type = 'string', dest = 'config', help = 'I/O configuration file for tracking', default = '')
parser.add_option('-n', type = 'int', dest = 'nIter', help = 'number of iterations to run', default = 10)
parser.add_option('-i', type = 'int', dest = 'initial', help = 'initial iteration number', default = 1)
parser.add_option('-s', type = 'int', dest = 'split', help = 'maximum number of events per job', default = 5000)
parser.add_option('-t', type = 'int', dest = 'timeout', help = 'timeout option', default = 1000)
parser.add_option('-w', type = 'string', dest = 'work', help = 'working directory of the outputs', default = '')
parser.add_option('-e', type = 'string', dest = 'email', help = 'email notification when done', default = '')
parser.add_option('--cali', action = 'store_true', dest = 'cali', help = 'enable chamber calibration', default = False)
parser.add_option('--hodo', action = 'store_true', dest = 'hodo', help = 'enable hodoscope alignment', default = False)
parser.add_option('--prop', action = 'store_true', dest = 'prop', help = 'enable prop tube alignment', default = False)
parser.add_option('--debug', action = 'store_true', dest = 'debug', help = 'enable massive debugging output', default = False)
(options, args) = parser.parse_args()
print options

if len(sys.argv) < 2:
    parser.parse_args(['--help'])

# initialize grid credetial
GU.gridInit()

# prepare the optimized submission sizes
rawFile = 'run_%d_raw.root' % options.run
optSizes = GU.getOptimizedSize(rawFile, options.split, 500)
nJobs = len(optSizes)
if nJobs < 1:
    sys.exit()

conf = GU.JobConfig(options.config)
for i in range(options.initial, options.nIter+1):
    print 'Working on alignment cycle', i

    # build the raw file
    inputFile = 'run_%d_align_%d.root' % (options.run, i)
    runCmd('./update ac %s %s' % (rawFile, inputFile))

    # submit the tracking jobs
    inputFile = os.path.abspath(inputFile)
    cmds = []
    for tag, item in enumerate(optSizes):
        cmd = GU.makeCommand('track', options.run, conf, firstEvent = item[0], nEvents = item[1], outtag = str(tag), infile = inputFile)
        cmds.append(cmd)
    GU.submitAllJobs(cmds)

    # wait for it to finish
    time.sleep(300)
    nMinutes = 5
    nSuccess = 0
    while nSuccess != nJobs:
        nTotalJobs, nFinishedJobs, failedOpts, _ = GU.getJobStatus(conf, 'track', options.run)
        print ' --- Tracking status: ', options.run, nTotalJobs, nFinishedJobs, len(failedOpts), failedOpts

        nSuccess = nFinishedJobs - len(failedOpts)
        failedJobs = []
        for opt in failedOpts:
            failedJobs.append(GU.makeCommandFromOpts('track', opt, conf) + ' --input='+inputFile)
        GU.submitAllJobs(failedJobs)

        if nMinutes > options.timeout and float(nSuccess)/float(nJobs) > 0.8:
            break

        time.sleep(60)
        nMinutes = nMinutes + 1

    # combine the outputs
    outputFile = 'rec_%d_align_%d.root' % (options.run, i)
    tempFiles = [os.path.join(conf.outdir, 'track', GU.version, GU.getSubDir(options.run), 'track_%06d_%s_%d.root' % (options.run, GU.version, tag)) for tag in range(nJobs)]
    if not GU.mergeFiles(outputFile, tempFiles, True):
        print 'Merging failed!'
        break
    for tempFile in tempFiles:
        os.remove(tempFile)

    # chamber alignment
    nTry = 1
    while nTry < 20:
        prepareConf('%s/increase.log_%d' % (options.work, i-1), 'mille.conf', nTry)
        runCmd('./milleAlign %s align_mille_%d.txt increase.log_%d > log_mille_%d' % (outputFile, i, i, i))
        if checkOverflow('increase.log_%d' % i):
            break
        runCmd('rm mille.conf')
        nTry = nTry + 1

    if not checkOverflow('increase.log_%d' % i):
        break
    runCmd('mv align_eval.root %s/align_eval_%d.root' % (options.work, i))
    runCmd('cp align_mille_%d.txt %s' % (i, options.work))
    runCmd('mv align_mille_%d.txt alignment/align_mille.txt' % i)
    runCmd('mv increase.log_%d %s' % (i, options.work))
    runCmd('mv mille.conf %s/mille.conf_%d_%d' % (options.work, i, nTry))
    runCmd('mv log_mille_%d %s' % (i, options.work))
    runCmd('rm %s' % inputFile)

    # chamber calibration
    if options.cali:
        runCmd('./makeRTv7 %d %d %s > %s/log_calibration_%d' % (options.run, i, outputFile, options.work, i))
        runCmd('cp param_calib/calibration_%d.txt %s' % (i, options.work))
        runCmd('mv param_calib/calibration_%d.txt alignment/calibration.txt' % i)
        runCmd('mv plot/* %s' % options.work)

    # hodo alignment
    if options.hodo:
        for m in range(10):
            runCmd('./hodoAlign %s alignment_hodo_%d.txt' % (outputFile, i))
            runCmd('cp alignment_hodo_%d.txt alignment/alignment_hodo.txt' % i)
        runCmd('mv hodo_eval.root %s/hodo_eval_%d.root' % (options.work, i))
        runCmd('mv alignment_hodo_%d.txt %s' % (i, options.work))

    # prop alignment
    if options.prop:
        for m in range(10):
            runCmd('./propAlign %s alignment_prop_%d.txt' % (outputFile, i))
            runCmd('cp alignment_prop_%d.txt alignment/alignment_prop.txt' % i)
        runCmd('mv prop_eval.root %s/prop_eval_%d.root' % (options.work, i))
        runCmd('mv alignment_prop_%d.txt %s' % (i, options.work))

    # final clean up
    runCmd('rm -r %s/*' % conf.outdir)
    runCmd('mv %s %s' % (outputFile, options.work))

GU.stopGridGuard()
if '@' in options.email:
    runCmd('echo "%s" | mail -s "%s" %s' % (' '.join(sys.argv), 'Completed!', options.email))
