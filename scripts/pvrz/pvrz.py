import os
import struct
import sys
import zlib

UINT_PACK_FORMAT = "<I" # little-endian unsigned int

def _read_file(filename):
	f = open(filename, "rb")
	header = f.read(Pvrz.HEADER_SIZE)
	data = f.read()
	f.close()
	return [header, data]

class Pvrz:
	
	Version = 1
	
	HEADER_SIZE = 13 * 4
	WIDTH_HEADER_OFFSET = 1 * 4
	HEIGHT_HEADER_OFFSET = 2 * 4
	
	@staticmethod
	def create(pvrz, pvr, zlibCompressionLevel = 6):
		if not os.path.exists(pvr):
			return "ERROR! File '%s' does not exist!" % pvr
		if zlibCompressionLevel < 1:
			zlibCompressionLevel = 1
		elif zlibCompressionLevel > 9:
			zlibCompressionLevel = 9
		flags = 0x0
		header, pvr_data = _read_file(pvr)
		f = open(pvrz, "wb")
		# header
		f.write("PVRZ")
		f.write(struct.pack(UINT_PACK_FORMAT, flags))
		f.write(header[Pvrz.WIDTH_HEADER_OFFSET : Pvrz.WIDTH_HEADER_OFFSET + 4])
		f.write(header[Pvrz.HEIGHT_HEADER_OFFSET : Pvrz.HEIGHT_HEADER_OFFSET + 4])
		pvr_data = header + pvr_data # merge back together with header
		f.write(struct.pack(UINT_PACK_FORMAT, len(pvr_data)))
		data = zlib.compress(pvr_data, zlibCompressionLevel)
		f.write(struct.pack(UINT_PACK_FORMAT, len(data)))
		f.write(data)
		# done
		f.close()
		os.remove(pvr)
		return "File '%s' has been successfully created." % pvrz
