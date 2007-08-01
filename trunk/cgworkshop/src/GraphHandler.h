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

	void init_graph(int height, int width, CvMat *smoothness, CvMat * pDoubleMask);

	double static calc_beta(int height, int width, CvMat* smoothness);

	void assign_weights(CvMat * Bu, CvMat * Fu, CvMat * pDoubleMask);
	
	double get_total_flow(CvMat * segmentation);
	

public:
	void do_MinCut(CvMat & result, CvMat * pDoubleMask);

public:
	Graph::flowtype getFlow() const {return m_flow;};
	


private:

	int m_width, m_height;

	Graph *m_igraph;
	double static beta;
	Graph::flowtype m_flow;
	map<int ,map< int, Graph::node_id> > m_nodes;


};

#endif // __GRAPH_HANDLER_H__
