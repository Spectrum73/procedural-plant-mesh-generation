#include "PlantGeneration.h"

Plant::Plant(PlantParameters aParams) : RootNode(NodeType::APICAL_BUD, glm::vec3(0.0f), glm::vec3(0.0f, 0.5f, 0.0f), 0.5f, this, aParams.ApicalBudExtinction, aParams.GrowthRate) {
	parameters = aParams;
}

bool recursiveLivingNodeCheck(Node* parentNode) {
	bool living = false;
	living |= !(parentNode->isDead());
	if (!parentNode->getChildren().empty()) {
		for (Node* node : parentNode->getChildren()) {
			living |= recursiveLivingNodeCheck(node);
		}
	}
	return living;
}

bool Plant::hasLivingBuds() {
	// Start at the root and do a check recursively
	bool livingBuds = recursiveLivingNodeCheck(&RootNode);
	return livingBuds;
}

// creates an internode connected to the provided parent
void GenerateInternode(Node* parent, float chanceDecay) {
	float dChance = parent->getDeathChance() / chanceDecay;
	float gChance = parent->getGrowthChance() * chanceDecay;
	if (dChance >= 1.0f) return;
	std::cout << std::to_string(dChance) + " GROWTH CHANCE : " + std::to_string(gChance) << std::endl;
	std::cout << std::to_string(parent->getPosition().x) + " , " + std::to_string(parent->getPosition().y) + " , " + std::to_string(parent->getPosition().z) << std::endl;
	glm::vec3 location = parent->getPosition() + glm::vec3(getRandomFloat(-0.9f, 0.9f), getRandomFloat(0.8f, 1.0f), getRandomFloat(-0.9f, 0.9f));
	glm::vec3 control = location + -glm::vec3(0.0f, 0.5f, 0.0f);
	float w = parent->getWidth() * getRandomFloat(0.6f, 0.8f);
	Plant* plant = parent->getPlant();
	Node* internode = new Node(NodeType::APICAL_BUD, location, control, w, plant, dChance, gChance);
	parent->AddChild(internode);
}

// Recursive function to perform a growth cycle on a given node and its children
void SimulateGrowthCycle(Node* node, float chanceDecay) {
	float r = getRandomFloat(0.0f, 1.0f);
	//std::cout << "DEAD? " + std::to_string(r) << std::endl;
	node->setDead(r <= node->getDeathChance()); // Does the node die?
	if (!node->isDead()) {
		if (getRandomFloat(0.0f, 1.0f) <= node->getGrowthChance()) { // Does this node grow a shoot
			GenerateInternode(node, chanceDecay);
			node->setType(NodeType::SEGMENT);
			node->setDead(true);
		}
	}
	node->setDeathChance(node->getDeathChance() * 1.5f);

	if (!node->getChildren().empty()) {
		for (Node* childNode : node->getChildren())
			SimulateGrowthCycle(childNode, chanceDecay); // Simulate the children, must be done before growing
	}
}

void Plant::GenerateGraph() {
	float chanceDecay = 0.96f; // Each child's death chance = parent's deathChance / chanceDecay
	GenerateInternode(&RootNode, chanceDecay); // force at least one bud to grow from the root.
	RootNode.setGrowthChance(0.0f);
	// Keep generating until all nodes are dead.
	while (hasLivingBuds()) {
		SimulateGrowthCycle(&RootNode, chanceDecay);
	}
	std::cout << "Graph Done!" << std::endl;
	return;
}

Curve makeCurveFromNodes(Node* firstNode, Node* secondNode) {
	return Curve(8, 6, firstNode->getWidth(), secondNode->getWidth(),
		firstNode->getPosition(),
		secondNode->getPosition(),
		firstNode->getControlPoint(),
		secondNode->getControlPoint()); // This is inverted to make sure that connected curves match correctly
}

// this function will recursively append curves created from segments to a passed vector
void addCurvesFromNode(Node* node, std::vector<Curve>& curves) {
	if (node != nullptr && !node->getChildren().empty()) {
		for (Node* childNode : node->getChildren()) {
			curves.push_back(makeCurveFromNodes(node, childNode));
			addCurvesFromNode(childNode, curves);
		}
	}
}

void Plant::GenerateMesh() {
	if (&RootNode != nullptr)
		addCurvesFromNode(&RootNode, curves);
	return;
}

void Plant::Draw(Shader& shader, Camera& camera) {
	for (Curve curve : curves)
	{
		curve.Draw(shader, camera);
	}
}