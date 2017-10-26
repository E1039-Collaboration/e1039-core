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
parser.add_option('-t', '--tconfig', type = 'string', dest = 'tconfig', help = 'I/O configuration file for tracking', default = '')
parser.add_option('-v', '--vconfig', type = 'string', dest = 'vconfig', help = 'I/O configuration file for vertexing', default = '')
parser.add_option('-r', '--record', type = 'string', dest = 'record', help = 'Record of the submitted runs', default = '')
parser.add_option('-e', '--exclude', type = 'string', dest = 'exclude', help = 'skip certain part of the work chain (t = tracking, v = vertexing, u = uploading)', default = '')
parser.add_option('-d', '--debug', action = 'store_true', dest = 'debug', help = 'Enable massive debugging output', default = False)

parser.add_option('-o', '--output', type = 'string', dest = 'output', help = 'output schema pattern, if not set, uploading will not start', default = '')
parser.add_option('-s', '--server', type = 'string', dest = 'server', help = 'MySQL server', default = 'seaquel.physics.illinois.edu')
parser.add_option('-p', '--port', type = 'int', dest = 'port', help = 'MySQL port', default = 3283)
parser.add_option('-m', '--max', type = 'int', dest = 'nJobsMax', help = 'maximum number of uploading instance', default = 8)
(options, args) = parser.parse_args()

if len(sys.argv) < 2:
    parser.parse_args(['--help'])

# initialize general tracking configuration
tconf = GU.JobConfig(options.tconfig)
vconf = GU.JobConfig(options.vconfig)

# the uploader
uploader = os.path.join(os.getenv('KTRACKER_ROOT'), 'sqlResWriter')

# initialize the runlist
runIDs = [int(line.strip(),split()[0]) for line in open(options.list).readlines()]

# initialize the history record
trackedRuns = []
vertexedRuns = []
uploadedRuns = []
if options.record == '':
    options.record = os.path.join(GU.workDir, 'record_%s.log' % (options.list.replace('list_', '').replace('.txt', '')))
if os.path.exists(options.record):
    fin = open(options.record, 'r')
    for line in fin.readlines():
        vals = line.strip().split()
        if len(vals) < 2:
            continue

        if vals[0] == 't':
            trackedRuns.append(int(vals[1]))
        elif vals[0] == 'v':
            vertexedRuns.append(int(vals[1]))
        elif vals[0] == 'u':
            uploadedRuns.append(int(vals[1]))
    fin.close()

# process excluded part, and make the list distinct
for runID in runIDs:
    if 't' in options.exclude:
        trackedRuns.append(runID)
    if 'v' in options.exclude:
        vertexedRuns.append(runID)
    if 'u' in options.exclude:
        uploadedRuns.append(runID)
trackedRuns = list(set(trackedRuns))
vertexedRuns = list(set(vertexedRuns))
uploadedRuns = list(set(uploadedRuns))

print 'Initial status: %d runs, %d tracked, %d vertexed, %d uploaded' % (len(runIDs), len(trackedRuns), len(vertexedRuns), len(uploadedRuns))
frecord = open(options.record, 'a')
time.sleep(3)

if options.debug:
    print 'Detailed initial status: '
    for runID in runIDs:
        print runID, runID in trackedRuns, runID in vertexedRuns, runID in uploadedRuns

# initialize grid
GU.gridInit()

# initialize all the log files, for job submittion and job monitoring
logfile = os.path.join(GU.workDir, 'monitor_%s_%s.log' % (options.list.replace('list_', '').replace('.txt', ''), GU.getTimeStamp()))
GU.submissionLog = logfile.replace('monitor', 'submission')
fout = open(logfile, 'w')

