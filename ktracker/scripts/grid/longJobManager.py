#!/usr/bin/env python

import os
import sys
import time
import re
import subprocess
from datetime import datetime
from optparse import OptionParser

import GridUtil as GU

# parse all the commandline controls
parser = OptionParser('Usage: %prog [options]')
parser.add_option('-l', '--list', type = 'string', dest = 'list', help = 'List of run IDs', default = '')
parser.add_option('-j', '--job', type = 'string', dest = 'job', help = 'Type of job: track, vertex, online, etc.', default = '')
parser.add_option('-c', '--config', type = 'string', dest = 'config', help = 'I/O configuration file', default = '')
parser.add_option('-n', '--njobs', type = 'int', dest = 'nJobs', help = 'Number of jobs to split one run', default = 10)
parser.add_option('-e', '--errlog', type = 'string', dest = 'errlog', help = 'Failed command log', default = 'submitAll_err.log')
parser.add_option('-d', '--debug', action = 'store_true', dest = 'debug', help = 'Enable massive debugging output', default = False)
parser.add_option('-s', '--submit', action = 'store_true', dest = 'submit', help = 'Submit the long jobs in smaller batches', default = False)
parser.add_option('-m', '--monitor', action = 'store_true', dest = 'monitor', help = 'Monitor the long job status', default = False)
(options, args) = parser.parse_args()

if len(sys.argv) < 2:
    parser.parse_args(['--help'])

# initialize the configuration
conf = GU.JobConfig(options.config)

# parse the list
jobInfo = [line.strip().split() for line in open(options.list)]
for i in range(len(jobInfo)):

    runID = int(re.findall(r'_(\d+)_r', jobInfo[i][1])[0])
    runID, outtag, firstEvent, nEvents = GU.getJobAttr(os.path.join(conf.outdir, 'opts', GU.version, GU.getSubDir(runID), '%s.opts' % jobInfo[i][1]))
    jobInfo[i] = jobInfo[i] + [runID, outtag, firstEvent, nEvents, False]

# submit jobs
if options.submit:
    cmds = []
    for job in jobInfo:
        singleSize = job[5]/options.nJobs
        optSizes = [[job[4] + i*singleSize, singleSize] for i in range(options.nJobs)]
        optSizes[-1][1] = nEvents - (options.nJobs - 1)*singleSize

        if options.debug:
            print job
            print optSizes

        for tag, item in optSizes:
            cmd = GU.makeCommand(options.job, job[2], conf, firstEvent = item[0], nEvents = item[1], outtag = '%s_%d' % (job[3], tag))
            cmds.append(cmd)
            if options.debug:
                print tag, cmd

    GU.submitAllJobs(cmds)

# monitor job status
if not options.monitor:
    sys.exit()

# real monitor begins
nLeftJobs = len(jobInfo)
while nLeftJobs != 0:
    for job in jobInfo:
        if job[6]:
            continue

        # check if it's finished
        job[6] = True
        outfiles = [os.path.join(conf.outdir, options.job, GU.version, GU.getSubDir(job[2]), '%s_%06d_%s_%s_%d.root' % (options.job, job[2], GU.version, job[3], i)) for i in range(options.nJobs)]
        for outfile in outfiles:
            if not os.path.exists(outfile):
                job[6] = False
                break

        logfiles = [os.path.join(conf.outdir, 'log', GU.version, GU.getSubDir(runID), '%s_%d.log' % (job[1], i)) for i in range(options.nJobs)]
        for logfile in logfiles:
            if not os.path.exists(logfile):
                job[6] = False
                break
            elif sum([1 for line in open(logfile)]) < 3:
                print ' !!! Bad log file 1: ', logfile
                job[6] = False
                break
            elif 'successfully' not in option(logfile).readlines()[-3]:
                print ' !!! Bad log file 2: ', logfile
                job[6] = False
                break

        if not job[6]:
            print ' --- %s not finished yet' % job[1]
            continue
        print ' --- %s finished!' % job[1]

        GU.runCommand('jobsub_rm --jobid=' + job[0])
        time.sleep(15)

        optfiles = [os.path.join(conf.outdir, 'opts', GU.version, GU.getSubDir(runID), '%s_%d.opts' % (job[1], i)) for i in range(options.nJobs)]
        for optfile in optfiles:
            GU.runCommand('rm ' + optfile)

        targetFile = os.path.join(conf.outdir, options.job, GU.version, GU.getSubDir(job[2]), '%s_%06d_%s_%s.root' % (options.job, job[2], GU.version, job[3]))
        if GU.mergeFiles(targetFile, outfiles):
            print ' --- %s merged successfully' % job[1]
            GU.runCommand('cp template.log ' + os.path.join(conf.outdir, 'log', GU.version, GU.getSubDir(runID), '%s.log' % job[1]))
        else:
            print ' !!! %s merge failed.' % job[1]

    nLeftJobs = len(jobInfo) - sum([1 for job in jobInfo if job[6]])
    print '%s: %d jobs left, sleep for 10 minutes.' % (GU.getTimeStamp(), nLeftJobs)
    time.sleep(600)
