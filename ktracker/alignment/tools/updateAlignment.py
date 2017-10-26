#!/usr/bin/env python

import sys
import MySQLdb
from optparse import OptionParser

# parse all the commandline controls
parser = OptionParser('Usage: %prog [options]')
parser.add_option('-a', '--amend', type = 'string', dest = 'amend', help = 'survey amendents for the geometry', default = '')
parser.add_option('-s', '--server', type = 'string', dest = 'server', help = 'MySQL server', default = 'e906-db2.fnal.gov')
parser.add_option('-p', '--port', type = 'int', dest = 'port', help = 'MySQL port', default = 3306)
parser.add_option('-g', '--geom', type = 'string', dest = 'origin', help = 'original geometry schema to copy from', default = '')
parser.add_option('-t', '--target', type = 'string', dest = 'target', help = 'target geometry schema to create', default = '')
(options, args) = parser.parse_args()

con = MySQLdb.connect(host = options.server, port = options.port, user = 'seaguest', passwd = 'qqbar2mu+mu-')
cur = con.cursor()

# get table names
cur.execute("SELECT table_name FROM information_schema.tables WHERE table_schema='%s'" % options.origin)
tableNames = [row[0] for row in cur.fetchall()]

# copy the tables first
cur.execute('DROP DATABASE IF EXISTS %s' % options.target)
cur.execute('CREATE DATABASE ' + options.target)
for table in tableNames:
    cur.execute('CREATE TABLE %s.%s LIKE %s.%s' % (options.target, table, options.origin, table))
    cur.execute('INSERT %s.%s SELECT * FROM %s.%s' % (options.target, table, options.origin, table))
con.commit()

if options.amend == '':
    sys.exit(0)

# read the new survey
survey = []
for line in open(options.amend).readlines():
    vals = line.strip().split()
    survey.append(vals)

# make the query
filedNames = survey[0]
for vals in survey[1:]:
	query = 'UPDATE %s.Planes SET ' % options.target
	for index, field in enumerate(filedNames):
		if index == 0:
			continue
		query = query + field + '=' + str(vals[index]) + ', '
	query = query[:-2] + " WHERE detectorName='" + vals[0] + "'"

	cur.execute(query)
	print query
con.commit()
