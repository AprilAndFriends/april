	ETCX format (ETC1-with-eXtra-alpha)

ETCX is a format that uses two ETC1 files to emulate ETC1 with an alpha channel. Both images are
contained within the ETCX header container.


	Format specification:

20 bytes			HEADER
X bytes				IMAGE DEFINITION (either 2 raw ETC1 images or zlib-compressed data that contains 2 raw ETC1 images)

Header:
4 bytes			"ETCX"
4 bytes			flags
4 bytes			width
4 bytes			height
4 bytes			data size in bytes
4 bytes			compressed data size in bytes

Supported flags:
bit 1 - has alpha channel
bit 2 - data is zlib-compressed

	ETCX Tool

ETCX Tool is a tool for simple merging of an ETC1 RGB and an ETC1 alpha file. The tool requires
the modified version of etcpak. The script includes all documentation inside. It can be called
using "etcx-tool.py -h".
