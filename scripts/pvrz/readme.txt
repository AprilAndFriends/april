	PVRZ format (PVR-Zlib-compressed)

PVRZ is a format that contains a zlib-compressed PVR file.


	Format specification:

20 bytes			HEADER
X bytes				IMAGE DEFINITION (zlib-compressed PVR image)

Header:
4 bytes			"PVRZ"
4 bytes			reserved
4 bytes			width
4 bytes			height
4 bytes			data size in bytes
4 bytes			compressed data size in bytes

	PVRZ Tool

PVRZ Tool is a tool for creating PVRZ files from other image files. First the image is converted to PVR
using PVRTexToolCLI.exe and then it is zlib-compressed. The script includes all documentation inside.
It can be called using "pvrz-tool.py -h".
