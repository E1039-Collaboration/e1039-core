import re
import os
import subprocess
import time
import threading
from datetime import datetime
from ROOT import TFile
from ROOT import TTree

import warnings
warnings.filterwarnings(action = 'ignore', category = RuntimeWarning)

workDir = os.getenv('HOME') + '/tmp'
version = os.getenv("SEAQUEST_RELEASE")
submissionLog = os.path.join(workDir, 'log')
stopGrid = threading.Event()

inputPrefix = {'track' : 'digit', 'vertex' : 'track'}
auxPrefix = {'track' : 'track_from_digit', 'vertex' : 'vertex_from_track'}

class JobConfig:
    """Container of all possible runKTracker script arguments that are not specific to a run"""

    def __init__(self, configFile = ''):
        self.attr = {}           # save the key-value pairs
        self.switch = []         # save the boolean switches
        self.inited = True       # flag to show it's properly initialized

        if not os.path.exists(configFile):
            self.inited = False
            return

        for line in open(configFile).readlines():
            if '#' in line:
                continue

            vals = [val.strip() for val in line.strip().split('=')]
            if len(vals) == 2:
                self.attr[vals[0]] = vals[1].replace('-eq-', '=')
            elif len(vals) == 1 and len(vals[0]) > 0:
                self.switch.append(vals[0])
            else:
                continue

    def __getattr__(self, attr):
        attr = attr.replace('_', '-')   # runKTracker use - instead of _
        if attr in self.attr:
            return self.attr[attr]
        elif attr in self.switch:
            return True
        else:
            return None

    def __str__(self):
        suffix = ' '
        for key in self.attr:
            suffix = suffix + '--%s=%s ' % (key, self.attr[key])
        for item in self.switch:
            suffix = suffix + '--%s ' % item
        return suffix

def getNEvents(name):
    """Find the number of events in either ROOT file or MySQL schema"""

    if not os.path.exists(name):
        print name, 'does not exist!'
        return 0;

    if 'root' in name:
        dataFile = TFile(name, 'READ')
        if dataFile.GetListOfKeys().Contains('save'):
            return dataFile.Get('save').GetEntries()
        else:
            return 0
    else:
        return 0

def getOptimizedSize(name, nEvtMax, nJobsMax = 10, nEvents_default = -1):
    """Optimize how a run is splitted into serveral jobs"""

    nEvents = nEvents_default
    if nEvents < 0:
        nEvents = getNEvents(name)
    if nEvents < 1:
        return []

    nJobs = nEvents/nEvtMax + 1
    if nJobs > nJobsMax:
        print '%s has %d events and more than %d jobs, redueced the number of jobs to %d' % (name, nEvents, nJobsMax, nJobsMax)
        nJobs = nJobsMax
    nEvtMax_opt = nEvents/nJobs

    sizes = []
    for i in range(nJobs-1):
        sizes.append((nEvtMax_opt*i, nEvtMax_opt))
    sizes.append((nEvtMax_opt*(nJobs-1), nEvtMax_opt+1000))

    return sizes

def getSubDir(runID):
    """Return the sub-director name by runID"""

    runNum = int(runID)
    return ('%06d' % runNum)[0:2] + '/' + ('%06d' % runNum)[2:4]

def getJobAttr(optfile):
    """Parse the name and content of job option file to get the basic information"""

    runID = int(re.findall(r'_(\d{6})_', optfile)[0])

    outtag = ''
    if len(re.findall(r'_(\d+).opts', optfile)) > 0:
        outtag = re.findall(r'_(\d+).opts', optfile)[0]

    firstEvent = 0
    nEvents = -1
    for line in open(optfile, 'r').readlines():
        if '#' in line:
            continue

        opt = line.strip().split()
        if len(opt) != 2:
            continue
        elif opt[0] == 'N_Events':
            nEvents = int(opt[1])
        elif opt[0] == 'FirstEvent':
            firstEvent = int(opt[1])

    return (runID, outtag, firstEvent, nEvents)

def gridInit():
    """Initialize grid credential"""
    os.environ['KRB5CCNAME'] = '/e906/app/users/%s/.krbcc/my_cert' % os.getenv('USER')
    if not ticketValid():
        os.system('kinit -A -r7d %s@FNAL.GOV' % os.getenv('USER'))

    startGridGuard()

def ticketValid():
    """Check if the ticket is still valid"""
    return len(os.popen('klist').readlines()) != 0

def gridGuard():
    """Renew the grid credetial every hour"""
    print 'Grid guard is started. '
    nMinutes = 0
    while not stopGrid.isSet():
        time.sleep(60)
        nMinutes = nMinutes + 1
        if nMinutes % 600 == 0:
            print 'Renew the ticket. '
            os.system('kinit -R -c %s' % os.getenv('KRB5CCNAME'))
    print 'Grid guard is terminated after %d minutes. ' % nMinutes

def startGridGuard():
    """Start a new thread used to refresh the gridGuard"""
    guardThread = threading.Thread(name = 'gridGuard', target = gridGuard)
    guardThread.start()

def stopGridGuard():
    """Stop the thread before exiting"""
    stopGrid.set()

