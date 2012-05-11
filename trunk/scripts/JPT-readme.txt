	JPT format (JPEG and PNG tarred)

JPT is a format that takes advantage of the compression ratio from JPEG and PNG at the same time.
The main image is stored in JPEG while the alpha channel is stored in a PNG file as a gray scale.
While the PNG file can be 32 bit, 24 bit, 8 bit or even palette based, it is recommended to use
only 8 bit or palette based images in order to save space and actually take advantage of the JPT
format.

Note:
While the gray scale is used for the alpha channel (white is opaque, black is transparent),
during the calculation of the final image actually only takes into account the red channel to
improve performance.

	Format specification:

4 bytes			HEADER
4 bytes + X bytes	IMAGE DEFINITION (JPEG, RGB of the image)
4 bytes + X bytes	IMAGE DEFINITION (PNG, alpha of the image)

Header:
3 bytes			"JPT"
1 byte			format version

Image defintion:
4 bytes			following image's size in bytes
X bytes			raw image file appended to the JPT file (JPEG of PNG)
