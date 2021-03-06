#include "AEE.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <time.h>
#include <unordered_map>

using namespace std;

// global variable for calcED()
int bot = 1;
int top = 2 * THRESHOLD + 2;
int vl = 0;
int vn = 0;
int vt = 0;
int l2 = 0;
unsigned editdist[2 * THRESHOLD + 3];

// global variable for aeeED()
int currentChar;
int currentPos;
int entityId;
int dist;
int enheadlen;
int entaillen;
int forwbot;
int forwupp;
int backbot;
int backupp;
int subdocmin;
int subdocmax;
int subdocmax2;
int subduration;
int pi, dl;

TrieNode::TrieNode(int charUniNum) {
	children = new TrieNode* [charUniNum];
	for (int i = 0 ; i < charUniNum ; i++)
		children[i] = NULL;
	leaf = false;
	entityCandidateLeft.clear();
	entityCandidateMiddle.clear();
	entityCandidateRight.clear();
}

TrieNode::~TrieNode() {

}


AEE::AEE() {
	segmentNum = THRESHOLD + 1;
	entityMinLen = MAXENTITYLEN;
	entityMaxLen = 0;
	segMinLen = MAXENTITYLEN;
	entity = new Entity[MAXENTITY];
	charUniMap = new int[ASCIISIZE];
	for (int i = 0 ; i < ASCIISIZE ; i++)
		charUniMap[i] = -1;
	charUniNum = 0;
	backStartPos = new int[MAXENTITY];
	forwTailPos = new int[MAXENTITY];
}

AEE::~AEE() {
}

bool AEE::compareEDResult(const EDExtractResult &a, const EDExtractResult &b) {
	return (a.id != b.id) ? (a.id < b.id) :
			(a.pos != b.pos) ? (a.pos < b.pos) :
			(a.len < b.len);
}

int AEE::calcED(const char* doc1, int len1, const char* doc2, int len2) {
	//if (abs(len1 - len2) > THRESHOLD)
	//	return THRESHOLD + 1;
	//if (len1 == 0)
	//	return len2;
	//if (len2 == 0)
	//	return len1;
	/*
	cout << len1 << endl;
	for (int i = 0 ; i < len1 ; i++)
		cout << doc1[i];
	cout << endl;
	cout << len2 << endl;
	for (int i = 0; i < len2; ++i)
		cout << doc2[i];
	cout << endl;
	*/
	bot = 1;
	top = 2 * THRESHOLD + 2;
	vl = 0;
	vn = 0;
	vt = 0;
	l2 = 0;
	//unsigned editdist[2 * THRESHOLD + 3];
	editdist[0] = THRESHOLD + 1;
	editdist[2 * THRESHOLD + 2] = THRESHOLD + 1;
	for (int i = 1; i < THRESHOLD + 1; ++i) {
		editdist[i] = THRESHOLD + 1 - i;
	}
	for (int i = THRESHOLD + 1; i < 2 * THRESHOLD + 2; ++i) {
		editdist[i] = i - THRESHOLD - 1;
	}
	//update edit distance
	for (int l1 = 1; l1 <= len1; ++l1) {
		for (int i = bot; i < top; ++i) {
			vl = editdist[i-1]+1;
			vt = editdist[i+1]+1;
			l2 = i - THRESHOLD + l1 - 2;
			vn = editdist[i] +
			     ((l2 >= 0 && l2 < len2) ? (doc1[l1-1] != doc2[l2]) : 1);
			editdist[i] = (vl > vt) ? ((vt > vn) ? vn : vt) : ((vl > vn) ? vn : vl);
		}
		for (int i = bot; i < top; ++i) {
			if (editdist[bot] > THRESHOLD) bot++;
			else break;
		}
		for (int i = top-1; i >= bot; --i) {
			if (editdist[top - 1] > THRESHOLD) top--;
			else break;
		}
		if (bot >= top) break;
	}
	// result
	//cout << "dist:" << editdist[THRESHOLD + 1 + len2 - len1] << endl;
	return editdist[THRESHOLD + 1 + len2 - len1];
}

