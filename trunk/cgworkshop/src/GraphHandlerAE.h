#ifndef __GRAPH_HANDLER_AE_H__
#define __GRAPH_HANDLER_AE_H__

#include <map>
#include "MaxFlow/graph.h"
#include <cv.h>

using namespace std;


class CGraphHandlerAE {

public:
	CGraphHandlerAE(int height, int width, CvMat *pSmooth, double alpha_int);
	~CGraphHandlerAE();

public:

	void DoAlphaExpansion(int aLabel, CvMat * pCurrLabel, CvMat * pResLabel, CvMat ** pProbArr);

public:

public:
	Graph::flowtype getFlow() const {return m_flow;};
	double CalcEnergy(CvMat * pCurrLabel,CvMat ** pProbArr);
	
protected:
	void init();
	double calc_beta();
	double calcDist(int i, int j);
	double getDist(int i, int j);
	
	void handleNeighbor(int aLabel, CvMat * pCurrLabel, int pi,int pj, int qi, int qj);


private:

	int 		m_nWidth;
	int 		m_nHeight;

	double		m_alpha;
	double		m_beta;

	CvMat * 	m_pSmooth;

	Graph * 	m_pGraph;
	
	Graph::flowtype m_flow;
	map<int ,map< int, Graph::node_id> > m_nodes;
	
	map<int ,map< int ,map< int ,map< int, double> > > > m_nLinks;


};

#endif // __GRAPH_HANDLER_AE_H__
