import os
import struct
import sys

VERSION = "1.0"

from jpt import Jpt

def process():
	print "-------------------------------------------------------------------------------"
	print "| April JPT Tool " + VERSION
	print "| JPT format version: " + str(Jpt.Version)
	print "-------------------------------------------------------------------------------"
	if len(sys.argv) < 2:
		info()
	elif sys.argv[1].lower() in ("help", "-h", "/h", "-?", "/?"):
		help()
	elif len(sys.argv) != 5:
		info()
	elif sys.argv[1].lower() == "merge":
		print Jpt.merge(sys.argv[2], sys.argv[3], sys.argv[4])
	elif sys.argv[1].lower() == "split":
		print Jpt.split(sys.argv[2], sys.argv[3], sys.argv[4])
	else:
		info()

def info():
	print ""
	print "usage: jpt-tool.py merge|split JPT_FILENAME JPEG_FILENAME PNG_FILNAME"
	print "       use 'jpt-tool.py -h' for more information"
	print ""
	os.system("pause")

def help():
	print ""
	print "usage: jpt-tool.py merge|split JPT_FILENAME JPEG_FILENAME PNG_FILNAME"
	print ""
	print "commands:"
	print "merge      - merges a JPEG and a PNG file into a JPT file"
	print "split      - splits a JPT file to a JPEG and a PNG file"
	print ""
	os.system("pause")

process()
