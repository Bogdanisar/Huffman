#include <iostream>
#include <fstream>
#include <stdio.h>
#include <queue>
#include <windows.h>

using namespace std;

typedef long long ll;
typedef unsigned long long ull;
const int characterMax = 256 + 5;
const int nodeMax = characterMax * 2 + 50;

enum opTypeEnum {
    COMPRESSION = 1,
    DECOMPRESSION
};

struct nodeType {
    ull occ;
    char ch;
    int st,dr;
    nodeType(ull _occ = 0,char _ch = '\0',int _st = 0,int _dr = 0) {
        occ = _occ;
        ch = _ch;
        st = _st;
        dr = _dr;
    }
};

class dataCompressionClass {
    private:
    ifstream in;
    string *code;
    ofstream *out;
    char currentCharacter;
    ll codeIndex,fileEnd;
    bool nextBit;

    public:
    dataCompressionClass(char* name,string* _code,ofstream *_out);
    ~dataCompressionClass();
    void writeData();

    private:
    bool getNext8Bit(char& ch);
    bool checkNextBit();
    char getNextBit();
};


struct trieNode {
    char ch;
    trieNode *st,*dr;
    trieNode(char _ch = 0,trieNode* _st = 0,trieNode* _dr = 0): ch(_ch), st(_st), dr(_dr) {}
};

class dataDecompressionClass {
    private:
    ifstream *in;
    ofstream out;
    const ull nrBits;
    int currentIndex;
    char current8Bit;
    trieNode * const root;
    trieNode *currentNode;
    bool fail;

    public:
    dataDecompressionClass(ifstream *_in,char* outputFileName,ull _nrBits,trieNode* _root);
    ~dataDecompressionClass();
    void writeData();
    bool hasFailed();

    private:
    char getNextBit();
    char walkTrie(char);
};

bool checkInput(int,char**,opTypeEnum&);
void printMessage(char*);
void buildFreq(char*,ull*);
void getNodes(nodeType*,int&,ull*);
void buildHuffman(nodeType*,int&,ull&);
void dfs(int,string,nodeType*,string*);
void writeI64(ull,ofstream&);
void writeI8(char,ofstream&);
void writeInfo(nodeType*,int,string*,ofstream&);
void compress(char*,char*);

void decompress(char*,char*);
ull readI64(ifstream&);
char readI8(ifstream&);
void add(trieNode*,char,const string&,int);

int main(int argc,char **argv)
{
    printf("\n");

    opTypeEnum operationType;
    if (checkInput(argc,argv,operationType) == false) {
        printMessage(argv[0]);
        return 0;
    }

    if (operationType == COMPRESSION) {
        compress(argv[2],argv[3]);
    }
    else {
        decompress(argv[2],argv[3]);
    }


    return 0;
}

bool checkInput(int argc,char** argv,opTypeEnum& operationType) {
    if (argc != 4) {
        printf("Invalid number of arguments!\n");
        return false;
    }

    if (argv[1][1] != '\0' || !(argv[1][0] == '1' || argv[1][0] == '2')) {
        printf("Invalid operation!\n");
        return false;
    }

    if (argv[1][0] == '1') {
        operationType = COMPRESSION;
    }
    else {
        operationType = DECOMPRESSION;
    }

    if (operationType == COMPRESSION) {
        int sz = strlen(argv[2]);
        if (strcmp(argv[2] + sz - 4,".txt") != 0) {
            printf("Invalid input file type!\n");
            return false;
        }

        FILE *f = fopen(argv[2],"r");

        if (f == NULL) {
            printf("Input file doesn't exist!\n");
            fclose(f);
            return false;
        }

        fseek(f,0,SEEK_END);
        int len = ftell(f);
        if (len == 0) {
            printf("Input file is empty!\n");
            fclose(f);
            return false;
        }

        fclose(f);


        sz = strlen(argv[3]);
        if (strcmp(argv[3] + sz - 4,".huf") != 0) {
            printf("Invalid output file type!\n");
            return false;
        }

        f = fopen(argv[3],"r");

        /*
        if (f != NULL) {
            printf("File already exists!\n");
            fclose(f);
            return false;
        }
        //*/
        fclose(f);
    }
    else {
        int sz = strlen(argv[2]);
        if (strcmp(argv[2] + sz - 4,".huf") != 0) {
            printf("Invalid input file type!\n");
            return false;
        }

        FILE *f = fopen(argv[2],"r");

        if (f == NULL) {
            printf("Input file doesn't exist!\n");
            fclose(f);
            return false;
        }

        fseek(f,0,SEEK_END);
        int len = ftell(f);
        if (len == 0) {
            printf("Input file is empty!\n");
            fclose(f);
            return false;
        }

        fclose(f);


        sz = strlen(argv[3]);
        if (strcmp(argv[3] + sz - 4,".txt") != 0) {
            printf("Invalid output file type!\n");
            return false;
        }

        f = fopen(argv[3],"r");

        /*
        if (f != NULL) {
            printf("File already exists!\n");
            fclose(f);
            return false;
        }
        fclose(f);
        //*/
    }

    return true;
}

