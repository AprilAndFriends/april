import os
import struct
import sys

VERSION = "1.0"

from pvrz import Pvrz

def process():
	print("-------------------------------------------------------------------------------")
	print("| April PVRZ Tool " + VERSION)
	print("| PVRZ format version: " + str(Pvrz.Version))
	print("-------------------------------------------------------------------------------")
	if len(sys.argv) < 2:
		info()
	elif sys.argv[1].lower() in ("help", "-h", "/h", "-?", "/?"):
		help()
	elif sys.argv[1].lower() == "create":
		if len(sys.argv) != 4 and len(sys.argv) != 5:
			info()
			return
		create(sys.argv[2:len(sys.argv)])
	else:
		info()

def create(args):
	if len(args) == 2:
		print(Pvrz.create(args[0], args[1]))
	else:
		print(Pvrz.create(args[0], args[1], int(args[2])))
	
def info():
	print("")
	print("usage: pvrz-tool.py create PVRZ_FILENAME PVR_FILENAME [ZLIB_COMPRESSION_LEVEL]")
	print("")
	if os.name != 'posix':
		os.system("pause")

def help():
	print("")
	print("usage: pvrz-tool.py create PVRZ_FILENAME PVR_FILENAME [ZLIB_COMPRESSION_LEVEL]")
	print("")
	print("commands:")
	print("create                 - creates an PVRZ file from a PVR file")
	print("")
	print("PVRZ_FILENAME          - PVRZ output filename")
	print("PVR_FILENAME           - PVR input filename")
	print("ZLIB_COMPRESSION_LEVEL - zlib compression level")
	print("")
	os.system("pause")

process()
