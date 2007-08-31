#include "GraphHandlerAE.h"

extern double alpha;

CGraphHandlerAE::CGraphHandlerAE(int height, int width, CvMat *pSmooth , double alpha_int)  
{

	m_nWidth = width;
	m_nHeight = height;
	m_pSmooth = pSmooth;
	
//	m_alpha = alpha_int;
	m_alpha = alpha;
	
	m_beta = calc_beta();
	
	init();
}

CGraphHandlerAE::~CGraphHandlerAE() {
}

double CGraphHandlerAE::getDist(int i, int j) {
	double result=0;
	
	for (int k = 0; k<3; k++)
			result += pow(cvmGet(m_pSmooth, i, k) - cvmGet(m_pSmooth, j, k),2);
	
	return result;
}

double CGraphHandlerAE::calcDist(int i, int j) {
	
	return m_alpha*exp(-getDist(i,j)/m_beta);
}

double CGraphHandlerAE::calc_beta() 
{
	double beta = 0;
 	double sum = 0;
 	int count = 0;
	for(int i = 0; i < m_nHeight; i ++)
	{
		for (int j = 0; j < m_nWidth; j ++) 
		{
			if (i<m_nHeight-1)
			{
				sum += getDist(i*m_nWidth +j, (i+1)*m_nWidth +(j+0));
				count++;
			}
			
			if (j<m_nWidth-1)
			{
				sum += getDist(i*m_nWidth +j, (i+0)*m_nWidth +(j+1));
				count++;
			}
		}
	}
	
	beta = (sum/count) * 10;
	printf("Calculated beta to be: %f\n", beta);
	return beta;

}


void CGraphHandlerAE::init() 
{

	for(int i = 0; i < m_nHeight; i ++)
	{
		for (int j = 0; j < m_nWidth; j ++) 
		{
			if (j<m_nWidth-1)
				m_nLinks[i][j][i][j+1] = calcDist(i*m_nWidth+j, (i+0)*m_nWidth+(j+1));			

			if (i<m_nHeight-1)
				m_nLinks[i][j][i+1][j] = calcDist(i*m_nWidth+j, (i+1)*m_nWidth+(j+0));
		}
	}

}


void CGraphHandlerAE::DoAlphaExpansion(int aLabel, CvMat * pCurrLabel, CvMat * pResLabel, CvMat ** pProbArr, CvMat * pScribbles)
{
	m_pGraph = new Graph();
	
	// Add initial nodes, t-links
	double source_w, sink_w;
	for (int i=0;i<m_nHeight;i++)
	{
		for (int j = 0; j < m_nWidth; j ++) 
		{
			
			m_nodes[i][j] = m_pGraph->add_node();
			
			if (cvmGet(pScribbles, i, j) > -1 && cvmGet(pScribbles, i, j) != aLabel)
				source_w = 10000;
			else	
				source_w = cvmGet(pProbArr[aLabel], i,j);
			
			int label = (int) cvmGet(pCurrLabel,i,j);
			
			if ((label == aLabel) || (cvmGet(pScribbles, i, j) == aLabel))
				sink_w = 10000; // inf
			else
				sink_w = cvmGet(pProbArr[label], i,j);

			m_pGraph->set_tweights(m_nodes[i][j], source_w, sink_w);
		}			
	}
	
	// Add aux nodes, n-links

	for(int i = 0; i < m_nHeight; i ++)
	{
		for (int j = 0; j < m_nWidth; j ++) 
		{
			if (j<m_nWidth-1)
				handleNeighbor(aLabel, pCurrLabel, i,j, i, j+1);		

			if (i<m_nHeight-1)
				handleNeighbor(aLabel, pCurrLabel, i,j, i+1, j);
		}
	}

	// Run max flow
	double m_flow = m_pGraph->maxflow();
	
	int counter1=0, counter2=0;
	// Find new labeling
	for(int i=0; i<m_nHeight; i++)
	{
		for (int j=0; j<m_nWidth; j++)
		{
			if (m_pGraph->what_segment(m_nodes[i][j])==Graph::SINK)
			{
				counter1++;
				//printf("ALPHA:: Giving (%d,%d) label=%d\n", i,j,aLabel);
				cvmSet(pResLabel, i,j, (float)aLabel);
			}
			else 
			{
				counter2++;
				cvmSet(pResLabel, i,j, cvmGet(pCurrLabel,i,j));
				//printf("OLD:: Giving (%d,%d) label=%f\n", i,j,cvmGet(pCurrLabel,i,j));
			}
		}
	}		
	
	printf("COUNTER1 %d COUNTER2 %d\n\n", counter1, counter2);	
	
	delete m_pGraph;
}


void CGraphHandlerAE::handleNeighbor(int aLabel, CvMat * pCurrLabel, int pi,int pj, int qi, int qj)
{
	int pLabel = (int) cvmGet(pCurrLabel,pi,pj);
	int qLabel = (int) cvmGet(pCurrLabel,qi,qj);
	
	double weight = m_nLinks[pi][pj][qi][qj];
	
	if (pLabel == qLabel)
	{
			if (pLabel == aLabel)
				m_pGraph->add_edge(m_nodes[pi][pj], m_nodes[qi][qj], 0);
			else
				m_pGraph->add_edge(m_nodes[pi][pj], m_nodes[qi][qj], weight);	
	}
	
	else
	{
		Graph::node_id aux_node = m_pGraph->add_node();
		
		if (pLabel == aLabel)
			m_pGraph->add_edge(m_nodes[pi][pj], aux_node, 0);
		else
			m_pGraph->add_edge(m_nodes[pi][pj], aux_node, weight);
			
		if (qLabel == aLabel)
			m_pGraph->add_edge(aux_node, m_nodes[qi][qj], 0);
		else
			m_pGraph->add_edge(aux_node, m_nodes[qi][qj], weight);
			
		m_pGraph->set_tweights(aux_node, 0, weight);	
	}
}

double CGraphHandlerAE::CalcEnergy(CvMat * pCurrLabel, CvMat ** pProbArr)
{
	double E1 =0.0, E2 = 0.0;
	
	for(int i=0; i<m_nHeight; i++)
	{
		for (int j=0; j<m_nWidth; j++)
		{
			int label1 = (int) cvmGet(pCurrLabel, i, j);
			int label2;


			if (j<m_nWidth-1)
			{
				label2 = cvmGet(pCurrLabel, i, j+1);
				if (label1 != label2)
					E2 += m_nLinks[i][j][i][j+1];		
			}
			if (i<m_nHeight-1)
			{
				label2 = cvmGet(pCurrLabel, i+1, j);
				if (label1 != label2)
					E2 += m_nLinks[i][j][i+1][j];				
			}
			
			E1 += cvmGet(pProbArr[label1], i,j);
		}
	}	
	printf("E1=%lf, E2=%lf, energy=%lf\n", E1, E2, E1+E2);
	return E1+E2;
}

