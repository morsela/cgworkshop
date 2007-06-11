#ifndef _H_POINT_H_
#define _H_POINT_H_


struct CPoint
{
	CPoint() {}
	
	CPoint (float newX, float newY)
	{
		SetPoint(newX, newY);
	}
	
	void SetPoint(float newX, float newY)
	{
		x = newX;
		y = newY;
	}

	float x;
	float y;
};


#endif	//_H_POINT_H_

