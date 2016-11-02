import os
import struct
import sys
import zlib

UINT_PACK_FORMAT = "<I" # little-endian unsigned int

def _read_file(filename):
	f = open(filename, "rb")
	header = f.read(Etcx.HEADER_SIZE)
	data = f.read()
	f.close()
	return [header, data]

class Etcx:
	
	Version = 1
	HEADER_HAS_ALPHA_BIT = 0x1
	HEADER_IS_ZLIB_COMPRESSED_BIT = 0x2
	
	HEADER_SIZE = 13 * 4
	# etcpak has the w/h order reversed for some reason
	WIDTH_HEADER_OFFSET = 7 * 4
	HEIGHT_HEADER_OFFSET = 6 * 4
	
	@staticmethod
	def create(etcx, etc1, etc1a, zlibCompressionLevel = 6):
		if not os.path.exists(etc1):
			return "ERROR! File '%s' does not exist!" % etc1
		if not os.path.exists(etc1a):
			etc1a = None
		if zlibCompressionLevel < 0:
			zlibCompressionLevel = 0
		elif zlibCompressionLevel > 9:
			zlibCompressionLevel = 9
		flags = 0x0
		header, etc1_data = _read_file(etc1)
		if etc1a != None:
			flags = Etcx.HEADER_HAS_ALPHA_BIT
			dummy_header, etc1a_data = _read_file(etc1a)
		f = open(etcx, "wb")
		# header
		f.write("ETCX")
		if zlibCompressionLevel > 0:
			flags |= Etcx.HEADER_IS_ZLIB_COMPRESSED_BIT
		f.write(struct.pack(UINT_PACK_FORMAT, flags))
		f.write(header[Etcx.WIDTH_HEADER_OFFSET : Etcx.WIDTH_HEADER_OFFSET + 4])
		f.write(header[Etcx.HEIGHT_HEADER_OFFSET : Etcx.HEIGHT_HEADER_OFFSET + 4])
		size = len(etc1_data)
		if etc1a != None:
			size += len(etc1a_data)
		f.write(struct.pack(UINT_PACK_FORMAT, size))
		if zlibCompressionLevel > 0:
			if etc1a != None:
				data = zlib.compress(etc1_data + etc1a_data, zlibCompressionLevel)
			else:
				data = zlib.compress(etc1_data, zlibCompressionLevel)
			f.write(struct.pack(UINT_PACK_FORMAT, len(data)))
			f.write(data)
		else:
			f.write(struct.pack(UINT_PACK_FORMAT, size))
			# ETC1
			f.write(etc1_data)
			# ETC1 alpha
			if etc1a != None:
				f.write(etc1a_data)
		# done
		f.close()
		os.remove(etc1)
		if etc1a != None:
			os.remove(etc1a)
		return "File '%s' has been successfully merged." % etcx
