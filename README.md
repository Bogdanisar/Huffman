# Huffman
Tool written in C++ which can be used to compress and decompress a text-file using huffman codes.
The file must contain only ASCII characters.

Navigate to containing folder in CMD and run:
g++ -std=c++11 huffman.cpp -o huffman.exe

Usage:
Command: huffman.exe <operation type> <if> <of>

- operation type = 1 if compression is desired.
<if> - full name of a local .txt file that is to be compressed.
<of> - full name of a local .huf file to store output.
E.g.: huffman.exe 1 localInput.txt localOutput.huf

- operation type = 2 if decompression is desired.
<if> - full name of a local .huf file that is to be decompressed.
<of> - full name of a local .txt file to store output.
E.g.: huffman.exe 1 localInput.huf localOutput.txt
