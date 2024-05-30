#include <string>
#include <iostream>
#include <algorithm>
#include "Tools.h"

using namespace std;
using namespace tools::compression;
using namespace tools::compression::huffman;

const static unsigned char BFINAL_FIELD = 1;
const static unsigned char BTYPE_FIELD = 2;
const static unsigned char HLIT_FIELD = 5;
const static unsigned char HDIST_FIELD = 5;
const static unsigned char HCLEN_FIELD = 4;

const static unsigned char NO_COMPRESSION = 0x00;
const static unsigned char FIXED_COMPRESSION = 0x01;
const static unsigned char DYNAMIC_COMPRESSION = 0x02;

struct FrequencyEntry {
    unsigned char frq = 0;
    unsigned short next_code;
};

unsigned int readBit(BitStream& data) {
    if(data.mask == 0x00) {
        data.mask = 0x01;
        data.buf = data.stream[data.inx++];
    }
    unsigned int bit = data.buf & data.mask ? 1 : 0;
    data.mask <<= 1;

    return bit;
}

unsigned int readField(BitStream& data, unsigned char len) {
    unsigned int field = 0;

    for(int i = 0; i < len; i++)
        field |= (readBit(data) << i);

    return field;
}

const int EXTRA_LENGTH[] = {11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227};

const int EXTRA_DISTANCE[] = {4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512, 768, 1024, 1536, 2048, 3072, 4096, 6144, 8192, 12288, 16384, 24576};

bool compareSymbols(const CodeLengthEntry& x, const CodeLengthEntry& y) {
    return (int) x.symbol < (int) y.symbol;
}

bool compareLengths(const CodeLengthEntry& x, const CodeLengthEntry& y) {
    return (int) x.len < (int) y.len;
}

Huffman::Huffman() {
}

vector <Node> Huffman::createCodes(vector <CodeLengthEntry>& aspTable) {
    sort(aspTable.begin(), aspTable.end(), compareSymbols);
    unsigned short MaxLen = max_element(aspTable.begin(), aspTable.end(), compareLengths)->len;

    vector <FrequencyEntry> aslFrequencyTable(MaxLen + 1);

    for(int i = 0; i < aspTable.size(); i++)
        aslFrequencyTable[aspTable[i].len].frq++;

    unsigned int ilCode = 0;
    aslFrequencyTable[0].frq = 0;
    for(int i = 1; i < aslFrequencyTable.size(); i++) {
        ilCode = (ilCode + aslFrequencyTable[i - 1].frq) << 1;
        aslFrequencyTable[i].next_code = ilCode;
    }

    vector <Node> slTree({Node()});
    for(int i = 0; i < aspTable.size(); i++)
        if(aspTable[i].len != 0) {
            unsigned int ilTmpCode = aslFrequencyTable[aspTable[i].len].next_code;
            aslFrequencyTable[aspTable[i].len].next_code++;

            ilTmpCode = ilTmpCode << (16 - aspTable[i].len);
            size_t slActualNode = 0;
            for(int j = 0; j < aspTable[i].len; j++) {
                if((ilTmpCode & 0x8000) != 0) {
                    if(slTree[slActualNode].one == 0) {
                        slTree.push_back(Node());
                        slTree[slActualNode].one = slTree.size() - 1;
                    }
                    slActualNode = slTree[slActualNode].one;
                } else {
                    if(slTree[slActualNode].zero == 0) {
                        slTree.push_back(Node());
                        slTree[slActualNode].zero = slTree.size() - 1;
                    }
                    slActualNode = slTree[slActualNode].zero;
                }
                ilTmpCode <<= 1;
            }
            slTree[slActualNode].symbol = aspTable[i].symbol;
        }

    return slTree;
}