def makeCommand(jobType, runID, conf, firstEvent = -1, nEvents = -1, outtag = '', infile = ''):
    """Make a runKTracker.py command according to the configurations"""
    cmd = 'runKTracker.py --grid --%s ' % jobType

    # if input is runID and no infile is specified, it's for data
    if infile == '':
        cmd = cmd + '--run=%d ' % runID
    else:
        cmd = cmd + '--run=%d --input=%s ' % (runID, infile)

    # if start/end eventID is set
    if firstEvent >= 0 and nEvents > 0 and outtag != '':
        cmd = cmd + '--n-events=%d --first-event=%d --outtag=%s' % (nEvents, firstEvent, outtag)

    # add the rest universal ones as defined by config file
    if conf.inited:
        cmd = cmd + str(conf).replace('${RUN}', '%06d' % runID)

    return cmd

def makeCommandFromOpts(jobType, opts, conf):
    """Make a runKTracker command using configuration and opts file"""

    runID, outtag, firstEvent, nEvents = getJobAttr(opts)
    cmd = 'runKTracker.py --grid --%s --run=%d' % (jobType, runID)
    if firstEvent >= 0 and nEvents > 0 and outtag != '':
        cmd = cmd + ' --n-events=%d --first-event=%d --outtag=%s' % (nEvents, firstEvent, outtag)

    if conf.inited:
        cmd = cmd + str(conf).replace('${RUN}', '%06d' % runID)

    return cmd

def submitOneJob(cmd):
    """Actually submit the jobs and parse the return info to see if it's successful"""

    output, err = subprocess.Popen(cmd, stdout = subprocess.PIPE, stderr = subprocess.PIPE, shell = True).communicate()
    try:
        jobID = re.findall(r'cluster (\d*)', output)[0]
        print cmd, 'successful, jobID =', jobID
        return True
    except:
        print cmd, 'failed, will try again later'
        return False

def submitAllJobs(cmds, maxFailCounts = 10):
    """Submit all jobs, retry the failed ones, and save the jobs in errlog when a jobs failed more than once"""

    totalFailCounts = 0
    while len(cmds) != 0:

        cmds_success = []
        start = datetime.now()
        for i, cmd in enumerate(cmds):
            print '%d/%d' % (i+1, len(cmds)),
            if submitOneJob(cmd):
                cmds_success.append(cmd)
            if (i+1) % 10 == 0 and i > 0:
                passedTime = (datetime.now() - start).seconds;
                remainTime = (passedTime/i*(len(cmds) - i))/60.
                print 'ETA: %.2f minuts.' % remainTime

        if len(cmds_success) == 0:
            totalFailCounts = totalFailCounts + 1
            if totalFailCounts > maxFailCounts:
                fout = open(submissionLog, 'a')
                fout.write(str(datetime.now()) + '\n')
                for cmd in cmds:
                    fout.write(cmd + '\n')
                fout.close()
                return False

        for cmd in cmds_success:
            cmds.remove(cmd)

        if len(cmds) == 0:
            return True

        print 'Sleep for 30 seconds ... '
        time.sleep(30)

def getJobStatus(conf, jobType, runID):
    """Get the status of all jobs associated with one runID"""

    checkpoint = -3
    optfiles = [os.path.join(conf.outdir, 'opts', version, getSubDir(runID), f) for f in os.listdir(os.path.join(conf.outdir, 'opts', version, getSubDir(runID))) if ('%06d' % runID) in f and auxPrefix[jobType] in f]

    nFinished = 0
    failedOpts = []
    missingOpts = []
    for optfile in optfiles:
        logfile = os.path.join(conf.outdir, 'log', version, getSubDir(runID), '%s_%06d_%s' % (auxPrefix[jobType], runID, version))
        outfile = os.path.join(conf.outdir, jobType, version, getSubDir(runID), '%s_%06d_%s' % (jobType, runID, version))

        outtag = re.findall(r'_(\d+).opts', optfile)
        if len(outtag) == 0:
            logfile = logfile + '.log'
            outfile = outfile + '.root'
        else:
            logfile = logfile + '_%s.log' % outtag[0]
            outfile = outfile + '_%s.root' % outtag[0]

        if not os.path.exists(logfile) or sum(1 for line in open(logfile)) < abs(checkpoint):
            missingOpts.append(optfile)
            continue

        nFinished = nFinished + 1
        if 'successfully' not in open(logfile).readlines()[checkpoint]:
            failedOpts.append(optfile)
        elif not os.path.exists(outfile):
            failedOpts.append(optfile)

    return (len(optfiles), nFinished, failedOpts, missingOpts)

def runCommand(cmd):
    """Run a bash command and check if there is any stderr output, if not return True, otherwise return False"""
    output, err = subprocess.Popen(cmd, stdout = subprocess.PIPE, stderr = subprocess.PIPE, shell = True).communicate()
    if err == '':
        return True
    else:
        return False

def getTimeStamp():
    return datetime.now().strftime('%y%m%d-%H%M')

def mergeFiles(targetFile, sourceFiles, ignoreCorrupted = False):
    """use hadd to merge ROOT files"""

    cmd = 'hadd '
    if ignoreCorrupted:
        cmd = cmd + '-k -f '

    cmd = cmd + targetFile
    for source in sourceFiles:
        cmd = cmd + ' ' + source

    output, error = subprocess.Popen(cmd, stdout = subprocess.PIPE, stderr = subprocess.PIPE, shell = True).communicate()
    for line in error.strip().split('\n'):
        if 'dictionary' not in line:
            return False

    return True
