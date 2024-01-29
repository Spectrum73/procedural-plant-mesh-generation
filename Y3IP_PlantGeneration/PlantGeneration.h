#pragma once
#include <vmmlib/vector.hpp>
using namespace vmml;

/// <summary>
/// Enumeration for types of nodes in the graph, 
/// SEGMENT refers to a trunk or stem.
/// </summary>
enum NodeType {
	NONE = 0,
	SEGMENT = 1,
	LEAF = 2,
};

/// <summary>
/// Class for nodes in a plant's graph representation.
/// I use classes as some node types may wish to inherit from a base node
/// which I cannot do with a struct.
/// </summary>
class Node {
public:
	int level;
	NodeType type;
	vec3f localPosition;
	vec3f eulerRotation;
	Node(int lv, NodeType tp, vec3f lp, vec3f er) {
		level = lv;
		type = tp;
		localPosition = lp;
		eulerRotation = er;
	}
};

/// <summary>
/// Class for a segment (Stem or Trunk)
/// detail refers to edges around the circumference
/// </summary>
class SegmentNode : Node {
public:
	int detail;
	SegmentNode(int dt) : Node{ level, type, localPosition, eulerRotation } {
		detail = dt;
	}
};

/// <summary>
/// A plant is a template of parameters used to generate an instance made of nodes.
/// TODO - Many of these are placeholders right now and here to demonstrate examples
/// Many potential parameters can be found at: https://onlinelibrary.wiley.com/doi/full/10.1111/cgf.12282?saml_referrer
/// </summary>
struct Plant {
	float ApicalAngleVariance; // Variance of the angular difference between the growth direction and the direction of the apical bud.
	int NbLateralBuds; // The number of lateral buds that are created per each node of a growing shoot.
	float ExtinctionRate; // The probability that a given bud will die during a single growth cycle.
	float GrowthRate; // The number of internodes generated on a single shoot during one growth cycle.
};