# loop until all jobs has been uploaded/tracked
while len(uploadedRuns) != len(runIDs) or len(trackedRuns) != len(runIDs) or len(vertexedRuns) != len(runIDs):
    # commands to be re-submitted
    failedJobs = []

    # check the status of tracking jobs, this part handles mergeing/moving ROOT files, and prepare vertex jobs
    vertexJobs = []
    for runID in runIDs:
        # make sure it has not been already processed
        if runID in trackedRuns:
            if not os.path.exists(os.path.join(vconf.outdir, 'opts', GU.version, GU.getSubDir(runID), '%s_%06d_%s.opts' % (GU.auxPrefix['vertex'], runID, GU.version))):
                vertexJobs.append(GU.makeCommand('vertex', runID, vconf))
            continue

        # check the running status
        nTotalJobs, nFinishedJobs, failedOpts, _ = GU.getJobStatus(tconf, 'track', runID)
        if options.debug:
            print ' --- Tracking status: ', runID, nTotalJobs, nFinishedJobs, len(failedOpts), failedOpts
        if len(failedOpts) != 0:   # something wrong
            fout.write('Tracking [%s]: %06d %02d %02d %02d %s\n' % (GU.getTimeStamp(), runID, nTotalJobs, nFinishedJobs, len(failedOpts), 'certain jobs failed'))
            fout.write('              ' + str(failedOpts) + '\n')
            for opt in failedOpts:
                failedJobs.append(GU.makeCommandFromOpts('track', opt, tconf))
            continue
        elif nTotalJobs != nFinishedJobs:   # not completely finished
            continue

        # merge and move the files
        targetDir = os.path.join(vconf.indir, 'track', GU.version, GU.getSubDir(runID))
        if not os.path.exists(targetDir):
            GU.runCommand('mkdir -p ' + targetDir)
            GU.runCommand('chmod 01755 ' + targetDir)

        targetFile = os.path.join(vconf.indir, 'track', GU.version, GU.getSubDir(runID), 'track_%06d_%s.root' % (runID, GU.version))
        mergedFile = os.path.join(GU.workDir, 'track_%06d_%s.root' % (runID, GU.version))
        sourceFiles = [os.path.join(tconf.outdir, 'track', GU.version, GU.getSubDir(runID), 'track_%06d_%s_%d.root' % (runID, GU.version, tag)) for tag in range(nTotalJobs)]
        if GU.mergeFiles(mergedFile, sourceFiles):
            print 'Tracking [%s]: Run %06d finished and merged' % (GU.getTimeStamp(), runID)
            if targetFile == mergedFile or GU.runCommand('mv %s %s' % (mergedFile, targetFile)):
                # label this run as tracked in both runtime list and file recorder
                trackedRuns.append(runID)
                frecord.write('t %06d %s\n' % (runID, GU.getTimeStamp()))

                # submit the vertexing job
                vertexJobs.append(GU.makeCommand('vertex', runID, vconf))

                # clean up the separated sub-files
                for sourceFile in sourceFiles:
                    os.remove(sourceFile)
            else:
                print 'Tracking [%s]: Run %06d failed in moving to pnfs' % (GU.getTimeStamp(), runID)
                fout.write('Tracking [%s]: %06d %02d %02d %02d %s\n' % (GU.getTimeStamp(), runID, nTotalJobs, nFinishedJobs, len(failedOpts), 'moving to pnfs failed'))
                os.remove(mergedFile)
        else:
            print 'Tracking [%s]: Run %06d failed in merging' % (GU.getTimeStamp(), runID)
            fout.write('Tracking [%s]: %06d %02d %02d %02d %s\n' % (GU.getTimeStamp(), runID, nTotalJobs, nFinishedJobs, len(failedOpts), 'merging failed'))
            os.remove(mergedFile)
    print 'Tracking [%s]: %d/%d tracked' % (GU.getTimeStamp(), len(trackedRuns), len(runIDs))
    fout.flush()
    frecord.flush()

    #submit all failed tracking jobs
    GU.submitAllJobs(failedJobs)

    #submit all the vertexing jobs
    GU.submitAllJobs(vertexJobs)

    # check the status of vertexing jobs
    failedJobs = []
    for index, runID in enumerate(trackedRuns):
        # make sure this run has not been processed before
        if runID in vertexedRuns:
            continue

        # check running status
        nTotalJobs, nFinishedJobs, failedOpts, _ = GU.getJobStatus(vconf, 'vertex', runID)
        if options.debug:
            print ' --- Vertexing status: ', runID, nTotalJobs, nFinishedJobs, len(failedOpts), failedOpts
        if len(failedOpts) != 0:
            fout.write('Vertexing [%s]: %06d %02d %02d %02d %s\n' % (GU.getTimeStamp(), runID, nTotalJobs, nFinishedJobs, len(failedOpts), 'certain jobs failed'))
            for opt in failedOpts:
                failedJobs.append(GU.makeCommandFromOpts('vertex', opt, vconf))
            continue
        elif nTotalJobs != nFinishedJobs:
            continue

        # label this run as vertexed
        vertexedRuns.append(runID)
        frecord.write('v %06d %s\n' % (runID, GU.getTimeStamp()))

    print 'Vertexing [%s]: %d/%d vertexed' % (GU.getTimeStamp(), len(vertexedRuns), len(runIDs))
    fout.flush()
    frecord.flush()

    # re-submit all the failed jobs
    GU.submitAllJobs(failedJobs)

    # upload the finished jobs
    nUploaderCycles = 0
    maxCycles = 10 if len(vertexedRuns) < len(runIDs) else 999999
    while len(uploadedRuns) < len(vertexedRuns) and nUploaderCycles < maxCycles:
        toBeUploadedRuns = [runID for runID in vertexedRuns if runID not in uploadedRuns]

        nRunning = int(os.popen('pgrep %s | wc -l' % uploader.split('/')[-1]).read().strip())
        nJobs = options.nJobsMax - nRunning
        if nJobs > len(toBeUploadedRuns):
            nJobs = len(toBeUploadedRuns)
        if nJobs < 0:
            nJobs = 0

        print 'Uploading [%s]: %d uploader running, will submit %d more, %d to go.' % (GU.getTimeStamp(), nRunning, nJobs, len(toBeUploadedRuns)-nJobs)
        for index in range(nJobs):
            runID = toBeUploadedRuns[index]
            sourceFile = os.path.join(vconf.outdir, 'vertex', GU.version, GU.getSubDir(runID), 'vertex_%06d_%s.root' % (runID, GU.version))
            targetSchema = options.output % runID
            uploadLog = os.path.join(GU.workDir, 'log_upload_%06d' % runID)
            uploadErr = os.path.join(GU.workDir, 'err_upload_%06d' % runID)

            cmd = '%s %s %s %s %s %d 1> %s 2> %s &' % (uploader, vconf.opts, sourceFile, targetSchema, options.server, options.port, uploadLog, uploadErr)
            print cmd
            os.system(cmd)

            uploadedRuns.append(runID)
            frecord.write('u %06d %s\n' % (runID, GU.getTimeStamp()))

        # reap IO dead process every 5 minutes
        if (nUploaderCycles+1) % 5 == 0:
            time.sleep(30)
            processInfo = os.popen('ps eo comm,pid,pcpu,stat | grep sqlResWriter').readlines()
            for process in processInfo:
                vals = process.strip().split()
                if 'D' in vals[3] and float(vals[2]) < 0.5:
                    if options.debug:
                        print ' --- kill process ' + vals[1]
                    os.system('kill -9 ' + vals[1])

        # sleep for 1 minute
        frecord.flush()
        nUploaderCycles = nUploaderCycles + 1
        time.sleep(60)

    # sleep for 10 minutes only if we have no runs to upload
    fout.flush()
    frecord.flush()
    time.sleep((maxCycles - nUploaderCycles)*60)

fout.close()
frecord.close()
GU.stopGridGuard()
