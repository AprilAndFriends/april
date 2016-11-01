import os
import struct
import sys

UINT_PACK_FORMAT = "<I" # little-endian unsigned int

def _read_file(filename):
	f = open(filename, "rb")
	header = f.read(Etcx.HEADER_SIZE)
	data = f.read()
	f.close()
	return [header, data]

class Etcx:
	
	Version = 1
	HEADER_SIZE = 13 * 4
	WIDTH_HEADER_OFFSET = 7 * 4
	HEIGHT_HEADER_OFFSET = 6 * 4
	
	@staticmethod
	def merge(etcx, etc1, etc1a):
		if not os.path.exists(etc1):
			return "ERROR! File '%s' does not exist!" % etc1
		if not os.path.exists(etc1a):
			return "ERROR! File '%s' does not exist!" % etc1a
		header, etc1_data = _read_file(etc1)
		dummy_header, etc1a_data = _read_file(etc1a)
		f = open(etcx, "wb")
		# header
		f.write("ETCX")
		f.write(struct.pack(UINT_PACK_FORMAT, 1))
		f.write(header[Etcx.WIDTH_HEADER_OFFSET : Etcx.WIDTH_HEADER_OFFSET + 4])
		f.write(header[Etcx.HEIGHT_HEADER_OFFSET : Etcx.HEIGHT_HEADER_OFFSET + 4])
		# ETC1
		f.write(etc1_data)
		# ETC1 alpha
		f.write(etc1a_data)
		# done
		f.close()
		os.remove(etc1)
		os.remove(etc1a)
		return "File '%s' has been successfully merged." % etcx
