#include "GraphHandler.h"

double GraphHandler::beta = 0;

GraphHandler::GraphHandler()  {

	//should copy the initial graph using copy c'tor
	m_igraph = new Graph();
}

GraphHandler::~GraphHandler() {
	delete m_igraph;
}

double getDist(CvMat * smoothness, int i, int j) {
	double result=0;
	
	for (int k = 0; k<3; k++)
			result += pow(cvmGet(smoothness, i, k) - cvmGet(smoothness, j, k),2);
	
	return result;
}

double calcDist(CvMat * smoothness, int i, int j, double beta) {
	
	double alpha = 25;
	return alpha*exp(-getDist(smoothness,i,j)/beta);
}

double GraphHandler::calc_beta(int rows, int cols, CvMat* smoothness) {

 	double sum = 0;
 	int count = 0;
	for(int i = 0; i < rows; i ++)
	{
		for (int j = 0; j < cols; j ++) 
		{
			if (i<rows-1)
			{
				sum += getDist(smoothness, i*cols +j, (i+1)*cols +(j+0));
				count++;
			}
			
			if (j<cols-1)
			{
				sum += getDist(smoothness, i*cols +j, (i+0)*cols +(j+1));
				count++;
			}
			/*
			if ((i<rows-1) && (j<cols-1))
			{
				sum += getDist(smoothness, i*cols +j, (i+1)*cols +(j+1));
				count++;
			}
			
			if ((i>0) && (j<cols-1))
			{
				sum += getDist(smoothness, i*cols +j, (i-1)*cols +(j+1));
				count++;
			}
			*/
		}
	}
	
	beta = (sum/count) * 10;
	printf("Calculated beta to be: %f\n", beta);
	return beta;

}


void GraphHandler::init_graph(int rows, int cols, CvMat * smoothness) {
	
	//create m_nodes
	for (int i = 0; i <rows; i++) 
		for (int j = 0; j < cols; j++)
			m_nodes[i][j] = m_igraph->add_node();

	for(int i = 0; i < rows; i ++)
	{
		for (int j = 0; j < cols; j ++) 
		{
			if (j<cols-1)	
				m_igraph->add_edge(m_nodes[i][j], m_nodes[i+0][j+1], calcDist(smoothness, i*cols+j, (i+0)*cols+(j+1), beta));				

			if (i<rows-1)
				m_igraph->add_edge(m_nodes[i][j], m_nodes[i+1][j+0], calcDist(smoothness, i*cols+j, (i+1)*cols+(j+0), beta));				
/*
			if ((i<rows-1) && (j<cols-1))
				m_igraph->add_edge(m_nodes[i][j], m_nodes[i+1][j+1], calcDist(smoothness, i*cols+j, (i+1)*cols+(j+1), beta));
								
			if ((i>0) && (j<cols-1))
				m_igraph->add_edge(m_nodes[i][j], m_nodes[i-1][j+1], calcDist(smoothness, i*cols+j, (i-1)*cols+(j+1), beta));
*/
		}
	}

}



void GraphHandler::assign_weights(CvMat *Bu, CvMat *Fu) {

	for (int i=0; i<Bu->rows; i++)
		for (int j=0; j<Bu->cols; j++) {
			//add the Sink E1 term
			m_igraph->set_tweights(m_nodes[i][j], 
										cvmGet(Fu,i,j),
										cvmGet(Bu,i,j));
														
	}


}

void GraphHandler::do_MinCut(CvMat &result) {

	m_flow = m_igraph->maxflow();
	int counter1 = 0;
	int counter2 = 0;


	for(int i=0; i<result.rows; i++)
		for (int j=0; j<result.cols; j++)
			//we can push it all into one line...
			if (m_igraph->what_segment(m_nodes[i][j])==Graph::SINK){
				counter1++;
				cvmSet(&result, i,j,0);
			}
			else {
				cvmSet(&result, i,j,1);
				counter2++;

			}
			
	printf("COUNTER1 %d COUNTER2 %d", counter1, counter2);
}

