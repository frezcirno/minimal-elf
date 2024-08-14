#!/usr/bin/env python3
import struct
import sys
from collections import namedtuple
from typing import BinaryIO, List, Tuple, cast

SHT_STRTAB = 0x03


def binary_open(filename: str, mode: str) -> BinaryIO:
    return cast(BinaryIO, open(filename, mode + "b"))


def get_input_output_files() -> Tuple[BinaryIO, BinaryIO]:
    inputfilepath = "/dev/stdin"
    outputfilepath = "/dev/stdout"

    argc = len(sys.argv)

    if argc > 1:
        inputfilepath = sys.argv[1]
        outputfilepath = inputfilepath
    if argc > 2:
        outputfilepath = sys.argv[2]

    if outputfilepath == inputfilepath:
        inputfile = binary_open(inputfilepath, "r+")
        outputfile = inputfile
    else:
        inputfile = binary_open(inputfilepath, "r")
        outputfile = binary_open(outputfilepath, "w")

    return inputfile, outputfile


class BetterStruct:
    def __init__(self, fields: List[Tuple[str, str]], little_endian: bool) -> None:
        struct_fmt = ""
        struct_fields = ""

        if little_endian:
            struct_fmt += "<"
        else:
            struct_fmt += ">"

        for fmt, name in fields:
            struct_fmt += fmt + " "
            if name is not None:
                struct_fields += name + " "

        self._struct = struct.Struct(struct_fmt)
        self._tu_type = namedtuple("my_tuple", struct_fields)
        self.size = self._struct.size

    def unpack(self, data: bytes) -> None:
        self.fields = self._tu_type._make(self._struct.unpack(data[: self.size]))

    def unpack_from(self, f: BinaryIO) -> None:
        self.fields = self._tu_type._make(self._struct.unpack(f.read(self.size)))

    def pack(self) -> bytes:
        return self._struct.pack(*self.fields)


if __name__ == "__main__":
    inputfile, outputfile = get_input_output_files()

    ehdr_ident = BetterStruct(
        [
            ("4s", "magic"),  # e_ident[:4] ELF magic number
            ("b", "arch"),  # e_ident[4] File class, 1=32-bit, 2=64-bit
            ("b", "endianness"),  # e_ident[5] Data encoding, 1=little, 2=big
            ("b", "version"),  # e_ident[6] ELF header version
            ("b", "abi"),  # e_ident[7] OS/ABI identification, 3=Linux
            ("b", "abi_version"),  # e_ident[8] ABI version, unused
            ("7x", None),  # e_ident[9:16] Padding
        ],
        True,
    )
    ehdr_ident_bytes = inputfile.read(ehdr_ident.size)
    ehdr_ident.unpack(ehdr_ident_bytes)

    if ehdr_ident.fields.magic != b"\x7fELF":
        print("Input is not an ELF file.", file=sys.stderr)
        exit(1)

    if ehdr_ident.fields.arch != 2:
        print("Only 64-bit ELF files are currently supported.", file=sys.stderr)
        exit(1)

    # Read the rest
    file_contents = ehdr_ident_bytes + inputfile.read(-1)

    ehdr_noident = BetterStruct(
        [
            ("H", "type"),
            ("H", "machine"),
            ("L", "version"),
            ("Q", "entry"),
            ("Q", "phoff"),
            ("Q", "shoff"),
            ("L", "flags"),
            ("H", "ehsize"),
            ("H", "phentsize"),
            ("H", "phnum"),
            ("H", "shentsize"),
            ("H", "shnum"),
            ("H", "shstrndx"),
        ],
        ehdr_ident.fields.endianness == 1,
    )
    ehdr_noident.unpack(file_contents[ehdr_ident.size :])

    shoff = ehdr_noident.fields.shoff
    shentsize = ehdr_noident.fields.shentsize
    shnum = ehdr_noident.fields.shnum
    shstrndx = ehdr_noident.fields.shstrndx

    end_of_shdr = shoff + shentsize * shnum
    if end_of_shdr != len(file_contents):
        print("Trailing bytes after end of section header.", file=sys.stderr)
        exit(1)

    # Strip section header table
    ehdr_noident.fields._replace(shoff=0, shentsize=0, shnum=0, shstrndx=0)

    file_contents = (
        file_contents[: ehdr_ident.size]
        + ehdr_noident.pack()
        + file_contents[ehdr_ident.size + ehdr_noident.size :]
    )

    shdr_shstr = BetterStruct(
        [
            ("L", "name"),
            ("L", "type"),
            ("Q", "flags"),
            ("Q", "addr"),
            ("Q", "offset"),
            ("Q", "size"),
            ("L", "link"),
            ("L", "info"),
            ("Q", "addralign"),
            ("Q", "entsize"),
        ],
        ehdr_ident.fields.endianness == 1,
    )
    shdr_shstr.unpack(file_contents[shoff + shentsize * shstrndx :])

    if shdr_shstr.fields.type != SHT_STRTAB:
        print("Can't find section header's string table.", file=sys.stderr)
        exit(1)

    end_of_strtable = shdr_shstr.fields.offset + shdr_shstr.fields.size
    if abs(shoff - end_of_strtable) > 8:
        print(
            "Section header and its string table are not roughly contiguous.",
            file=sys.stderr,
        )
        exit(1)

    outputfile.seek(0)
    outputfile.write(file_contents[: shdr_shstr.fields.offset])
