#include "SDRST.h"

float similarity(const std::vector<float> &left, const std::vector<float> &right) {
	float s = 0.0f;

	for (int i = 0; i < left.size(); i++)
		s += left[i] * right[i];

	return s;
}

void moveTowards(const std::vector<float> &target, std::vector<float> &vec, float ratio) {
	for (int i = 0; i < vec.size(); i++)
		vec[i] = (1.0f - ratio) * vec[i] + ratio * target[i];
}

void randomPrototype(std::vector<float> &vec, std::mt19937 &generator) {
	std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

	for (int i = 0; i < vec.size(); i++)
		vec[i] = dist01(generator);
}

void SDRST::Node::add(Vec* pVec, int maxOccupantsPerNode, std::mt19937 &generator, bool allowReAdd) {
	
}

void SDRST::Node::findMostSimilar(const std::vector<float> &vec, Node* &pNode, Vec* &pMostSim, float &sim) {
	pNode = nullptr;

	if (_pParent != nullptr)
		sim = similarity(vec, _isLeft ? _pParent->_protoLeft : _pParent->_protoRight);

	if (_occupants.empty() && _pLeft == nullptr && _pRight == nullptr) {
		pNode = this;

		pMostSim = nullptr;
	}

	for (int i = 1; i < _occupants.size(); i++)
		if (vec == _occupants[i]->_vec) {
			pMostSim = _occupants[i];

			return;
		}

	// Find closest occupant
	float maxSim = -999999.0f;
	int maxSimIndex = -1;

	if (!_occupants.empty()) {
		maxSimIndex = 0;

		maxSim = similarity(vec, _occupants.front()->_vec);

		for (int i = 1; i < _occupants.size(); i++) {
			float sim = similarity(vec, _occupants[i]->_vec);

			if (sim > maxSim) {
				maxSim = sim;
				maxSimIndex = i;
			}
		}
	}

	if (maxSim > sim) {
		sim = maxSim;

		pMostSim = _occupants[maxSimIndex];
	}

	if (_pLeft != nullptr) {
		pNode = this;

		// Check prototypes
		float simProtoLeft = similarity(vec, _protoLeft);

		float simProtoRight = similarity(vec, _protoRight);

		/*if (maxSimIndex != -1)
			if (maxSim > simProtoLeft && maxSim > simProtoRight) {
				sim = maxSim;

				return _occupants[maxSimIndex];
			}*/
			
		// Go down most similar route
		if (simProtoLeft > simProtoRight)
			return _pLeft->findMostSimilar(vec, pNode, pMostSim, sim);

		return _pRight->findMostSimilar(vec, pNode, pMostSim, sim);
	}

	if (maxSimIndex == -1) {
		pMostSim = nullptr;

		return;
	}

	pNode = this;
}

void SDRST::create(int vecSize, std::mt19937 &generator) {
	_vecSize = vecSize;

	_pRoot = std::make_unique<Node>();

	_pRoot->_pTree = this;
}

void SDRST::addSub(Vec* pVec, int maxOccupantsPerNode, std::mt19937 &generator) {
	Node* pNode;

	float mostSim = -999999.0f;

	Vec* pMostSim;
	
	_pRoot->findMostSimilar(pVec->_vec, pNode, pMostSim, mostSim);

	if (pNode == nullptr) {
		_pRoot->_occupants.push_back(pVec);

		_pRoot->_numOccupantsBelow++;

		return;
	}

	bool added = false;

	// Add to most similar if there is room, otherwise to children (make children if there are none)
	while (!added) {
		if (pNode->_occupants.size() < maxOccupantsPerNode) {
			for (int i = 0; i < pNode->_occupants.size(); i++)
				_reAddBuffer.push_back(pNode->_occupants[i]);

			pNode->_occupants.push_back(pVec);

			pNode->_numOccupantsBelow++;

			// Update similarity
			if (pNode->_pParent != nullptr) {
				if (pNode->_isLeft)
					moveTowards(pVec->_vec, pNode->_pParent->_protoLeft, 1.0f / pNode->_numOccupantsBelow);
				else
					moveTowards(pVec->_vec, pNode->_pParent->_protoRight, 1.0f / pNode->_numOccupantsBelow);
			}

			added = true;
		}
		else {
			// If there are children, add to those
			if (pNode->_pLeft != nullptr) {
				float simLeft;
				float simRight;

				Node* pLeftNode;

				Vec* pMostSimLeft;
					
				pNode->_pLeft->findMostSimilar(pVec->_vec, pLeftNode, pMostSimLeft, simLeft);

				Node* pRightNode;

				Vec* pMostSimRight;
				
				pNode->_pRight->findMostSimilar(pVec->_vec, pRightNode, pMostSimRight, simRight);

				pNode->_numOccupantsBelow++;

				if (simLeft > simRight) {
					pNode = pLeftNode;

					pMostSim = pMostSimLeft;
				}
				else {
					pNode = pRightNode;

					pMostSim = pMostSimRight;
				}
			}
			else {
				// Split and add there
				pNode->_pLeft = std::make_unique<Node>();
				pNode->_pRight = std::make_unique<Node>();

				pNode->_pLeft->_isLeft = true;
				pNode->_pLeft->_pTree = this;
				pNode->_pLeft->_pParent = pNode;

				pNode->_pRight->_isLeft = false;
				pNode->_pRight->_pTree = this;
				pNode->_pRight->_pParent = pNode;

				pNode->_protoLeft.resize(pVec->_vec.size());

				randomPrototype(pNode->_protoLeft, generator);

				pNode->_protoRight.resize(pVec->_vec.size());

				randomPrototype(pNode->_protoRight, generator);

				// Left is new, right is original
				pNode->_numOccupantsBelow++;
				pNode->_pLeft->_numOccupantsBelow++;

				pNode->_pLeft->_occupants.push_back(pVec);		

				pNode->_protoLeft = pVec->_vec;
				pNode->_protoRight = pVec->_vec;

				added = true;
			}
		}
	}
}

void SDRST::add(Vec* pVec, int maxOccupantsPerNode, std::mt19937 &generator, int balanceIter) {
	addSub(pVec, maxOccupantsPerNode, generator);
	
	for (int iter = 0; iter < balanceIter; iter++) {
		for (int i = 0; i < _reAddBuffer.size(); i++)
			addSub(_reAddBuffer[i], maxOccupantsPerNode, generator);

		_reAddBuffer.clear();
	}
}

Vec* SDRST::findMostSimilar(const std::vector<float> &vec, float &sim) {
	Node* pNode;

	Vec* pMostSim;

	_pRoot->findMostSimilar(vec, pNode, pMostSim, sim);

	return pMostSim;
}