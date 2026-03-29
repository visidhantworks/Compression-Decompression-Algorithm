#include "compressor.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <bitset>
#include <algorithm>
#include <iomanip>

using namespace std;

// Constructor implementations
HuffmanNode::HuffmanNode(char d, int f) : data(d), freq(f), left(nullptr), right(nullptr) {}
HuffmanNode::HuffmanNode(int f) : data(0), freq(f), left(nullptr), right(nullptr) {}

LZ77Match::LZ77Match(int o, int l, char c) : offset(o), length(l), nextChar(c) {}

// Comparator for priority queue
struct Compare {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->freq > b->freq;
    }
};

TextCompressor::TextCompressor() : huffmanRoot(nullptr), originalSize(0), compressedSize(0) {}

TextCompressor::~TextCompressor() {
    deleteHuffmanTree(huffmanRoot);
}

void TextCompressor::deleteHuffmanTree(HuffmanNode* node) {
    if (node) {
        deleteHuffmanTree(node->left);
        deleteHuffmanTree(node->right);
        delete node;
    }
}

// Build Huffman Tree
void TextCompressor::buildHuffmanTree(const string& text) {
    unordered_map<char, int> freq;
    for (char c : text) {
        freq[c]++;
    }

    priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> pq;

    for (auto& p : freq) {
        pq.push(new HuffmanNode(p.first, p.second));
    }

    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();

        HuffmanNode* merged = new HuffmanNode(left->freq + right->freq);
        merged->left = left;
        merged->right = right;

        pq.push(merged);
    }

    huffmanRoot = pq.top();
}

// Generate Huffman codes
void TextCompressor::generateHuffmanCodes(HuffmanNode* node, string code) {
    if (!node) return;

    if (!node->left && !node->right) {
        huffmanCodes[node->data] = code.empty() ? "0" : code;
        return;
    }

    generateHuffmanCodes(node->left, code + "0");
    generateHuffmanCodes(node->right, code + "1");
}

// LZ77 Compression
vector<LZ77Match> TextCompressor::lz77Compress(const string& text) {
    vector<LZ77Match> matches;
    int windowSize = 4096;
    int bufferSize = 18;

    for (int i = 0; i < (int)text.length(); ) {
        int bestOffset = 0;
        int bestLength = 0;

        int searchStart = max(0, i - windowSize);
        int searchEnd = i;

        // Find the longest match
        for (int j = searchStart; j < searchEnd; j++) {
            int length = 0;
            while (j + length < searchEnd &&
                i + length < (int)text.length() &&
                text[j + length] == text[i + length] &&
                length < bufferSize) {
                length++;
            }

            if (length > bestLength) {
                bestLength = length;
                bestOffset = i - j;
            }
        }

        char nextChar = (i + bestLength < (int)text.length()) ? text[i + bestLength] : '\0';
        matches.push_back(LZ77Match(bestOffset, bestLength, nextChar));
        i += bestLength + 1;
    }

    return matches;
}

// LZ77 Decompression
string TextCompressor::lz77Decompress(const vector<LZ77Match>& matches) {
    string result;

    for (const auto& match : matches) {
        if (match.length > 0) {
            int start = (int)result.length() - match.offset;
            for (int i = 0; i < match.length; i++) {
                result += result[start + i];
            }
        }
        if (match.nextChar != '\0') {
            result += match.nextChar;
        }
    }

    return result;
}

// Huffman Encoding
string TextCompressor::huffmanEncode(const string& text) {
    string encoded;
    for (char c : text) {
        encoded += huffmanCodes[c];
    }
    return encoded;
}

// Huffman Decoding
string TextCompressor::huffmanDecode(const string& encoded) {
    string decoded;
    HuffmanNode* current = huffmanRoot;

    for (char bit : encoded) {
        if (bit == '0') {
            current = current->left;
        }
        else {
            current = current->right;
        }

        if (!current->left && !current->right) {
            decoded += current->data;
            current = huffmanRoot;
        }
    }

    return decoded;
}

// Convert binary string to bytes
vector<unsigned char> TextCompressor::binaryStringToBytes(const string& binary) {
    vector<unsigned char> bytes;
    string paddedBinary = binary;

    // Pad to make it divisible by 8
    while (paddedBinary.length() % 8 != 0) {
        paddedBinary += "0";
    }

    for (size_t i = 0; i < paddedBinary.length(); i += 8) {
        string byte = paddedBinary.substr(i, 8);
        unsigned char byteValue = 0;
        for (int j = 0; j < 8; j++) {
            if (byte[j] == '1') {
                byteValue |= (1 << (7 - j));
            }
        }
        bytes.push_back(byteValue);
    }

    return bytes;
}

// Convert bytes to binary string
string TextCompressor::bytesToBinaryString(const vector<unsigned char>& bytes, int validBits) {
    string binary;
    for (size_t i = 0; i < bytes.size(); i++) {
        bitset<8> bits(bytes[i]);
        binary += bits.to_string();
    }

    // Trim to valid bits
    if (validBits > 0 && validBits < (int)binary.length()) {
        binary = binary.substr(0, validBits);
    }

    return binary;
}

