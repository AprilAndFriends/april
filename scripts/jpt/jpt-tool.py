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
	elif sys.argv[1].lower() == "merge":
		if len(sys.argv) != 5:
			info()
		else:
			print Jpt.merge(sys.argv[2], sys.argv[3], sys.argv[4])
	elif sys.argv[1].lower() == "split":
		if len(sys.argv) != 5:
			info()
		else:
			print Jpt.split(sys.argv[2], sys.argv[3], sys.argv[4])
	elif sys.argv[1].lower() == "convert":
		try:
			import Image
			from pilconv import PilConv
		except:
			print "ERROR! Please install PIL to use the 'convert' command."
			print "http://www.pythonware.com/products/pil"
			return
		if len(sys.argv) != 5 and len(sys.argv) != 6:
			info()
		else:
			quality = 75
			if len(sys.argv) == 6:
				quality = int(sys.argv[5])
			print PilConv.convert(sys.argv[2], sys.argv[3], sys.argv[4], quality)
	else:
		info()

def info():
	print ""
	print "usage: jpt-tool.py merge|split JPT_FILENAME JPEG_FILENAME PNG_FILNAME"
	print "       jpt-tool.py convert FILENAME JPEG_FILENAME PNG_FILNAME [JPEG_QUALITY]"
	print "       use 'jpt-tool.py -h' for more information"
	print ""
	os.system("pause")

def help():
	print ""
	print "usage: jpt-tool.py merge|split JPT_FILENAME JPEG_FILENAME PNG_FILNAME"
	print "       jpt-tool.py convert FILENAME JPEG_FILENAME PNG_FILNAME JPEG_QUALITY"
	print ""
	print "commands:"
	print "merge            - merges a JPEG and a PNG file into a JPT file"
	print "split            - splits a JPT file to a JPEG and a PNG file"
	print "JPT_FILENAME     - JPT filename to use in the process"
	print "JPEG_FILENAME    - JPEG filename to use in the process"
	print "PNG_FILENAME     - PNG filename to use in the process"
	print "FILENAME         - image filename to use in the process"
	print "JPEG_QUALITY     - value from 1 (worst) to 95 (best) for JPEG compression quality (default = 75); values above 95 should be avoided, 100 completely disables the quantization stage"
	print ""
	os.system("pause")

process()
