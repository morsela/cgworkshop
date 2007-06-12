#ifndef _H_POINT_H_
#define _H_POINT_H_


//TODO: you know what it is, you want it so bad -> TEMPLATES! woohoo!
struct CPointFloat
{
	CPointFloat() {}
	
	CPointFloat (float newX, float newY)
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

struct CPointInt
{
	CPointInt() {}
	
	CPointInt (int newX, int newY)
	{
		SetPoint(newX, newY);
	}
	
	void SetPoint(int newX, int newY)
	{
		x = newX;
		y = newY;
	}

	int x;
	int y;
};

#endif	//_H_POINT_H_