void printMessage(char *name) {
    printf("Usage: %s <operation type> <if> <of>\n\n",name);
    printf("- operation type = 1 if compression is desired;\n");
    printf("<if> - full name of a local .txt file that is to be compressed.\n");
    printf("<of> - full name of a local .huf file to store output.\n");
    printf("E.g.: %s 1 localInput.txt localOutput.huf\n\n",name);
    printf("- operation type = 2 if decompression is desired;\n");
    printf("<if> - full name of a local .huf file that is to be decompressed.\n");
    printf("<of> - full name of a local .txt file to store output.\n");
    printf("E.g.: %s 1 localInput.huf localOutput.txt\n\n",name);
}

void compress(char* inputFileName,char* outputFileName) {
    ull freq[characterMax] = {};
    nodeType node[nodeMax];
    int nrNodes;
    string code[characterMax];

    buildFreq(inputFileName,freq);

    getNodes(node,nrNodes,freq);
    int lastLeafIndex = nrNodes;

    ull textLength;
    buildHuffman(node,nrNodes,textLength);
    int rootIndex = nrNodes;

    dfs(rootIndex,"",node,code);

    ofstream out(outputFileName,ios::binary);
    writeInfo(node,lastLeafIndex,code,out);

    writeI64(textLength,out);

    dataCompressionClass writerObject(inputFileName,code,&out);
    writerObject.writeData();

    printf("Compression was successful!\n");
}

void buildFreq(char* fileName,ull* freq) {
    FILE *in = fopen(fileName,"r");
    char c;

    while (true) {
        c = getc(in);
        if (feof(in) != 0) {
            break;
        }

        ++freq[c];
    }
}

void getNodes(nodeType* node,int& nrNodes,ull* freq) {
    nrNodes = 0;

    for (int i=0;i <= 127;++i) {
        if (freq[i] == 0) {
            continue;
        }

        node[++nrNodes] = nodeType(freq[i],(char)i);
    }

    if (nrNodes == 1) {
        node[++nrNodes] = nodeType(0,'\0');
    }
}

void buildHuffman(nodeType* node,int& nrNodes,ull& textLength) {
    class comp {
        private:
        nodeType *arr;

        public:
        comp(nodeType* _arr): arr(_arr) {}

        bool operator() (const int idx1,const int idx2) {
            return arr[idx1].occ > arr[idx2].occ;
        }
    };

    priority_queue< int,vector<int>,comp > pq(node);

    for (int i=1;i <= nrNodes;++i) {
        pq.push(i);
    }

    textLength = 0;
    while (pq.size() > 1) {
        int st = pq.top(); pq.pop();
        int dr = pq.top(); pq.pop();

        node[++nrNodes] = nodeType(node[st].occ + node[dr].occ,'\0',st,dr);
        textLength += node[nrNodes].occ;
        pq.push(nrNodes);
    }
}

void dfs(int nodeIndex,string nodeCode,nodeType* node,string* code) {

    if (node[nodeIndex].st == 0) {
        code[ node[nodeIndex].ch ] = nodeCode;

        return;
    }

    dfs(node[nodeIndex].st,nodeCode + "0",node,code);
    dfs(node[nodeIndex].dr,nodeCode + "1",node,code);
}

void writeI64(ull number,ofstream& out) {
    char aux[10] = {};
    ull mask = (1<<8) - 1;

    int shiftAmount = 56;
    for (int i=0;i < 8;++i) {
        aux[i] = (number>>shiftAmount) & mask;
        shiftAmount -= 8;
    }

    out.write(aux,8);
}

void writeI8(char number,ofstream& out) {
    out.write(&number,1);
}


void writeInfo(nodeType* node,int lastLeafIndex,string* code,ofstream& out) {
    writeI64(lastLeafIndex,out);

    for (int i=1;i <= lastLeafIndex;++i) {
        char ch = node[i].ch;
        char len = (char)code[ch].size();

        writeI8(ch,out);
        writeI8(len,out);

        string tempCode = code[ch];
        while (tempCode.size() % 8 != 0) {
            tempCode += "0";
        }

        int numberCode = 0;
        for (int j=0;j < (int)tempCode.size();++j) {
            numberCode <<= 1;
            numberCode += tempCode[j] - '0';

            if ((j+1) % 8 == 0) {
                writeI8(numberCode,out);
            }
        }
    }
}

dataCompressionClass::dataCompressionClass(char* name,string* _code,ofstream *_out) {
    in.open(name);

    in.seekg(0,in.end);
    fileEnd = in.tellg();
    in.seekg(0,in.beg);

    code = _code;
    out = _out;

    currentCharacter = in.get();
    codeIndex = 0;

    nextBit = true;
}