// Main compression function
bool TextCompressor::compressFile(const string& inputFile, const string& outputFile) {
    ifstream file(inputFile);
    if (!file.is_open()) {
        cout << "Error: Could not open input file!" << endl;
        return false;
    }

    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    if (text.empty()) {
        cout << "Error: Input file is empty!" << endl;
        return false;
    }

    originalSize = text.length();

    // Step 1: LZ77 Compression
    vector<LZ77Match> lz77Matches = lz77Compress(text);

    // Convert LZ77 matches to string for Huffman coding
    string lz77String;
    for (const auto& match : lz77Matches) {
        lz77String += to_string(match.offset) + "," + to_string(match.length) + "," + match.nextChar + ";";
    }

    // Step 2: Huffman Coding
    buildHuffmanTree(lz77String);
    generateHuffmanCodes(huffmanRoot, "");
    string huffmanEncoded = huffmanEncode(lz77String);

    // Convert to bytes
    vector<unsigned char> compressedBytes = binaryStringToBytes(huffmanEncoded);
    compressedSize = compressedBytes.size();

    // Save compressed data
    ofstream outFile(outputFile, ios::binary);
    if (!outFile.is_open()) {
        cout << "Error: Could not create output file!" << endl;
        return false;
    }

    // Write metadata
    int validBits = (int)huffmanEncoded.length();
    outFile.write(reinterpret_cast<const char*>(&validBits), sizeof(validBits));

    // Write Huffman codes
    int codeCount = (int)huffmanCodes.size();
    outFile.write(reinterpret_cast<const char*>(&codeCount), sizeof(codeCount));
    for (const auto& pair : huffmanCodes) {
        outFile.write(&pair.first, sizeof(char));
        int codeLength = (int)pair.second.length();
        outFile.write(reinterpret_cast<const char*>(&codeLength), sizeof(codeLength));
        outFile.write(pair.second.c_str(), codeLength);
    }

    // Write compressed data
    int dataSize = (int)compressedBytes.size();
    outFile.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
    outFile.write(reinterpret_cast<const char*>(compressedBytes.data()), compressedBytes.size());

    outFile.close();

    // Update compressed size to include metadata
    compressedSize = sizeof(validBits) + sizeof(codeCount);
    for (const auto& pair : huffmanCodes) {
        compressedSize += sizeof(char) + sizeof(int) + pair.second.length();
    }
    compressedSize += sizeof(dataSize) + compressedBytes.size();

    return true;
}

// Main decompression function
bool TextCompressor::decompressFile(const string& inputFile, const string& outputFile) {
    ifstream file(inputFile, ios::binary);
    if (!file.is_open()) {
        cout << "Error: Could not open compressed file!" << endl;
        return false;
    }

    // Read metadata
    int validBits;
    file.read(reinterpret_cast<char*>(&validBits), sizeof(validBits));

    // Read Huffman codes
    int codeCount;
    file.read(reinterpret_cast<char*>(&codeCount), sizeof(codeCount));
    huffmanCodes.clear();

    for (int i = 0; i < codeCount; i++) {
        char c;
        file.read(&c, sizeof(char));
        int length;
        file.read(reinterpret_cast<char*>(&length), sizeof(length));
        string code(length, ' ');
        file.read(&code[0], length);
        huffmanCodes[c] = code;
    }

    // Read compressed data
    int dataSize;
    file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
    vector<unsigned char> compressedBytes(dataSize);
    file.read(reinterpret_cast<char*>(compressedBytes.data()), dataSize);

    file.close();

    // Rebuild Huffman tree from codes
    deleteHuffmanTree(huffmanRoot);
    huffmanRoot = new HuffmanNode(0);
    for (auto& pair : huffmanCodes) {
        HuffmanNode* current = huffmanRoot;
        for (char bit : pair.second) {
            if (bit == '0') {
                if (!current->left) current->left = new HuffmanNode(0);
                current = current->left;
            }
            else {
                if (!current->right) current->right = new HuffmanNode(0);
                current = current->right;
            }
        }
        current->data = pair.first;
    }

    // Decode Huffman encoded string
    string binaryStr = bytesToBinaryString(compressedBytes, validBits);
    string decodedString = huffmanDecode(binaryStr);

    // Parse LZ77 matches from decoded string
    vector<LZ77Match> matches;
    stringstream ss(decodedString);
    string token;

    while (getline(ss, token, ';')) {
        if (token.empty()) continue;
        int pos1 = token.find(',');
        int pos2 = token.find(',', pos1 + 1);
        if (pos1 == string::npos || pos2 == string::npos) continue;

        int offset = stoi(token.substr(0, pos1));
        int length = stoi(token.substr(pos1 + 1, pos2 - pos1 - 1));
        char nextChar = token[pos2 + 1];

        matches.push_back(LZ77Match(offset, length, nextChar));
    }

    // Decompress LZ77
    string decompressedText = lz77Decompress(matches);

    // Save decompressed text
    ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        cout << "Error: Could not create output file!" << endl;
        return false;
    }

    outFile << decompressedText;
    outFile.close();

    return true;
}

string TextCompressor::getStatisticsString() {
    if (originalSize == 0) return "No data compressed yet.";

    double ratio = (double)compressedSize / originalSize;
    double percentage = ratio * 100;

    stringstream ss;
    ss << fixed << setprecision(2);
    ss << "Original size: " << originalSize << " bytes\n";
    ss << "Compressed size: " << compressedSize << " bytes\n";
    ss << "Compression ratio: " << ratio << " (" << percentage << "%)\n";

    return ss.str();
}
