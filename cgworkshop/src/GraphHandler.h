#pragma once
#include <map>
#include "MaxFlow/graph.h"
#include <cv.h>

using namespace std;


class GraphHandler {

	static Graph initial_graph;


public:

	GraphHandler();
	
	~GraphHandler();

public:

	void init_graph(int height, int width, CvMat *smoothness);

	void assign_weights(CvMat * Tu, CvMat * Su);
	
	void do_MinCut(CvMat & result);

	Graph::flowtype getFlow() const {return m_flow;};
	


private:


	Graph *m_igraph;
	Graph::flowtype m_flow;
	map<int ,map< int, Graph::node_id>> nodes;


};