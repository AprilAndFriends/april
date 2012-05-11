import os
import struct
import sys

VERSION_SCRIPT = "1.0"
VERSION_JPT = 1

UINT_PACK_FORMAT = "<I" # little-endian unsigned int

def process():
	print "-------------------------------------------------------------------------------"
	print "| April JPT Tool " + VERSION_SCRIPT
	print "| JPT format version: " + str(VERSION_JPT)
	print "-------------------------------------------------------------------------------"
	if len(sys.argv) < 2:
		info()
	elif sys.argv[1].lower() in ("help", "-h", "/h", "-?", "/?"):
		help()
	elif len(sys.argv) != 5:
		info()
	elif sys.argv[1].lower() == "merge":
		merge(sys.argv[2], sys.argv[3], sys.argv[4])
	elif sys.argv[1].lower() == "split":
		split(sys.argv[2], sys.argv[3], sys.argv[4])
	else:
		info()
	os.system("pause")

def _read_file(filename):
	f = open(filename, "rb")
	data = f.read()
	f.close()
	return data

def _write_file(filename, data):
	f = open(filename, "wb")
	f.write(data)
	f.close()

def info():
	print ""
	print "usage: jpt-tool.py merge|split JPT_FILENAME JPEG_FILENAME PNG_FILNAME"
	print "       use 'jpt-tool.py -h' for more information"
	print ""

def help():
	print ""
	print "usage: jpt-tool.py merge|split JPT_FILENAME JPEG_FILENAME PNG_FILNAME"
	print ""
	print "commands:"
	print "merge      - merges a JPEG and a PNG file into a JPT file"
	print "split      - splits a JPT file to a JPEG and a PNG file"
	print ""

def merge(jpt, jpeg, png):
	if not os.path.exists(jpeg):
		print "ERROR! File '" + jpeg + "' does not exist!"
		return
	if not os.path.exists(png):
		print "ERROR! File '" + png + "' does not exist!"
		return
	jpeg_data = _read_file(jpeg)
	png_data = _read_file(png)
	f = open(jpt, "wb")
	# header
	f.write("JPT")
	f.write(chr(VERSION_JPT))
	# JPEG
	f.write(struct.pack(UINT_PACK_FORMAT, len(jpeg_data)))
	f.write(jpeg_data)
	# PNG
	f.write(struct.pack(UINT_PACK_FORMAT, len(png_data)))
	f.write(png_data)
	# done
	f.close()
	print "File '" + jpt + "' has been successfully merged."

def split(jpt, jpeg, png):
	if not os.path.exists(jpt):
		print "ERROR! File '" + jpt + "' does not exist!"
		return
	f = open(jpt, "rb")
	# header
	header = f.read(3)
	if header != "JPT":
		print "ERROR! File '" + jpt + "' is not a valid JPT file! Expected 'JPT', got '" + header + "'."
		return
	version = ord(f.read(1))
	if version > VERSION_JPT:
		print "ERROR! File '" + jpt + "' version is not supported! Got version " + str(version) + "."
		return
	# write JPEG
	jpeg_size = struct.unpack(UINT_PACK_FORMAT, f.read(4))[0]
	jpeg_data = f.read(jpeg_size)
	_write_file(jpeg, jpeg_data)
	# write PNG
	png_size = struct.unpack(UINT_PACK_FORMAT, f.read(4))[0]
	png_data = f.read(png_size)
	_write_file(png, png_data)
	# done
	f.close()
	print "File '" + jpt + "' has been successfully split."

process()