void Huffman::extractLengthTable(vector <Node>& aHCLENTree, BitStream& spStream, vector <CodeLengthEntry>& slLengthTable) {
    size_t alActualNode = 0;
    for(int i = 0; i < slLengthTable.size();) {
        alActualNode = readBit(spStream) ? aHCLENTree[alActualNode].one : aHCLENTree[alActualNode].zero;
        if(alActualNode == 0)
            throw string("Bad Code Lengths Tree.");
        if(aHCLENTree[alActualNode].one == 0 && aHCLENTree[alActualNode].zero == 0) {
            if(aHCLENTree[alActualNode].symbol > 15) {
                int ilRepeat;
                switch(aHCLENTree[alActualNode].symbol) {
                    case 16:
                        ilRepeat = readField(spStream, 2) + 3;
                        break;
                    case 17:
                        ilRepeat = readField(spStream, 3) + 3;
                        break;
                    case 18:
                        ilRepeat = readField(spStream, 7) + 11;
                }
                while(ilRepeat--) {
                    if(aHCLENTree[alActualNode].symbol != 16)
                        slLengthTable[i].len = 0;
                    else
                        slLengthTable[i].len = slLengthTable[i - 1].len;
                    i++;
                }
            } else
                slLengthTable[i++].len = aHCLENTree[alActualNode].symbol;
            alActualNode = 0;
        }
    }
}

ZLIB::ZLIB() {
}