int AEE::createIndex(const char *entity_file_name) {
	FILE* infile = fopen(entity_file_name, "r");
	char* line = NULL;
	size_t len = 0;
	unsigned id = 0;
	Entity currentEntity;
	while(getline(&line, &len, infile) != -1) {
		currentEntity.name = line;
		currentEntity.length = strlen(line);

		// cut \n
		if (currentEntity.name[currentEntity.length - 1] == '\n') {
			currentEntity.name[currentEntity.length - 1] = '\0';
			currentEntity.length -= 1;
		}

		// build char map
		for (int ci = 0 ; ci < currentEntity.length ; ci++) {
			if (charUniMap[(int)(currentEntity.name[ci])] < 0)
				charUniMap[(int)(currentEntity.name[ci])] = charUniNum ++;
		}

		// update max and min len of entity name
		if (currentEntity.length < entityMinLen)
			entityMinLen = currentEntity.length;
		else if (currentEntity.length > entityMaxLen)
			entityMaxLen = currentEntity.length;

		int mode = currentEntity.length % segmentNum;
		int seglen = currentEntity.length / segmentNum;
		if (seglen < segMinLen)
			segMinLen = seglen;
		currentEntity.segpos[0] = 0;
		for (int i = 1 ; i < segmentNum ; i++) {
			currentEntity.segpos[i] = currentEntity.segpos[i - 1] + seglen;
			if (i == segmentNum - mode)
				seglen ++;
		}
		currentEntity.segpos[3] = currentEntity.length;

		//for (int i = 0; i < 4; ++i) {
		//	cout << currentEntity.segpos[i] << " ";
		//}
		//cout << endl;
		entity[id] = currentEntity;
		id ++;
	}
	entityNum = id;
	subDocMinLen = entityMinLen - THRESHOLD;
	subDocMaxLen = entityMaxLen + THRESHOLD;
	
	subDocED = new int[subDocMaxLen];

	for (int i = 0 ; i < ASCIISIZE ; i++) {
		if (charUniMap[i] < 0)
			charUniMap[i] = charUniNum;
	}
	charUniNum ++;

	// build trie tree
	root = new TrieNode(charUniNum);
    for (int ei = 0 ; ei < entityNum ; ei++) {
		int mode = entity[ei].length % segmentNum;
		int startpos = 0;
		int enlen = entity[ei].length / segmentNum;
		//cout << "-----\n" << entity[ei].name << endl;
		for (int i = 0; i < segmentNum; ++i) {
			if (i == segmentNum - mode)
				enlen ++;
			TrieNode* parent = root;
			for (int p = 0; p < enlen; p++) {
				int cp = startpos + p;
				int cc = charUniMap[(int)(entity[ei].name[cp])];
				//cout << entity[ei].name[cp] << " " << cc << " ";
				if (parent->children[cc] == NULL) {
					TrieNode* child = new TrieNode(charUniNum);
					parent->children[cc] = child;
				}
				parent = parent->children[cc];
			}
			//cout << endl;
			parent->leaf = true;
			if (i == 0)
				parent->entityCandidateLeft.push_back(ei);
			else if (i == segmentNum - 1)
				parent->entityCandidateRight.push_back(ei);
			else
				parent->entityCandidateMiddle.push_back(ei);
			/*
			for (int pi = 0 ; pi < parent->entityCandidateLeft.size() ; pi++)
				cout << parent->entityCandidateLeft[pi] << " ";
		    cout << endl;
			for (int pi = 0 ; pi < parent->entityCandidateMiddle.size() ; pi++)
				cout << parent->entityCandidateMiddle[pi] << " ";
		    cout << endl;
		    for (int pi = 0 ; pi < parent->entityCandidateRight.size() ; pi++)
				cout << parent->entityCandidateRight[pi] << " ";
		    cout << endl;
		    */
			startpos += enlen;
		}
		//if (startpos != entity[ei].length) {
		//	cout << "Error! startpos != entity[ei].length" << endl;
		//}
	}

    return SUCCESS;
}

int AEE::aeeJaccard(const char *document, double threshold, vector<JaccardExtractResult> &result) {
    result.clear();
    return SUCCESS;
}

