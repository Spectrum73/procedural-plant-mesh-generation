#include "PlantGeneration.h"

Plant::Plant(PlantParameters aParams) : RootNode(NodeType::APICAL_BUD, glm::vec3(0.0f), glm::vec3(0.0f, 0.5f, 0.0f), 0.5f, aParams.RootCircumferenceEdges, aParams.RootCurveSegments, this, aParams.ApicalBudExtinction, aParams.GrowthRate) {
	parameters = aParams;
}

Plant::Plant(Plant* aPlant) : Plant(aPlant->getParameters()){
	CopyGraph(aPlant);
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

// TODO: edge loops need to be bridgeable between different edge counts before this should be implemented
// Returns how many edges a node should have
int ResolveEdges(float width, int parentEdges) {
	return parentEdges;
}
// Returns how many segments a node should have
int ResolveSegments(float length, int parentSegments) {
	if (length < 0.1f) return 1;
	else if (length < 0.5f) return ceilf(parentSegments / 2.0f);
	else if (length < 1.0f) return ceilf(parentSegments / 1.4f);
	else return parentSegments;
}

// creates an internode connected to the provided parent
void Plant::GenerateInternode(Node* parent, float chanceDecay) {
	float dChance = parent->getDeathChance() / chanceDecay;
	float gChance = parent->getGrowthChance() * chanceDecay;
	if (dChance >= 1.0f) return;
	std::cout <<  "DEATH CHANCE : " + std::to_string(dChance) + " | GROWTH CHANCE : " + std::to_string(gChance) << std::endl;
	std::cout << std::to_string(parent->getPosition().x) + " , " + std::to_string(parent->getPosition().y) + " , " + std::to_string(parent->getPosition().z) << std::endl;
	glm::vec3 location = parent->getPosition() + glm::vec3(getRandomFloat(-0.9f, 0.9f), getRandomFloat(0.4f, 1.0f), getRandomFloat(-0.9f, 0.9f));
	glm::vec3 control = glm::vec3(0.0f, 0.5f, 0.0f);
	float w = parent->getWidth() * getRandomFloat(0.6f, 0.8f);
	int edges = ResolveEdges(w, parent->getEdges());
	int segments = ResolveSegments(glm::distance(location, parent->getPosition()), parent->getSegments());
	std::cout << "EDGES : " + std::to_string(edges) + "| SEGMENTS : " + std::to_string(segments) << std::endl;
	Node* internode = new Node(NodeType::APICAL_BUD, location, control, w, edges, segments, this, dChance, gChance);
	parent->AddChild(internode);
}

// Recursive function to perform a growth cycle on a given node and its children
void Plant::SimulateGrowthCycle(Node* node, float chanceDecay) {
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
	// If plant has already grown (we are regenerating)
	if (!hasLivingBuds()) {
		RootNode.getChildren().clear();
		RootNode.setDead(false);
		RootNode.setDeathChance(parameters.ApicalBudExtinction);
		RootNode.setGrowthChance(parameters.GrowthRate);
	}

	GenerateInternode(&RootNode, chanceDecay); // force at least one bud to grow from the root.
	RootNode.setGrowthChance(0.0f);


	// Keep generating until all nodes are dead.
	while (hasLivingBuds()) {
		SimulateGrowthCycle(&RootNode, chanceDecay);
	}
	std::cout << "Graph Done!" << std::endl;
	return;
}

void Plant::CopyGraph(Plant* copyTarget) {
	RootNode = *(new Node(&copyTarget->RootNode, this));
}

// Creates a curve by passing in two nodes and the desired circumference subdivisions and number of segments.
Curve makeCurveFromNodes(Node* firstNode, Node* secondNode, int aSubdivisions, int aSegments) {
	return Curve(aSubdivisions, aSegments, firstNode->getWidth(), secondNode->getWidth(),
		firstNode->getPosition(),
		secondNode->getPosition(),
		firstNode->getControlPoint(),
		-secondNode->getControlPoint()); // This is inverted to make sure that connected curves match correctly
}

// this function will recursively append curves created from segments to a passed vector
void addCurvesFromNode(Node* node, std::vector<Curve>& curves, int aEdgeReduction = 0, int aSegmentReduction = 0) {
	if (node != nullptr && !node->getChildren().empty()) {
		PlantParameters pParams = node->getPlant()->getParameters();
		for (Node* childNode : node->getChildren()) {
			int edges = node->getEdges() - aEdgeReduction;
			edges = edges < 3 ? edges = 3 : edges; // Clamp to the minimum
			int segments = node->getSegments() - aSegmentReduction;
			segments = segments < 1 ? segments = 1 : segments; // Clamp to the minimum
			curves.push_back(makeCurveFromNodes(node, childNode, edges, segments));
			addCurvesFromNode(childNode, curves, aEdgeReduction, aSegmentReduction);
		}
	}
}

void Plant::GenerateMesh(int aEdgeReduction, int aSegmentReduction) {
	if (this->curves.size() > 0) {
		this->curves.clear();
	}
	// Clear our previous mesh
	this->Delete();

	if (&RootNode != nullptr)
		addCurvesFromNode(&RootNode, this->curves, aEdgeReduction, aSegmentReduction);

	// Concatenate all curve meshes to our main plant mesh
	for (Curve& curve : this->curves)
	{
		this->Concatenate(curve);
	}
}

/*
void Plant::Draw(Shader& shader, Camera& camera) {
	for (Curve curve : curves)
	{
		curve.Draw(shader, camera);
	}
}
*/