shared_ptr <vector <unsigned char>> ZLIB::inflate(vector <unsigned char>& apData) {
    shared_ptr <vector <unsigned char>> alDstData = make_shared <vector <unsigned char>>();
    BitStream slStream;
    vector <Node> alFCTree;
    vector <Node> alSCTree;

    slStream.stream = apData;
    slStream.inx = 2;
    slStream.mask = 0x00;

    while(slStream.inx < slStream.stream.size() - 4) {
        unsigned int ilBlockType = readField(slStream, BFINAL_FIELD);
        unsigned int ilCompressionType = readField(slStream, BTYPE_FIELD);

        if(ilCompressionType == NO_COMPRESSION) { // Uncompressed block.
            readField(slStream, 5); // When there is not compression the rest of the byte is discarded.
            struct BlockSize {
                unsigned short size;
                unsigned short csize;
            } slSize;
            slSize.size = readField(slStream, sizeof(unsigned short) * 8);
            slSize.csize = readField(slStream, sizeof(unsigned short) * 8);
            if(slSize.size == 0)
                break;
            for(size_t igActualInx = 0; igActualInx < slSize.size; igActualInx++)
                alDstData->push_back(readField(slStream, 8));
        } else if(ilCompressionType == FIXED_COMPRESSION || ilCompressionType == DYNAMIC_COMPRESSION) { // Block with compression based on hoffman tables.
            if(ilCompressionType == DYNAMIC_COMPRESSION) { // Hoffman tables are dynamic, they must be built.
                unsigned int ilHLIT = readField(slStream, HLIT_FIELD) + 257;
                unsigned int ilHDIST = readField(slStream, HDIST_FIELD) + 1;
                unsigned int ilHCLEN = readField(slStream, HCLEN_FIELD) + 4;

                vector <CodeLengthEntry> slLenTable(19);
                vector <CodeLengthEntry> slLLTable(ilHLIT);
                vector <CodeLengthEntry> slDistTable(ilHDIST);

                slLenTable[0].symbol = 16;
                slLenTable[1].symbol = 17;
                slLenTable[2].symbol = 18;
                slLenTable[3].symbol = 0;
                slLenTable[4].symbol = 8;
                slLenTable[5].symbol = 7;
                slLenTable[6].symbol = 9;
                slLenTable[7].symbol = 6;
                slLenTable[8].symbol = 10;
                slLenTable[9].symbol = 5;
                slLenTable[10].symbol = 11;
                slLenTable[11].symbol = 4;
                slLenTable[12].symbol = 12;
                slLenTable[13].symbol = 3;
                slLenTable[14].symbol = 13;
                slLenTable[15].symbol = 2;
                slLenTable[16].symbol = 14;
                slLenTable[17].symbol = 1;
                slLenTable[18].symbol = 15;

                for(unsigned short i = 0; i < ilHLIT; i++)
                    slLLTable[i].symbol = i;

                for(unsigned short i = 0; i < ilHDIST; i++)
                    slDistTable[i].symbol = i;

                for(unsigned int i = 0; i < ilHCLEN; i++)
                    slLenTable[i].len = readField(slStream, 3);

                vector <Node> alTree = Huffman::createCodes(slLenTable);

                Huffman::extractLengthTable(alTree, slStream, slLLTable);
                Huffman::extractLengthTable(alTree, slStream, slDistTable);

                alFCTree = Huffman::createCodes(slLLTable);
                alSCTree = Huffman::createCodes(slDistTable);
            } else { // Hoffman tables are defined by fixed longitudes as defined by the reference document RFC1951.
                vector <CodeLengthEntry> slLLTable(288); // It is built with all the codes for the table of literals/longitudes.
                vector <CodeLengthEntry> slDistTable(32); // It is built with all the codes for the shifts table.

                for(int i = 0; i < slLLTable.size(); i++) {
                    slLLTable[i].symbol = i;
                    if(i <= 143)
                        slLLTable[i].len = 8;
                    else if(i <= 255)
                        slLLTable[i].len = 9;
                    else if(i <= 279)
                        slLLTable[i].len = 7;
                    else
                        slLLTable[i].len = 8;
                }

                for(int i = 0; i < slDistTable.size(); i++) {
                    slDistTable[i].symbol = i;
                    slDistTable[i].len = 5;
                }

                alFCTree = Huffman::createCodes(slLLTable);
                alSCTree = Huffman::createCodes(slDistTable);
            }

            size_t alActualFCNode = 0;
            while(true) {
                alActualFCNode = readBit(slStream) ? alFCTree[alActualFCNode].one : alFCTree[alActualFCNode].zero;
                if(alActualFCNode == 0)
                    throw string("ZLIB::inflate: Bad Literal/Length Tree.");
                if(alFCTree[alActualFCNode].one == 0 && alFCTree[alActualFCNode].zero == 0) {
                    if(alFCTree[alActualFCNode].symbol < 256)
                        alDstData->push_back((unsigned char) alFCTree[alActualFCNode].symbol);
                    else if(alFCTree[alActualFCNode].symbol == 256)
                        break;
                    else if(alFCTree[alActualFCNode].symbol > 256) {
                        int ilLength;
                        if(alFCTree[alActualFCNode].symbol < 265)
                            ilLength = alFCTree[alActualFCNode].symbol - 254;
                        else if(alFCTree[alActualFCNode].symbol < 285)
                            ilLength = readField(slStream, (alFCTree[alActualFCNode].symbol - 261) / 4) + EXTRA_LENGTH[alFCTree[alActualFCNode].symbol - 265];
                        else
                            ilLength = 258;

                        int ilDistance;
                        size_t alActualSCNode = 0;
                        while(alSCTree[alActualSCNode].one != 0 && alSCTree[alActualSCNode].zero != 0) {
                            alActualSCNode = readBit(slStream) ? alSCTree[alActualSCNode].one : alSCTree[alActualSCNode].zero;
                            if(alActualSCNode == 0)
                                throw string("ZLIB::inflate: Bad Distance Tree.");
                        }
                        ilDistance = alSCTree[alActualSCNode].symbol;
                        if(ilDistance > 3)
                            ilDistance = readField(slStream, (ilDistance - 2) / 2) + EXTRA_DISTANCE[ilDistance - 4];
                        
                        size_t ilActualInx = alDstData->size() - ilDistance - 1;
                        while(ilLength--)
                            alDstData->push_back(alDstData->at(ilActualInx++));
                    }
                    alActualFCNode = 0;
                }
            }
        } else
            throw string("ZLIB::inflate: Type of Compression is reserved.");
        if(ilBlockType != 0) break; // Final block.
    }

    return alDstData;
}