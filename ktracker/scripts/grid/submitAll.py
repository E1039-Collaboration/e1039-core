#!/usr/bin/env python

import os
import sys
from optparse import OptionParser

import GridUtil

# parse all the commandline controls
parser = OptionParser('Usage: %prog [options]')
parser.add_option('-l', '--list', type = 'string', dest = 'list', help = 'List of run IDs', default = '')
parser.add_option('-j', '--job', type = 'string', dest = 'job', help = 'Type of job: track, vertex, online, etc.', default = '')
parser.add_option('-s', '--split', type = 'int', dest = 'nEvtMax', help = 'Constrain the single job to be less than certain events, default is no splitting', default = -1)
parser.add_option('-c', '--config', type = 'string', dest = 'config', help = 'I/O configuration file', default = '')
parser.add_option('-n', '--maxjobs', type = 'int', dest = 'nJobsMax', help = 'Maximum number of jobs to split one run', default = 10)
parser.add_option('-r', '--resubmit', type = 'string', dest = 'resubmit', help = 'Catchup where it has been left', default = '')
parser.add_option('-e', '--errlog', type = 'string', dest = 'errlog', help = 'Failed command log', default = 'submitAll_err.log')
parser.add_option('-m', '--mcmode', action = 'store_true', dest = 'mcmode', help = 'Running for MC data', default = False)
parser.add_option('-d', '--debug', action = 'store_true', dest = 'debug', help = 'Enable massive debugging output', default = False)
(options, args) = parser.parse_args()

if len(sys.argv) < 2:
    parser.parse_args(['--help'])

# initialize grid credetial
GridUtil.gridInit()

# if in catchup mode, then simply submit jobs and exit
if options.resubmit != '':
    cmds = [line.strip() for line in open(options.resubmit).readlines()]
    GridUtil.submitAllJobs(cmds, options.errlog)
    GridUtil.stopGridGuard()
    sys.exit(0)

# initialize the configuration
conf = GridUtil.JobConfig(options.config)

# process runID list, if in MC mode, initialize file list from options.list
runIDs = []
runFiles = []
runLengths = []
if not options.mcmode:    # working with real data, runid is meaningful and is used to find the files
    if os.path.exists(options.list):
        for line in open(options.list):
            vals = line.strip().split()
            if len(vals) == 2:
                runIDs.append(int(vals[0]))
                runLengths.append(int(vals[1]))
            else:
                runIDs.append(int(vals[0]))
                runLengths.append(-1)
    else:
        runIDs = [int(word.strip()) for word in options.list.split(',')]
        runLengths = [-1 for i in range(runIDs)]

    ver = GridUtil.version
    if conf.inv is not None:
        ver = conf.inv
    runFiles = [os.path.join(conf.indir, GridUtil.inputPrefix[options.job], ver, GridUtil.getSubDir(runID), '%s_%06d_%s.root' % (GridUtil.inputPrefix[options.job], runID, ver)) for runID in runIDs]
else:                   # working with MC, when runid is fake but file list is real
    runFiles = [os.path.join(conf.indir, line.strip().split()[1]) for line in open(options.list) if '#' not in line]
    runIDs = [int(line.strip().split()[0]) for line in open(options.list) if '#' not in line]

if options.debug:
    for index, item in enumerate(runFiles):
        print runIDs[index], item

# prepare the job commands
cmds = []
for index, runID in enumerate(runIDs):
    if options.nEvtMax < 0:
        cmd = GridUtil.makeCommand(options.job, runID, conf, infile = runFiles[index])
        cmds.append(cmd)
    else:
        optSizes = GridUtil.getOptimizedSize(runFiles[index], options.nEvtMax, options.nJobsMax, runLengths[index])
        if len(optSizes) == 0:
            continue

        if options.debug:
            print index, runID, len(optSizes), optSizes[-1][1]

        for tag, item in enumerate(optSizes):
            cmd = GridUtil.makeCommand(options.job, runID, conf, firstEvent = item[0], nEvents = item[1], outtag = str(tag), infile = runFiles[index])
            cmds.append(cmd)

GridUtil.submitAllJobs(cmds, 5)
GridUtil.stopGridGuard()
