#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <string>
#include <vector>
#include <unordered_map>

struct HuffmanNode {
    char data;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char d, int f);
    HuffmanNode(int f);
};

struct LZ77Match {
    int offset;
    int length;
    char nextChar;

    LZ77Match(int o, int l, char c);
};

class TextCompressor {
private:
    std::unordered_map<char, std::string> huffmanCodes;
    HuffmanNode* huffmanRoot;

    size_t originalSize;
    size_t compressedSize;

    void deleteHuffmanTree(HuffmanNode* node);
    void buildHuffmanTree(const std::string& text);
    void generateHuffmanCodes(HuffmanNode* node, std::string code);
    std::string huffmanEncode(const std::string& text);
    std::string huffmanDecode(const std::string& encoded);
    std::vector<LZ77Match> lz77Compress(const std::string& text);
    std::string lz77Decompress(const std::vector<LZ77Match>& matches);
    std::vector<unsigned char> binaryStringToBytes(const std::string& binary);
    std::string bytesToBinaryString(const std::vector<unsigned char>& bytes, int validBits);

public:
    TextCompressor();
    ~TextCompressor();

    bool compressFile(const std::string& inputFile, const std::string& outputFile);
    bool decompressFile(const std::string& inputFile, const std::string& outputFile);
    std::string getStatisticsString();
};

#endif // ADD GETSTAT METHOD (26/05)