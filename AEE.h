#ifndef __EXP3__AEE_H__
#define __EXP3__AEE_H__

#include <vector>
#include <utility>
#include <unordered_map>
#include <string>

template <typename EntityIDType, typename PosType, typename LenType, typename SimType>
struct ExtractResult {
    EntityIDType id;
    PosType pos;
    LenType len;
    SimType sim;
    bool operator!=(const ExtractResult &b) {
    	return !(id == b.id && pos == b.pos && len == b.len && sim == b.sim);
    }
};

typedef ExtractResult<unsigned, unsigned, unsigned, unsigned> EDExtractResult;
typedef ExtractResult<unsigned, unsigned, unsigned, double> JaccardExtractResult;

const int SUCCESS = 0;
const int FAILURE = 1;
const int THRESHOLD = 2;
const int HASHSIZE = 1500007;
const int MAXENTITY = 110000;
const int MAXENTITYLEN = 300;
const int MAXDOC = 5100;
const int MAXDOCLEN = 1100;
const int ASCIISIZE = 256;

struct Entity {
	int length;
	int segpos[4];
	std::string name;
};

class TrieNode {
public:
	TrieNode** children;
	bool leaf;
	std::vector<int> entityCandidateLeft;
	std::vector<int> entityCandidateMiddle;
	std::vector<int> entityCandidateRight;
public:
	TrieNode(int charUniNum = ASCIISIZE);
	~TrieNode();
};

class AEE {
private:
	int* backStartPos;
	int backStartPosSize;
	int* forwTailPos;
	int forwTailPosSize;

private:
	int segmentNum;
	int entityNum;
	int entityMinLen;
	int subDocMinLen;
	int entityMaxLen;
	int subDocMaxLen;
	int* subDocED;
	int segMinLen;
	Entity* entity;
	int* charUniMap;
	int charUniNum;
	TrieNode* root;
	int calcED(const char* doc1, int len1, const char* doc2, int len2);
	static bool compareEDResult(const EDExtractResult &a, const EDExtractResult &b);

public:
    AEE();
    ~AEE();

    int createIndex(const char *entity_file_name);
    int aeeJaccard(const char *document, double threshold, std::vector<JaccardExtractResult> &result);
    int aeeED(const char *document, unsigned threshold, std::vector<EDExtractResult> &result);
};

#endif
