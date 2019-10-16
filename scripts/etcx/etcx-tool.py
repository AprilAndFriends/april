import os
import struct
import sys

VERSION = "1.0"

from etcx import Etcx

def process():
	print("-------------------------------------------------------------------------------")
	print("| April ETCX Tool " + VERSION)
	print("| ETCX format version: " + str(Etcx.Version))
	print("-------------------------------------------------------------------------------")
	if len(sys.argv) < 2:
		info()
	elif sys.argv[1].lower() in ("help", "-h", "/h", "-?", "/?"):
		help()
	elif sys.argv[1].lower() == "create":
		if len(sys.argv) != 5 and len(sys.argv) != 6:
			info()
			return
		create(sys.argv[2:len(sys.argv)])
	else:
		info()

def create(args):
	if len(args) == 3:
		print(Etcx.create(args[0], args[1], args[2]))
	else:
		print(Etcx.create(args[0], args[1], args[2], int(args[3])))
	
def info():
	print("")
	print("usage: etcx-tool.py create ETCX_FILENAME ETC1_FILENAME ETC1A_FILNAME [ZLIB_COMPRESSION_LEVEL]")
	print("")
	if os.name != 'posix':
		os.system("pause")

def help():
	print("")
	print("usage: etcx-tool.py create ETCX_FILENAME ETC1_FILENAME ETC1A_FILNAME [ZLIB_COMPRESSION_LEVEL]")
	print("")
	print("commands:")
	print("create                 - creates an ETCX file from an ETC1 and an ETC1A file")
	print("")
	print("ETCX_FILENAME          - ETCX filename to use in the process")
	print("ETC1_FILENAME          - ETC1 filename to use in the process")
	print("ETC1A_FILENAME         - ETC1A filename to use in the process")
	print("ZLIB_COMPRESSION_LEVEL - zlib compression level")
	print("")
	os.system("pause")

process()
