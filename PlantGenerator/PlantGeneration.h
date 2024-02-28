#ifndef PLANT_GENERATION_H
#define PLANT_GENERATION_H

#include <glm/glm.hpp>
#include <vector>
#include "Curve.h"
#include "Camera.h"
#include "RandomNumberGeneration.h"

/// <summary>
/// Enumeration for types of nodes in the graph, 
/// SEGMENT refers to a trunk or stem.
/// </summary>
enum NodeType {
	NONE = 0,
	APICAL_BUD = 1, // On main stem
	LATERAL_BUD = 2, // for branches
	SEGMENT = 3,
	LEAF = 4,
};

// Forward declaration
class Plant;

/// <summary>
/// Class for nodes in a plant's graph representation.
/// I use classes as some node types may wish to inherit from a base node
/// which I cannot do with a struct.
/// </summary>
class Node {
public:
	Node(NodeType aType, glm::vec3 aLocalPosition, glm::vec3 aControlPoint, float aWidth, int aEdges, int aSegments, Plant* aPlant, float aDeathChance, float aGrowthChance, Node* parent = NULL) {
		type = aType;
		localPosition = aLocalPosition;
		controlPoint = aControlPoint;
		width = aWidth;
		circumferenceEdges = aEdges;
		curveSegments = aSegments;
		growthChance = aGrowthChance;
		deathChance = aDeathChance;
		dead = false;
		plant = aPlant;
		if (parent != NULL)
			parent->AddChild(this);
	}
	Node(Node* copyNode, Plant* newPlant) : Node(copyNode->type, copyNode->localPosition, copyNode->controlPoint, copyNode->width, copyNode->circumferenceEdges, copyNode->curveSegments, newPlant, copyNode->deathChance, copyNode->growthChance) {
		for (int i = 0; i < copyNode->children.size(); i++) {
			children.push_back(new Node(copyNode->children[i], newPlant));
		}
	}
	virtual ~Node() {
		for (int i = 0; i < children.size(); i++) {
			delete children[i];
		}
		children.clear();
	} // allows polymorphic behaviour
	void AddChild(Node* child) { if (child != nullptr) children.push_back(child); }
	std::vector<Node*>& getChildren() { return(children); }
	int getNbChildren() { return (int)(children.size() / sizeof(Node*)); }
	bool isDead() { return dead; }
	void setDead(bool status) { dead = status; }
	float getDeathChance() { return deathChance; }
	void setDeathChance(float chance) { deathChance = chance; }
	float getGrowthChance() { return growthChance; }
	void setGrowthChance(float chance) { growthChance = chance; }
	glm::vec3 getPosition() { return localPosition; }
	float getWidth() { return width; }
	glm::vec3 getControlPoint() { return controlPoint; }
	Plant* getPlant() { return plant; }
	NodeType getType() { return type; }
	void setType(NodeType aType) { type = aType; }
	int getEdges() { return circumferenceEdges; }
	int getSegments() { return curveSegments; }
private:
	NodeType type;
	glm::vec3 localPosition;
	float width;
	int circumferenceEdges = 8; // The number of edges per circumference on each curve's mesh
	int curveSegments = 6; // The number of segments in the curve's mesh.
	glm::vec3 controlPoint; // Bezier curve for this node's control point
	std::vector<Node*> children;
	Plant* plant;
	float growthChance;
	float deathChance;
	bool dead;

	friend class SegmentNode;
};

/// <summary>
/// A plant is a template of parameters used to generate an instance made of nodes.
/// TODO - Many of these are placeholders right now and here to demonstrate examples
/// Many potential parameters can be found at: https://onlinelibrary.wiley.com/doi/full/10.1111/cgf.12282?saml_referrer
/// https://onlinelibrary.wiley.com/doi/epdf/10.1111/cgf.12282
/// </summary>
struct PlantParameters {
	int RootCircumferenceEdges = 8; // The number of edges per circumference on each curve's mesh
	int RootCurveSegments = 6; // The number of segments in the curve's mesh.

	float AAV; // ApicalAngleVariance | Variance of the angular difference between the growth direction and the direction of the apical bud.
	int NLB; // NbLateralBuds | The number of lateral buds that are created per each node of a growing shoot.
	float BAM; // Branching Angle Mean |
	float BAV; // Branching Angle Variance
	float RAM; // Roll Angle Mean
	float RAV; // Roll Angle Variance

	float ApicalBudExtinction; // Probability that a given bud will die during a single growth cycle.
	float LateralBudExtinction;
	float ApicalLightFactor; // Influence of light on the growth probability of a bud.
	float LateralLightFactor;
	float ApicalDominance; // Control over Auxin factors in the plant.
	float ApicalDominanceDistance;
	float ApicalDominanceAgeFactor;
	float GrowthRate; // The number of internodes generated on a single shoot duringone growth cycle.

	float InternodeLength; // The base length of a single internode and its relation to the tree age
	float InternodeAgeFactor;

	float ApicalControlLevel; // The impact of the branch level on the growth rate and its relation to the tree age.
	float ApicalControlAgeFactor;

	float Phototropism; // The impact of the average direction of incoming light andgravity on the growth direction of a shoot.
	float Gravitropism;

	float PruningFactor; // The impact of the amount of incoming light on the shedding of branches.
	float LBPruningFactor; // LowBranchPruningFactor | The height below which all lateral branches are pruned.

	float GravityBendingStrength; // The impact of gravity on branch structural bending and its relation to branch thickness
	float GravityBendingAngle;

	float t; // Growth time. Controls age and size.
};


class Plant : public Mesh {
public:
	Plant(PlantParameters aParams);
	Plant(Plant* aPlant);
	void GenerateGraph();
	void CopyGraph(Plant* copyTarget);
	void GenerateMesh(int aEdgeReduction = 0, int aSegmentReduction = 0); // Generates the mesh with reduced qualities
	void setParameters(const PlantParameters& p) { parameters = p; }
	PlantParameters getParameters() { return parameters; }
	void addChild(Node* node) { RootNode.AddChild(node); }
private:
	bool hasLivingBuds(); // Returns whether any buds are still alive
	// creates an internode connected to the provided parent
	void GenerateInternode(Node* parent, float chanceDecay);
	// Recursive function to perform a growth cycle on a given node and its children
	void SimulateGrowthCycle(Node* node, float chanceDecay);
	PlantParameters parameters;
	Node RootNode;
	std::vector<Curve> curves;
};

#endif