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

void SDRST::Node::findMostSimilar(const std::vector<float> &vec, Node* &pNode, Vec* &pMostSim, float &sim) {
	pNode = this;

	if (_pParent != nullptr) {
		float thisSim = similarity(vec, _isLeft ? _pParent->_protoLeft : _pParent->_protoRight);

		if (thisSim > sim)
			sim = thisSim;
	}

	if (_occupants.empty() && _pLeft == nullptr && _pRight == nullptr)
		return;

	for (int i = 0; i < _occupants.size(); i++)
		if (vec == _occupants[i]->_vec) {
			pMostSim = _occupants[i];

			sim = similarity(vec, vec);

			return;
		}

	// Find closest occupant
	if (!_occupants.empty()) {
		float maxSim = similarity(vec, _occupants.front()->_vec);
		int maxSimIndex = 0;

		for (int i = 1; i < _occupants.size(); i++) {
			float s = similarity(vec, _occupants[i]->_vec);

			if (s > maxSim) {
				maxSim = s;
				maxSimIndex = i;
			}
		}

		if (maxSim > sim) {
			sim = maxSim;

			pMostSim = _occupants[maxSimIndex];
		}
	}

	if (_pLeft != nullptr) {
		// Check prototypes
		float simProtoLeft = similarity(vec, _protoLeft);

		float simProtoRight = similarity(vec, _protoRight);
			
		// Go down most similar route
		if (simProtoLeft > simProtoRight)
			_pLeft->findMostSimilar(vec, pNode, pMostSim, sim);
		else 
			_pRight->findMostSimilar(vec, pNode, pMostSim, sim);
	}
}

void SDRST::create(int vecSize, std::mt19937 &generator) {
	_vecSize = vecSize;

	_pRoot = std::make_unique<Node>();

	_pRoot->_pTree = this;
}

void SDRST::addSub(Vec* pVec, int maxOccupantsPerNode, std::mt19937 &generator, bool allowReAdd) {
	Node* pNode = nullptr;

	float mostSim = -999999.0f;

	Vec* pMostSim;
	
	_pRoot->findMostSimilar(pVec->_vec, pNode, pMostSim, mostSim);

	if (pNode == nullptr) {
		_pRoot->_occupants.push_back(pVec);

		return;
	}

	bool added = false;

	// Add to most similar if there is room, otherwise to children (make children if there are none)
	while (!added) {
		if (pNode->_occupants.size() < maxOccupantsPerNode) {
			int prevSize = pNode->_occupants.size();

			if (allowReAdd) {
				for (int i = 0; i < pNode->_occupants.size(); i++)
					_reAddBuffer.push_back(pNode->_occupants[i]);

				pNode->_occupants.clear();
			}

			pNode->_occupants.push_back(pVec);

			// Update similarity
			if (pNode->_pParent != nullptr) {
				if (pNode->_isLeft)
					moveTowards(pVec->_vec, pNode->_pParent->_protoLeft, 1.0f / (1 + prevSize));
				else
					moveTowards(pVec->_vec, pNode->_pParent->_protoRight, 1.0f / (1 + prevSize));
			}

			added = true;
		}
		else {
			// If there are children, add to those
			if (pNode->_pLeft != nullptr) {
				float simLeft;
				float simRight;

				Node* pLeftNode = nullptr;

				Vec* pMostSimLeft = nullptr;
					
				pNode->_pLeft->findMostSimilar(pVec->_vec, pLeftNode, pMostSimLeft, simLeft);

				Node* pRightNode = nullptr;

				Vec* pMostSimRight = nullptr;
				
				pNode->_pRight->findMostSimilar(pVec->_vec, pRightNode, pMostSimRight, simRight);

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

				// Left is new, right is original
				pNode->_pLeft->_occupants.push_back(pVec);		

				pNode->_protoLeft = pVec->_vec;
				pNode->_protoRight = pVec->_vec;

				pNode = pNode->_pLeft.get();

				added = true;
			}
		}
	}
}

void SDRST::add(Vec* pVec, int maxOccupantsPerNode, std::mt19937 &generator, int balanceIter) {
	_reAddBuffer.clear();

	_reAddBuffer.push_back(pVec);

	for (int iter = 0; iter < balanceIter; iter++) {
		std::vector<Vec*> reAddCopy = _reAddBuffer;

		_reAddBuffer.clear();

		for (int i = 0; i < reAddCopy.size(); i++)
			addSub(reAddCopy[i], maxOccupantsPerNode, generator, iter != (balanceIter - 1));
	}

	_reAddBuffer.clear();
}

Vec* SDRST::findMostSimilar(const std::vector<float> &vec, float &sim) {
	Node* pNode;

	Vec* pMostSim;

	_pRoot->findMostSimilar(vec, pNode, pMostSim, sim);

	return pMostSim;
}