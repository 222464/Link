#pragma once

#include <vector>
#include <memory>
#include <random>

float similarity(const std::vector<float> &left, const std::vector<float> &right);
void moveTowards(const std::vector<float> &target, std::vector<float> &vec, float ratio);
void randomPrototype(std::vector<float> &vec, std::mt19937 &generator);

class Vec {
public:
	std::vector<float> _vec;

	virtual ~Vec() {}
};

class SDRST {
private:
	struct Node {
		bool _isLeft;

		SDRST* _pTree;

		Node* _pParent;

		std::unique_ptr<Node> _pLeft;
		std::unique_ptr<Node> _pRight;

		// Prototype is actual object if is leaf
		std::vector<float> _protoLeft;
		std::vector<float> _protoRight;

		std::vector<Vec*> _occupants;

		int _numOccupantsBelow;

		Node()
			: _pTree(nullptr), _pParent(nullptr), _isLeft(false), _numOccupantsBelow(0)
		{}

		void add(Vec* pVec, int maxOccupantsPerNode, std::mt19937 &generator, bool allowReAdd);

		Vec* findMostSimilar(const std::vector<float> &vec, Node* &pNode, float &sim);
	};
	
	std::unique_ptr<Node> _pRoot;

	int _vecSize;

	std::vector<Vec*> _reAddBuffer;

	void addSub(Vec* pVec, int maxOccupantsPerNode, std::mt19937 &generator);

public:
	void create(int vecSize, std::mt19937 &generator);

	Vec* findMostSimilar(const std::vector<float> &vec, float &sim);

	void add(Vec* pVec, int maxOccupantsPerNode, std::mt19937 &generator, int balanceIter);
};