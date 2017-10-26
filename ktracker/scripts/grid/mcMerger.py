#!/usr/bin/env python

import os
import sys
import time
import subprocess
from datetime import datetime
from optparse import OptionParser

import GridUtil as GU

# parse all the commandline controls
parser = OptionParser('Usage: %prog [options]')
parser.add_option('-l', '--list', type = 'string', dest = 'list', help = 'List of run IDs', default = '')
parser.add_option('-c', '--conf', type = 'string', dest = 'conf', help = 'Configuration file', default = '')
parser.add_option('-j', '--job', type = 'string', dest = 'job', help = 'type of the job', default = '')
parser.add_option('-s', '--submit', action = 'store_true', dest = 'submit', help = 'submit the missing jobs', default = False)
parser.add_option('-d', '--debug', action = 'store_true', dest = 'debug', help = 'Enable massive debugging output', default = False)
(options, args) = parser.parse_args()

if len(sys.argv) < 2:
    parser.parse_args(['--help'])

conf = GU.JobConfig(options.conf)

# initialize the runlist and file list
runFiles = [line.strip().split()[1] for line in open(options.list)]
runIDs = [int(line.strip().split()[0]) for line in open(options.list)]

cmds = []
for index, runFile in enumerate(runFiles):

    nTotalJobs, nFinishedJobs, failedOpts, missingOpts = GU.getJobStatus(conf, options.job, runIDs[index])
    if len(failedOpts) != 0 or len(missingOpts) != 0 or nTotalJobs != nFinishedJobs:
        print 'Certain job failed for ', runIDs[index], runFile, nTotalJobs, nFinishedJobs, failedOpts, missingOpts
        for opt in failedOpts+missingOpts:
            cmd = GU.makeCommandFromOpts(options.job, opts, conf) + ' --input=%s' % os.path.join(conf.indir, runFile)
            cmds.append(cmd)
        continue

    targetFile = os.path.join(conf.indir, runFile)
    sourceFiles = [os.path.join(conf.outdir, options.job, GU.version, GU.getSubDir(runIDs[index]), '%s_%06d_%s_%s.root' % (options.job, runIDs[index], GU.version, i)) for i in range(nTotalJobs)]

    if options.debug:
        print 'Merging', targetFile, nTotalJobs, nFinishedJobs, failedOpts, sourceFiles

    if not GU.mergeFiles(targetFile, sourceFiles):
        print index, runFile, 'failed merging ... '

if options.submit:
    GU.submitAllJobs(cmds)
else:
    for cmd in cmds:
        print cmd