int AEE::aeeED(const char *document, unsigned threshold, vector<EDExtractResult> &result) {
    result.clear();
    vector<EDExtractResult> resultCandidate;
    resultCandidate.clear();
    int doclen = strlen(document);
    TrieNode* parent = root;
    
    const int endpos = (doclen - segMinLen + 1);
    for (int startpos = 0 ; startpos < endpos; startpos++) {
    	//cout << "startpos:" << startpos << endl;
    	parent = root;
    	currentPos = startpos;
    	//while (false) {
    	while (currentPos < doclen) {
    		currentChar = charUniMap[(int)document[currentPos]];
    		//if (currentChar < 0)
    		//	break;
    		parent = parent->children[currentChar];
    		currentPos ++;
    		if (parent == NULL)
    			break;
    		else if (parent->leaf) {
    			subduration = currentPos - startpos;
    			subdocmin = max(0, subDocMinLen - subduration);
    			subdocmax = min(doclen - currentPos, subDocMaxLen - subduration);
    			subdocmax2 = min(startpos, subDocMaxLen - subduration);
    			for (pi = 0 ; pi < parent->entityCandidateLeft.size() ; pi++) {
					//cout << parent->entityCandidateLeft[pi] << " ";
					entityId = parent->entityCandidateLeft[pi];
					entaillen = entity[entityId].segpos[3] - entity[entityId].segpos[1];
					forwbot = max(subdocmin, entaillen - THRESHOLD);
					forwupp = min(subdocmax, entaillen + THRESHOLD);
					//int forwbot = 1;
					//int forwupp = doclen - currentPos;
					for (dl = forwbot; dl <= forwupp; ++dl) {
						dist = calcED(entity[entityId].name.c_str() + entity[entityId].segpos[1], entaillen,
						              document + currentPos, dl);
						if (dist <= THRESHOLD) {
							resultCandidate.push_back(EDExtractResult{(unsigned)entityId, (unsigned)(startpos), (unsigned)(dl + (currentPos - startpos)), (unsigned)(dist)});
							/*
						ganj	EDExtractResult er = {(unsigned)entityId, (unsigned)(startpos), (unsigned)(dl + (currentPos - startpos)), (unsigned)(dist)};
							cout << "------------------" << endl;
							cout << er.id << " " << er.pos << " " << er.len << " " << er.sim << endl;
							cout << entity[entityId].name << endl;
							for (int ci = 0 ; ci < er.len ; ci++) {
								cout << document[er.pos + ci];
							}
							cout << endl;
							cout << "------------------" << endl;
							*/
						}
					}
				}
			    //cout << endl;
				for (pi = 0 ; pi < parent->entityCandidateMiddle.size() ; pi++) {
					//cout << parent->entityCandidateMiddle[pi] << " ";
					entityId = parent->entityCandidateMiddle[pi];
					enheadlen = entity[entityId].segpos[1];
					entaillen = entity[entityId].segpos[3] - entity[entityId].segpos[2];
					forwbot = max(0, entaillen - 1);
					forwupp = min(subdocmax, entaillen + 1);
					backbot = max(0, enheadlen - 1);
					backupp = min(subdocmax2, enheadlen + 1);
					// backwards
					backStartPosSize = 0;
					for (dl = backbot ; dl <= backupp ; ++dl) {
						dist = calcED(entity[entityId].name.c_str(), enheadlen,
						              document + startpos - dl, dl);
						if (dist == 1)
							backStartPos[backStartPosSize++] = startpos - dl;
					}
					// forwards
					forwTailPosSize = 0;
					for (dl = forwbot ; dl <= forwupp ; ++dl) {
						dist = calcED(entity[entityId].name.c_str() + entity[entityId].segpos[2], entaillen,
						              document + currentPos, dl);
						if (dist == 1)
							forwTailPos[forwTailPosSize++] = currentPos + dl;
					}
					// combine
					for (int rbi = 0; rbi < backStartPosSize; ++rbi) {
						for (int rfi = 0; rfi < forwTailPosSize; ++rfi)
							resultCandidate.push_back(EDExtractResult{(unsigned)entityId, (unsigned)(backStartPos[rbi]),
									                                  (unsigned)(forwTailPos[rfi] - backStartPos[rbi]), (unsigned)(2)});
					}
				}
			    //cout << endl;
			    for (pi = 0 ; pi < parent->entityCandidateRight.size() ; pi++) {
					//cout << parent->entityCandidateLeft[pi] << " ";
					entityId = parent->entityCandidateRight[pi];
					enheadlen = entity[entityId].segpos[2];
					backbot = max(subdocmin, enheadlen - THRESHOLD);
					backupp = min(subdocmax2, enheadlen + THRESHOLD);
					//int backbot = 1;
					//int backupp = startpos;
					//cout << backbot << " " << backupp << endl;
					for (dl = backbot; dl <= backupp; ++dl) {
						//cout << "right" << endl;
						dist = calcED(entity[entityId].name.c_str(), enheadlen,
						              document + startpos - dl, dl);
						if (dist <= THRESHOLD && dist > 0) { // dist == 0 can be found in candidateleft
							resultCandidate.push_back(EDExtractResult{(unsigned)entityId, (unsigned)(startpos - dl), (unsigned)(dl + (currentPos - startpos)), (unsigned)(dist)});
							/*
							EDExtractResult er = {(unsigned)entityId, (unsigned)(startpos - dl), (unsigned)(dl + (currentPos - startpos)), (unsigned)(dist)};
							cout << "------------------" << endl;
							cout << er.id << " " << er.pos << " " << er.len << " " << er.sim << endl;
							cout << entity[entityId].name << endl;
							for (int ci = 0 ; ci < er.len ; ci++) {
								cout << document[er.pos + ci];
							}
							cout << endl;
							cout << "------------------" << endl;
							*/
						}
					}
				}
			    //cout << endl;
    		}
    	}
    	//cout << "startpos: " << startpos << endl;
    }
    sort(resultCandidate.begin(), resultCandidate.end(), compareEDResult);
    if (resultCandidate.size() > 0) {
    	result.push_back(resultCandidate[0]);
    	for (int i = 1; i < resultCandidate.size(); ++i) {
    		if (result.back() != resultCandidate[i])
    			result.push_back(resultCandidate[i]);
    	}
    } else {
    	return SUCCESS;
    }
    //char* str1 = "ex";
    //char* str2 = "ex ";
    //cout << calcED(str1, 2, str2, 3) << endl;
    //cout << calcED(str1, 1, str2, 0) << endl;
    //cout << calcED(str1, 1, str2, 3) << endl;
    return SUCCESS;
}
