#ifndef _H_POINT_H_
#define _H_POINT_H_

template <typename T>
class CPoint
{
public:
	
	CPoint() {}
	
	CPoint (T newX, T newY)
	{
		SetPoint(newX, newY);
	}
	
	void SetPoint(T newX, T newY)
	{
		x = newX;
		y = newY;
	}

	//is the <T> needed?
	int operator==(const CPoint<T>& point)
	{
		return (point.x==x && point.y == y);
	}

	T x;
	T y;
};

typedef CPoint<int>		CPointInt;
typedef CPoint<float>	CPointFloat;

#endif	//_H_POINT_H_

