import os
import struct
import sys

VERSION = "1.0"

from etcx import Etcx

def process():
	print "-------------------------------------------------------------------------------"
	print "| April ETCX Tool " + VERSION
	print "| ETCX format version: " + str(Etcx.Version)
	print "-------------------------------------------------------------------------------"
	if len(sys.argv) < 2:
		info()
	elif sys.argv[1].lower() in ("help", "-h", "/h", "-?", "/?"):
		help()
	elif sys.argv[1].lower() == "merge":
		if len(sys.argv) != 5:
			info()
			return
		merge(sys.argv[2:len(sys.argv)])
	else:
		info()

def merge(args):
	print Etcx.merge(args[0], args[1], args[2])
	
def info():
	print ""
	print "usage: etcx-tool.py merge ETCX_FILENAME ETC1_FILENAME ETC1A_FILNAME"
	print "       etcx-tool.py split ETCX_FILENAME ETC1_FILENAME ETC1A_FILNAME"
	print "       etcx-tool.py prepare FILENAME ETC1_FILENAME ETC1A_FILNAME [ETC1_QUALITY]"
	print "       etcx-tool.py convert FILENAME ETCX_FILENAME [ETC1_QUALITY]"
	print "       use 'etcx-tool.py -h' for more information"
	print ""
	if os.name != 'posix':
		os.system("pause")

def help():
	print ""
	print "usage: etcx-tool.py merge ETCX_FILENAME ETC1_FILENAME ETC1A_FILNAME"
	print "       etcx-tool.py split ETCX_FILENAME ETC1_FILENAME ETC1A_FILNAME"
	print "       etcx-tool.py prepare FILENAME ETC1_FILENAME ETC1A_FILNAME [ETC1_QUALITY]"
	print "       etcx-tool.py convert FILENAME ETCX_FILENAME [ETC1_QUALITY]"
	print ""
	print "commands:"
	print "merge            - merges a ETC1 and a ETC1A file into a ETCX file"
	print ""
	print "ETCX_FILENAME    - ETCX filename to use in the process"
	print "ETC1_FILENAME    - ETC1 filename to use in the process"
	print "ETC1A_FILENAME   - ETC1A filename to use in the process"
	print ""
	os.system("pause")

process()
