#include "GraphHandler.h"

Graph GraphHandler::initial_graph = Graph();

GraphHandler::GraphHandler()  {

	//should copy the initial graph using copy c'tor
	m_igraph = new Graph();
}

GraphHandler::~GraphHandler() {
	delete m_igraph;
}

double calcDist(CvMat * smoothness, int i, int j) {

	double result=0;

	for (int k = 0; k<3; k++)
		result += pow(cvmGet(smoothness, i, k) - cvmGet(smoothness, j, k),2);
	
	return exp(-result);
}

void GraphHandler::init_graph(int rows, int cols, CvMat * smoothness) {

	//map <int, map<int, Graph::node_id>> node;
	
	//create nodes
	for (int i=0; i<rows; i++) 
		for (int j=0; j< cols; j++)
			nodes[i][j] = m_igraph->add_node();

	//need to somehow calculate beta here
	double beta = 10.0;

	
	for(int i=1; i<rows; i+=2)
		for (int j=1; j< cols; j+=2) {
			m_igraph->add_edge(nodes[i][j], nodes[i-1][j-1], calcDist(smoothness, i*cols+j, (i-1)*cols+(j-1)));
			m_igraph->add_edge(nodes[i][j], nodes[i-1][j], calcDist(smoothness, i*cols+j, (i-1)*cols+j));						
			m_igraph->add_edge(nodes[i][j], nodes[i-1][j+1], calcDist(smoothness, i*cols +j, (i-1)*cols +(j+1)));						
			m_igraph->add_edge(nodes[i][j], nodes[i][j-1], calcDist(smoothness, i*cols+j, i*cols+(j-1)));						
			m_igraph->add_edge(nodes[i][j], nodes[i][j+1], calcDist(smoothness, i*cols + j, i*cols + (j+1)));						
			m_igraph->add_edge(nodes[i][j], nodes[i+1][j-1], calcDist(smoothness, i*cols+j, (i+1)*cols+(j-1)));						
			m_igraph->add_edge(nodes[i][j], nodes[i+1][j], calcDist(smoothness, i*cols+j, (i+1)*cols+j));						
			m_igraph->add_edge(nodes[i][j], nodes[i+1][j+1], calcDist(smoothness, i*cols+j, (i+1)*cols+(j+1)));				
		}

}



void GraphHandler::assign_weights(CvMat *Tu, CvMat *Su) {

	for (int i=0; i<Tu->rows; i++)
		for (int j=0; j<Tu->cols; j++) {
			//add the Sink E1 term
			m_igraph->set_tweights(nodes[i][j], 
										cvmGet(Su,i,j),
										cvmGet(Tu,i,j));
	}


}

void GraphHandler::do_MinCut(CvMat &result) {

	m_flow = m_igraph->maxflow();
	int counter1 = 0;
	int counter2 = 0;


	for(int i=0; i<result.rows; i++)
		for (int j=0; j<result.cols; j++)
			//we can push it all into one line...
			if (m_igraph->what_segment(nodes[i][j])==Graph::SINK){
				counter1++;
				cvmSet(&result, i,j,0);
			}
			else {
				cvmSet(&result, i,j,1);
				counter2++;
				printf("%d %d \n", i, j);
			}
			
	printf("COUNTER1 %d COUNTER2 %d", counter1, counter2);
}