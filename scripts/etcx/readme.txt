	ETCX format (ETC1-with-eXtra-alpha)

ETCX is a format that uses two ETC1 files to emulate ETC1 with an alpha channel. Both images are
contained within the ETCX header container.


	Format specification:

16 bytes			HEADER
4 bytes + X bytes	IMAGE DEFINITION (JPEG, RGB of the image)
4 bytes + X bytes	IMAGE DEFINITION (PNG, alpha of the image)

Header:
4 bytes			"ETCX"
4 bytes			flags
4 bytes			width
4 bytes			height

Supported flags:
bit 1 - has alpha channel

	ETCX Tool

ETCX Tool is a tool for simple merging of an ETC1 RGB and an ETC1 alpha file. The tool requires
the modified version of etcpak. The script includes all documentation inside. It can be called
using "etcx-tool.py -h".
