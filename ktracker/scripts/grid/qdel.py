#!/usr/bin/env python

import os
import sys
import re
import subprocess
from optparse import OptionParser

# parse all the commandline controls
parser = OptionParser('Usage: %prog [options]')
parser.add_option('-r', '--range', type = 'string', dest = 'range', help = 'range of job ID, e.g. 101-202', default = '')
parser.add_option('-p', '--pattern', type = 'string', dest = 'pattern', help = 'pattern of job name', default = '')
parser.add_option('-s', '--status', type = 'string', dest = 'status', help = 'running status of jobs', default = '')
parser.add_option('-l', '--list', type = 'string', dest = 'list', help = 'list of runIDs to be removed', default = '')
parser.add_option('-d', '--debug', action = 'store_true', dest = 'debug', help = 'Enable massive debugging output', default = False)
(options, args) = parser.parse_args()

if len(sys.argv) < 2:
    parser.parse_args(['--help'])

jobIDmin = -1
jobIDmax = 9999999
if options.range != '':
    jobIDmin = int(options.range.split('-')[0])
    jobIDmax = int(options.range.split('-')[1])

pattern = '[a-z]'
if options.pattern != '':
    pattern = options.pattern

status = options.status

killRunList = []
if options.list != '' and os.path.exists(options.list):
    killRunList = [int(line.strip()) for line in open(options.list)]

output, err = subprocess.Popen('jobsub_q | grep liuk', stdout = subprocess.PIPE, stderr = subprocess.PIPE, shell = True).communicate()
jobDetails = []
for line in output.split('\n'):
    vals = line.strip().split()
    if len(vals) < 3:
        continue

    jobID = int(re.findall(r'^(\d{7})', vals[0])[0])
    runID = int(re.findall(r'_(\d{6})_r', vals[8])[0])
    jobDetails.append((jobID, runID, vals[0], vals[8], vals[5], line))

toBeKilled = []
for job in jobDetails:
    if job[0] < jobIDmin or job[0] > jobIDmax:
        continue
    if len(re.findall(pattern, job[2])) == 0:
        continue
    if status not in job[3]:
        continue
    if len(killRunList) > 0 and (job[1] not in killRunList):
        continue

    print job[3]
    toBeKilled.append(job[2])

proceed = raw_input('Proceed (Y/N) ? ')
if proceed.lower() == 'n':
    sys.exit()

for index, url in enumerate(toBeKilled):
    print '%d/%d' % (index+1, len(toBeKilled)),
    os.system('jobsub_rm --jobid=' + url)
