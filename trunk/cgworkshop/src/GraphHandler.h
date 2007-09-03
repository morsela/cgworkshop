#ifndef __GRAPH_HANDLER_H__
#define __GRAPH_HANDLER_H__

#include <map>
#include "MaxFlow/graph.h"
#include <cv.h>

using namespace std;


class GraphHandler {

public:
	
	GraphHandler();
	
	~GraphHandler();

public:
 
	void init_graph(int height, int width, CvMat *smoothness);

	double static calc_beta(int height, int width, CvMat* smoothness);

	void assign_weights(CvMat * Bu, CvMat * Fu);
	

public:
	void do_MinCut(CvMat & result);

public:
	Graph::flowtype getFlow() const {return m_flow;};
	


private:


	Graph *m_igraph;
	double static beta;
	Graph::flowtype m_flow;
	map<int ,map< int, Graph::node_id> > m_nodes;


};

#endif // __GRAPH_HANDLER_H__
