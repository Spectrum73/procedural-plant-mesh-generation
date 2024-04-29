#include "PlantGeneration.h"

Plant::Plant(PlantParameters aParams) : RootNode(NodeType::APICAL_BUD, glm::vec3(0.0f), glm::vec3(0.0f, 0.5f, 0.0f), aParams.RootWidth, glm::vec3(0.0f, 1.0f, 0.0f), aParams.RootCircumferenceEdges, aParams.RootCurveSegments, this, aParams.ApicalBudExtinction, aParams.GrowthRate) {
	parameters = aParams;
}

Plant::Plant(Plant* aPlant) : Plant(aPlant->getParameters()){
	if (aPlant == this) return; // Can't copy ourselves!
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
void Plant::GenerateInternode(Node* parent) {
	// First we determine whether we're growing an apical or lateral bud.
	NodeType tp = NodeType::LATERAL_BUD;
	if (parent->getType() == NodeType::APICAL_BUD && parent->getChildren().size() == 0) {
		// If no children exist already and the parent is apical then this bud is apical
		tp = NodeType::APICAL_BUD;
	}

	float dChance = tp == NodeType::APICAL_BUD ? parameters.ApicalBudExtinction : parameters.LateralBudExtinction;
	float gChance = parent->getGrowthChance() * pow(parameters.InternodeAgeFactor, parameters.t) * 0.7f;

	// Calculate the branch's direction and width
	glm::vec3 branchDirection;
	float w;
	if (tp == NodeType::APICAL_BUD) // Apical bud
	{
		w = parent->getWidth() * getRandomFloat(0.75f, 0.95f);

		// Convert apical angle variance to radians
		float AAVrad = parameters.AAV * glm::pi<float>() / 180.0f;
		// Apical branching angle calculations
		float theta = normalRand(0, AAVrad);
		float phi = uniformRand(0.0f, 2.0f * glm::pi<float>());

		branchDirection = sphericalToCartesian(theta, phi);
	}
	else // Lateral bud
	{
		w = parent->getWidth() * getRandomFloat(0.75f, 0.9f);
		if (w < 0.1f) w = 0.02f;

		// Generate branch roll and angles
		float branchingAngle;
		
		float rollAngle;
		// If we are branching off an apical node we have a wider range of points to branch in
		if (parent->getType() == NodeType::APICAL_BUD) {
			branchingAngle = normalRand(parameters.BAM, parameters.BAV);
			// Convert to radians
			branchingAngle = branchingAngle * glm::pi<float>() / 180.0f;

			rollAngle = uniformRand(0.0f, 2.0f * glm::pi<float>());
		}
		else {
			float AAVrad = parameters.AAV * glm::pi<float>() / 180.0f;
			branchingAngle = -normalRand(0, AAVrad);
			// Convert to radians
			branchingAngle = branchingAngle * glm::pi<float>() / 180.0f;

			// Generate random roll angle
			rollAngle = normalRand(parameters.RAM, parameters.RAV);
			rollAngle = rollAngle * glm::pi<float>() / 180.0f;
		}

		branchDirection = sphericalToCartesian(branchingAngle, rollAngle);
	}

	float branchLength = parameters.InternodeLength * pow(parameters.InternodeAgeFactor, parameters.t);
	fprintf(stdout, "%f %f %f\n", branchDirection.x, branchDirection.y, branchDirection.z);

	// Generate curve positional points
	branchDirection = glm::normalize(branchDirection);
	if (tp == LATERAL_BUD && parent->getType() == LATERAL_BUD)
		branchDirection += parent->getDirection();
	branchDirection = glm::normalize(branchDirection);

	// Bending force calculation
	// Gravitropism
	glm::vec3 horizontalDirection = branchDirection * branchLength;
	horizontalDirection.y = 0;
	glm::vec3 bending = GRAVITY * parameters.Gravitropism * (glm::length(horizontalDirection)) / w;
	if (tp == LATERAL_BUD) {
		branchDirection += bending;
		branchDirection = glm::normalize(branchDirection);
	}

	// Phototropism
	branchDirection += parameters.Phototropism * -glm::normalize(parameters.lightDirection);
	branchDirection = glm::normalize(branchDirection);

	//glm::vec3 location = parent->getPosition() + glm::vec3(getRandomFloat(-0.9f, 0.9f), getRandomFloat(0.4f, 1.0f), getRandomFloat(-0.9f, 0.9f));
	glm::vec3 location = parent->getPosition() + branchDirection * branchLength;
	glm::vec3 control = (parent->getPosition() - location) * -0.5f + glm::vec3(0.0f, .15f, 0.0f);;

	int edges = ResolveEdges(w, parent->getEdges());
	int segments = ResolveSegments(glm::distance(location, parent->getPosition()), parent->getSegments());
	std::cout << "EDGES : " + std::to_string(edges) + "| SEGMENTS : " + std::to_string(segments) << std::endl;
	Node* internode = new Node(tp, location, control, w, branchDirection, edges, segments, this, dChance, gChance);
	parent->AddChild(internode);
}

// Recursive function to perform a growth cycle on a given node and its children
void Plant::SimulateGrowthCycle(Node* node) {
	float r = getRandomFloat(0.0f, 1.0f);

	// Increase branch thickness
	//node->setWidth(node->getWidth() + (parameters.RootWidth * pow(0.95f, parameters.t))*1.3f);
	//float maxWidth = parameters.RootWidth * 3.0f;
	//if (node->getWidth() > maxWidth) node->setWidth(maxWidth);

	if (!node->isDead()) {
		node->setGrowthChance(node->getGrowthChance() * parameters.InternodeAgeFactor * parameters.InternodeAgeFactor);
		node->setDeathChance(node->getDeathChance() / (parameters.InternodeAgeFactor));
		node->setDead(r <= node->getDeathChance()); // Does the node die?
		if (!node->isDead())
			if (getRandomFloat(0.0f, 1.0f) <= node->getGrowthChance()) // Does this node grow a shoot
			{
				// Drastically reduce growth chance after creating a child
				GenerateInternode(node);
				node->setGrowthChance(node->getGrowthChance() * 0.4f);
			}
	}

	if (!node->getChildren().empty()) {
		for (Node* childNode : node->getChildren())
			SimulateGrowthCycle(childNode); // Simulate the children, must be done before growing
	}
}

void Plant::GenerateGraph() {
	parameters.t = 0;

	// If plant has already grown (we are regenerating)
	if (RootNode.getChildren().size() > 0) {
		RootNode.getChildren().clear();
		RootNode.setDead(false);

		// Keeping a pointer to the old one and deleting was crashing so I'm assuming this overwrites the old one
		RootNode = (Node(NodeType::APICAL_BUD, glm::vec3(0.0f), glm::vec3(0.0f, 0.5f, 0.0f), parameters.RootWidth, glm::vec3(0.0f, 1.0f, 0.0f), parameters.RootCircumferenceEdges, parameters.RootCurveSegments, this, parameters.ApicalBudExtinction, parameters.GrowthRate));
	}

	GenerateInternode(&RootNode); // force at least one bud to grow from the root.
	RootNode.setGrowthChance(0.0f);


	// Keep generating until all nodes are dead.
	int growthCycles = 0;
	while (growthCycles < parameters.maxAge) {
		SimulateGrowthCycle(&RootNode);
		parameters.t++; // Increase the tree's age
		growthCycles++;
	}
	std::cout << "Graph Done!" << std::endl;
	return;
}

void Plant::CopyGraph(Plant* copyTarget) {
	//RootNode = *(new Node(&copyTarget->RootNode, this));
	RootNode = copyTarget->RootNode;
}

// Creates a curve by passing in two nodes and the desired circumference subdivisions and number of segments.
Curve makeCurveFromNodes(Node* firstNode, Node* secondNode, int aSubdivisions, int aSegments, bool unionise) {
	bool willUnionise = unionise && firstNode->getPlant()->getParameters().unioniseBranchMeshes;

	// We have to offset to add a slight overlap so that the boolean union operation detects they're connected
	glm::vec3 offset = willUnionise ? (firstNode->getPosition() - secondNode->getPosition()) * 0.04f : glm::vec3(0);

	// If the branch segment has no children then we make the tip have close to zero width (vertices will be cleaned up during merging later)
	float tipWidth = secondNode->getChildren().size() > 0 ? secondNode->getWidth() : 0.0001f;
	//float tipWidth = secondNode->getWidth();

	return Curve(aSubdivisions, aSegments, firstNode->getWidth(), tipWidth,
		firstNode->getPosition() + offset,
		secondNode->getPosition(),
		firstNode->getControlPoint(),
		-secondNode->getControlPoint(), // This is inverted to make sure that connected curves match correctly
		willUnionise); // Only add caps to union meshes
}

// this function will recursively append curves created from segments to a passed vector
void addCurvesFromNode(Node* node, std::vector<Curve>& curves, std::vector<bool>& unionCurve, int aEdgeReduction = 0, int aSegmentReduction = 0) {
	if (node != nullptr && !node->getChildren().empty()) {
		PlantParameters pParams = node->getPlant()->getParameters();

		int index = 0;
		for (Node* childNode : node->getChildren()) {
			int edges = node->getEdges() - aEdgeReduction;
			edges = edges < 3 ? edges = 3 : edges; // Clamp to the minimum
			int segments = node->getSegments() - aSegmentReduction;
			segments = segments < 1 ? segments = 1 : segments; // Clamp to the minimum
			curves.push_back(makeCurveFromNodes(node, childNode, edges, segments, (index != 0)));
			unionCurve.push_back(index != 0);
			addCurvesFromNode(childNode, curves, unionCurve, aEdgeReduction, aSegmentReduction);

			index++;
		}
	}
}

void GenerateFoliage(Node* node, Mesh &baseMesh, int foliageType) {
	// Foliage Data
	std::vector<Texture> FoliageTextures
	{
		Texture("textures/bark_0.png", "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE)
	};

	std::vector<Vertex> planeVertices =
	{ //               COORDINATES           /           NORMALS         /            COLORS          /       TEXTURE COORDINATES    //
		Vertex{glm::vec3(-1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)}
	};

	std::vector<GLuint> planeIndices =
	{
		0, 1, 2,
		0, 2, 3
	};

	std::vector<Vertex> domeVertices =
	{ //               COORDINATES           /           NORMALS         /            COLORS          /       TEXTURE COORDINATES    //
		// Base - 0
		Vertex{glm::vec3(-1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(-0.6f, 0.6f)},
		Vertex{glm::vec3(-1.5f, 0.0f,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(-1.0f, 0.0f)},
		Vertex{glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(-0.6f, -0.6f)},
		Vertex{glm::vec3(-0.0f, 0.0f, -1.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, -1.0f)},
		Vertex{glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.6f, -0.6f)},
		Vertex{glm::vec3(1.5f, 0.0f, -0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.6f, 0.6f)},
		Vertex{glm::vec3(0.0f, 0.0f,  1.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

		// Second Level - 8
		Vertex{glm::vec3(-0.5f, 0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(-.3f, .3f)},
		Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(-.3f, -.3f)},
		Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(.3f, -.3f)},
		Vertex{glm::vec3(0.5f, 0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(.3f, .3f)},

		// Tip - 12
		Vertex{glm::vec3(0.0f, 0.65f,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)}
	};

	std::vector<GLuint> domeIndices =
	{
		0, 8, 1,
		1, 8, 9,
		1, 9, 2,

		2, 9, 3,
		3, 9, 10,
		3, 10, 4,

		4, 10, 5,
		5, 10, 11,
		5, 11, 6,

		6, 11, 7,
		7, 11, 8,
		7, 8, 0,

		// Cap
		12, 9, 8,
		12, 10, 9,
		12, 11, 10,
		12, 8, 11
	};

	if (node != nullptr && !node->getChildren().empty()) {
		// This node has children, unviable for foliage

		PlantParameters pParams = node->getPlant()->getParameters();
		for (Node* childNode : node->getChildren()) {
			GenerateFoliage(childNode, baseMesh, foliageType);
		}
	}
	else {
		std::vector<Vertex>* verts;
		std::vector<GLuint>* ind;
		switch (foliageType)
		{
		case 1:
			verts = &planeVertices;
			ind = &planeIndices;
			break;
		case 2:
			verts = &domeVertices;
			ind = &domeIndices;
			break;
		default:
			return;
		}

		float scale = 0.5f;
		glm::vec3 offset = glm::vec3(0.0f, -0.2f, 0.0f);
		for (int i = 0; i < verts->size(); i++) {
			(*verts)[i].position *= scale;
			(*verts)[i].position += offset;
			(*verts)[i].position += node->getPosition();
		}
		// This node is an end node, it is viable for foliage
		Mesh foliageMesh = Mesh(*verts, *ind, FoliageTextures);
		baseMesh.Concatenate(foliageMesh);
	}
}

void Plant::GenerateMesh(int aEdgeReduction, int aSegmentReduction) {
	if (this->curves.size() > 0) {
		this->curves.clear();
		this->unionCurve.clear();
	}
	// Clear our previous mesh
	this->Delete();

	if (&RootNode != nullptr)
		addCurvesFromNode(&RootNode, this->curves, this->unionCurve, aEdgeReduction, aSegmentReduction);

	// Concatenate all curve meshes to our main plant mesh
	
	int index = 0;
	for (Curve& curve : this->curves)
	{
		this->Concatenate(curve, this->unionCurve[index] && parameters.unioniseBranchMeshes);
		// Merge after every connection rather than at the end so that mesh cutting is cleaner and easier
		MergeVerticesByDistance(0.03f);

		index++;
	}
	if (&RootNode != nullptr)
		GenerateFoliage(&RootNode, *this, parameters.foliageType);
	RecalculateNormals();

	
	/*if (curves.size() > 2) {
		std::cout << "Attempting to concatenate" << std::endl;
		this->Concatenate(curves[0]);
		this->Concatenate(curves[1]);
		this->Concatenate(curves[2]);
	}*/
}