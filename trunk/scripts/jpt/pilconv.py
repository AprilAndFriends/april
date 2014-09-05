import os
import sys

try:
	import Image
except:
	print "ERROR! Please install PIL to use this script."
	print "http://www.pythonware.com/products/pil"
	sys.exit()

class PilConv:
	
	Version = 1
	
	@staticmethod
	def convert(origin, jpeg, png, quality):
		if not os.path.exists(origin):
			return "ERROR! File '%s' does not exist!" % origin
		origin_image = Image.open(origin) # opens image
		origin_image.save(jpeg, quality = quality) # saves JPEG
		if "A" in origin_image.getbands():
			alpha = origin_image.split()[-1] # gets alpha channel
			if "icc_profile" in alpha.info:
				alpha.info.pop("icc_profile")
			png_image = alpha
		else:
			png_image = Image.new("L", origin_image.size, 255) # creates opaque PNG alpha image
		png_image.save(png) # saves PNG
		return "File '%s' has been successfully converted." % origin