dataCompressionClass::~dataCompressionClass() {
    in.close();
}

void dataCompressionClass::writeData() {
    while (true) {
        char ch;

        if (getNext8Bit(ch) == false) {
            break;
        }
        writeI8(ch,*out);
    }
}

bool dataCompressionClass::getNext8Bit(char& ch) {
    if (checkNextBit() == false) {
        return false;
    }

    ch = 0;
    int nr = 0;
    while (checkNextBit() && nr < 8) {
        ch <<= 1;

        char aux = getNextBit();
        ch += aux;
        ++nr;
    }

    while (nr < 8) {
        ch <<= 1;
        ++nr;
    }

    return true;
}

bool dataCompressionClass::checkNextBit() {
    return nextBit;
}

char dataCompressionClass::getNextBit() {
    char ret = code[currentCharacter][codeIndex++] - '0';

    if (codeIndex == code[currentCharacter].size()) {
        if (in.tellg() == fileEnd) {
            nextBit = false;
        }
        else {
            currentCharacter = in.get();
            codeIndex = 0;
        }
    }

    return ret;
}


void decompress(char* inputFileName,char* outputFileName) {
    ifstream in(inputFileName,ios::binary);

    trieNode root;

    ull nrChar = readI64(in);
    for (ull i=0;i < nrChar;++i) {
        char currentCharacter = readI8(in);
        unsigned char codeLength = readI8(in);

        int inputLength = codeLength;
        while (inputLength % 8 != 0) {
            ++inputLength;
        }

        int byteNumber = inputLength / 8;

        char aux[300];
        in.read(aux,byteNumber);

        string code = "";
        for (int idx=0;idx < codeLength;++idx) {
            int i = idx/8, j = idx - 8*i;

            j = 7-j;
            char temp = (aux[i]>>j) & 1;
            code += temp + '0';
        }

        add(&root,currentCharacter,code,0);
    }

    ull nrBits = readI64(in);
    dataDecompressionClass writerObject(&in,outputFileName,nrBits,&root);
    writerObject.writeData();

    if (writerObject.hasFailed()) {
        printf("Failed to compress!\n");
    }
    else {
        printf("Data decompression successful!\n");
    }
}

ull readI64(ifstream& in) {
    char aux[10];
    in.read(aux,8);

    ull ret = 0;
    for (int i=0;i < 8;++i) {
        ret <<= 8;
        ret += (unsigned char)aux[i];
    }

    return ret;
}

char readI8(ifstream& in) {
    char ret;
    in.read(&ret,1);

    return ret;
}

void add(trieNode* node,char character,const string& code,int pos) {
    if ((int)code.size() == pos) {
        node->ch = character;
        return;
    }

    if (code[pos] == '0') {
        if (node->st == NULL) {
            node->st = new trieNode;
        }

        add(node->st,character,code,pos+1);
    }
    else {
        if (node->dr == NULL) {
            node->dr = new trieNode;
        }

        add(node->dr,character,code,pos+1);
    }
}

dataDecompressionClass::dataDecompressionClass(ifstream *_in,char* outputFileName,ull _nrBits,trieNode* _root): nrBits(_nrBits),root(_root) {
    in = _in;
    out.open(outputFileName);

    currentNode = root;

    in->read(&current8Bit,1);
    currentIndex = 0;

    fail = false;
}

dataDecompressionClass::~dataDecompressionClass() {
    out.close();
}

bool dataDecompressionClass::hasFailed() {
    return fail;
}

void dataDecompressionClass::writeData() {

    for (ull i=0;i < nrBits;++i) {
        char bit = getNextBit();

        char outCh = walkTrie(bit);
        if (fail) {
            return;
        }

        if (outCh != '\0') {
            out<<outCh;
        }
    }
}

char dataDecompressionClass::getNextBit() {
    if (currentIndex == 8) {
        currentIndex = 0;
        in->read(&current8Bit,1);
    }

    char bit = ((unsigned char)current8Bit>>(7-currentIndex)) & 1;
    bit += '0';

    ++currentIndex;
    return bit;
}

char dataDecompressionClass::walkTrie(char bit) {
    if (bit == '0') {
        if (currentNode->st == NULL) {
            printf("Error going left on trie while decompressing. Input file could be corrupt.\n");
            fail = true;
            return '\0';
        }

        currentNode = currentNode->st;
    }
    else {
        if (currentNode->dr == NULL) {
            printf("Error going right on trie while decompressing. Input file could be corrupt.\n");
            fail = true;
            return '\0';
        }

        currentNode = currentNode->dr;
    }

    char ret = currentNode->ch;
    if (ret != '\0') {
        currentNode = root;
    }

    return ret;
}
