#ifndef BOOLEAN_UNION_H
#define BOOLEAN_UNION_H

#include "Mesh.h"
#include <algorithm>
#include "mcut/include/mcut/mcut.h"

#define my_assert(cond)                                                                            \
	if(!(cond))                                                                                    \
	{                                                                                              \
		fprintf(stderr, "MCUT error: %s\n", #cond);                                                \
		return 1;                                                                        \
	}

// Returns 0 on success, anything else on a failure
int BooleanUnion(Mesh& aMesh1, Mesh& aMesh2)
{
    if (aMesh1.vertices.size() == 0)
    {
        aMesh1 = aMesh2;
        return 0;
    }
    else if (aMesh2.vertices.size() == 0) return 0;

    // Mesh 1 Conversions
    std::vector<float> pos1Vector;
    for (int i = 0; i < aMesh1.vertices.size(); i++) {
        pos1Vector.push_back(aMesh1.vertices[i].position.x);
        pos1Vector.push_back(aMesh1.vertices[i].position.y);
        pos1Vector.push_back(aMesh1.vertices[i].position.z);
    }
    std::vector<McUint32> m1uint_indices(aMesh1.indices.size());
    std::transform(aMesh1.indices.begin(), aMesh1.indices.end(), m1uint_indices.begin(),
        [](int value) { return static_cast<McUint32>(value); });

    // Mesh 2 Conversions
    std::vector<float> pos2Vector;
    for (int i = 0; i < aMesh2.vertices.size(); i++) {
        pos2Vector.push_back(aMesh2.vertices[i].position.x);
        pos2Vector.push_back(aMesh2.vertices[i].position.y);
        pos2Vector.push_back(aMesh2.vertices[i].position.z);
    }
    std::vector<McUint32> m2uint_indices(aMesh2.indices.size());
    std::transform(aMesh2.indices.begin(), aMesh2.indices.end(), m2uint_indices.begin(),
        [](int value) { return static_cast<McUint32>(value); });

    // Create an MCUT context
    McContext context = MC_NULL_HANDLE;
    McResult status = mcCreateContext(&context, MC_NULL_HANDLE);

    my_assert(status == MC_NO_ERROR);

    ///
    ///
    /// 
    
    McFloat cubeVertices[] = {
    -5, -5, 5, // 0
    5, -5, 5, // 1
    5, 5, 5, //2
    -5, 5, 5, //3
    -5, -5, -5, //4
    5, -5, -5, //5
    5, 5, -5, //6
    -5, 5, -5 //7
    };
    McUint32 cubeFaces[] = {
        0, 1, 2, 3, //0
        7, 6, 5, 4, //1
        1, 5, 6, 2, //2
        0, 3, 7, 4, //3
        3, 2, 6, 7, //4
        4, 5, 1, 0 //5
    };
    McUint32 cubeFaceSizes[] = {
        4, 4, 4, 4, 4, 4
    };
    McUint32 numCubeVertices = 8;
    McUint32 numCubeFaces = 6;

    McFloat cutMeshVertices[] = {
        -20, -4, 0, //0
        0, 20, 20, //1
        20, -4, 0, //2
        0, 20, -20 //3
    };
    McUint32 cutMeshFaces[] = {
        0, 1, 2, //0
        0, 2, 3 //1
    };
    McUint32 cutMeshFaceSizes[] = {
        3, 3
    };
    McUint32 numCutMeshVertices = 4;
    McUint32 numCutMeshFaces = 2;

    // Boolean Union
    status = mcDispatch(
        context,
        MC_DISPATCH_VERTEX_ARRAY_FLOAT | MC_DISPATCH_ENFORCE_GENERAL_POSITION |
        MC_DISPATCH_FILTER_FRAGMENT_SEALING_OUTSIDE | MC_DISPATCH_FILTER_FRAGMENT_LOCATION_ABOVE /*Boolean Union Flags*/,
        //cubeVertices,
        //cubeFaces,
        //cubeFaceSizes,
        //numCubeVertices,
        //numCubeFaces,
        //cutMeshVertices,
        //cutMeshFaces,
        //nullptr,
        //numCutMeshVertices,
        //numCutMeshFaces);
        pos1Vector.data(),
        m1uint_indices.data(),
        nullptr,
        static_cast<McUint32>(pos1Vector.size()/3),
        static_cast<McUint32>(m1uint_indices.size()/3),
        pos2Vector.data(),
        m2uint_indices.data(),
        nullptr,
        static_cast<McUint32>(pos2Vector.size()/3),
        static_cast<McUint32>(m2uint_indices.size()/3));
    my_assert(status == MC_NO_ERROR);

    McUint32 numConnComps;
    std::vector<McConnectedComponent> connComps;
    // MC_CONNECTED_COMPONENT_TYPE_FRAGMENT
    status = mcGetConnectedComponents(context, MC_CONNECTED_COMPONENT_TYPE_FRAGMENT, 0, NULL, &numConnComps);
    my_assert(status == MC_NO_ERROR);

    fprintf(stdout, "COMPONENTS: %d\n", numConnComps);
    if (numConnComps == 0) {
        fprintf(stdout, "no connected components found\n");
        return 1;
        //exit(EXIT_FAILURE);
    }

    connComps.resize(numConnComps);
    status = mcGetConnectedComponents(context, MC_CONNECTED_COMPONENT_TYPE_FRAGMENT, (McUint32)connComps.size(), connComps.data(), NULL);
    my_assert(status == MC_NO_ERROR);

    McConnectedComponent connComp = connComps[0]; // connected component handle.
    McSize numBytes = 0; // number of bytes we must allocate.

    // query vertices
    status = mcGetConnectedComponentData(context, connComp, MC_CONNECTED_COMPONENT_DATA_VERTEX_FLOAT, 0, NULL, &numBytes);
    my_assert(status == MC_NO_ERROR);
    std::vector<McFloat> vertices;
    vertices.resize(numBytes / sizeof(McFloat));
    status = mcGetConnectedComponentData(context, connComp, MC_CONNECTED_COMPONENT_DATA_VERTEX_FLOAT, numBytes, (McVoid*)vertices.data(), NULL);
    my_assert(status == MC_NO_ERROR);

    numBytes = 0;

    // query indices
    status = mcGetConnectedComponentData(context, connComp, MC_CONNECTED_COMPONENT_DATA_FACE_TRIANGULATION, 0, NULL, &numBytes);
    my_assert(status == MC_NO_ERROR);
    std::vector<McUint32> faceIndices;
    faceIndices.resize(numBytes / sizeof(McUint32));
    status = mcGetConnectedComponentData(context, connComp, MC_CONNECTED_COMPONENT_DATA_FACE_TRIANGULATION, numBytes, (McVoid*)faceIndices.data(), NULL);
    my_assert(status == MC_NO_ERROR);

    numBytes = 0;

    // Free resources
    mcReleaseConnectedComponents(context, 0, NULL);
    mcReleaseContext(context);

    // Convert back to a Mesh class
    aMesh1.indices.clear();
    aMesh1.vertices.clear();
    aMesh2.indices.clear();
    aMesh2.vertices.clear();

    for (int i = 0; i < faceIndices.size(); i++)
    {
        aMesh1.indices.push_back(static_cast<int>(faceIndices[i]));
    }

    for (int i = 0; i < vertices.size(); i += 3) {
        Vertex vert;
        vert.position = glm::vec3(vertices[i], vertices[i+1], vertices[i+2]);
        vert.normal = glm::vec3(0);
        aMesh1.vertices.push_back(vert);
    }

    return 0;
}
#endif