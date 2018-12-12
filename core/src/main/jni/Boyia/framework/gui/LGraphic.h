/*
============================================================================
 Name        : lgraphic.h
 Author      : Yanbo
 Version     :
 Copyright   : Your copyright notice
 Description : lgraphic.h - header file
============================================================================
*/

#ifndef LGraphic_h
#define LGraphic_h

#include "PlatformLib.h"

//
namespace util
{
	template <class T> class LPoint_t;
	template <class T> class LSize_t;
	template <class T> class LRect_t;

	template <class T>
	class LPoint_t
	{
	public:
		T iX;
		T iY;

	public:
		LPoint_t(){}
		LPoint_t(const LPoint_t<T>& point){ iX = point.iX; iY = point.iY; }
	    LPoint_t(T aX, T aY) { iX = aX; iY = aY; }

	public:
		LVoid Set(T aX, T aY) { iX = aX; iY = aY; }
		const LPoint_t<T>& operator = (const LPoint_t<T>& aPoint)
		{
			iX = aPoint.iX;
			iY = aPoint.iY;
			return *this;
		}

		LBool operator == (const LPoint_t<T>& aPoint)
		{
            return iX == aPoint.iX && iY == aPoint.iY;
		}
	};

	template <class T>
	class LSize_t
	{
	public:
		T iWidth;
		T iHeight;

	public:
		LSize_t() {}
		LSize_t(const LSize_t<T>& size) { iWidth = size.iWidth; iWidth = size.iHeight; }
		LSize_t(T aWidth, T aHeight) { iWidth = aWidth; iHeight = aHeight; }

	public:
		LVoid Set(T aWidth, T aHeight) { iWidth = aWidth; iHeight = aHeight; }
		const LSize_t<T>& operator = (const LSize_t<T>& aPoint)
		{
			iHeight = aPoint.iHeight;
			iWidth = aPoint.iWidth;
			return *this;
		}
	};

	template <class T>
	class LRect_t
	{
	public:
		LPoint_t<T> iTopLeft;
		LPoint_t<T> iBottomRight;

	public:
		LRect_t(){}
		LRect_t(const LRect_t<T>& rect) { iTopLeft = rect.iTopLeft; iBottomRight = rect.iBottomRight; }
		LRect_t(LPoint_t<T>& aTopLeft, LPoint_t<T>& aBottomRight)
		{
            iTopLeft = aTopLeft;
			iBottomRight = aBottomRight;
		}

		LRect_t(T x, T y, T w, T h) { iTopLeft = LPoint_t<T>(x, y);  iBottomRight = LPoint_t<T>(x+w, y+h); }

		LRect_t(const LPoint_t<T>& aTopLeft, const LSize_t<T>& aSize)
		{
            iTopLeft.Set(aTopLeft.iX, aTopLeft.iY);
            //iTopLeft.iX = aTopLeft.iX;
            //iTopLeft.iY = aTopLeft.iY;
			//iTopLeft = aTopLeft;

            iBottomRight.iX = aTopLeft.iX + aSize.iWidth;
            iBottomRight.iY = aTopLeft.iY + aSize.iHeight;
		}

	public:
		LVoid Set(const LPoint_t<T>& aTopLeft, const LPoint_t<T>& aBottomRight)
		{
			iTopLeft = aTopLeft;
			iBottomRight = aBottomRight;
		}

		LVoid Set(const LPoint_t<T>& aTopLeft, const LSize_t<T>& aSize)
		{
			iTopLeft = aTopLeft;
            iBottomRight.iX = aTopLeft.iX + aSize.iWidth;
            iBottomRight.iX = aTopLeft.iY + aSize.iHeight;
		}

		const LRect_t<T>& operator = ( const LRect_t<T>& aRect )
		{
			iTopLeft = aRect.iTopLeft;
			iBottomRight = aRect.iBottomRight;
			return *this;
		}

		T GetHeight() const { return iBottomRight.iY - iTopLeft.iY; }
		T GetWidth() const  { return iBottomRight.iX - iTopLeft.iX; }
		LSize_t<T> GetSize() const { return LSize_t<T>(iBottomRight.iY - iTopLeft.iY,iBottomRight.iX - iTopLeft.iX); }
	};

	typedef LPoint_t<LInt> LPoint;
	typedef LSize_t<LInt> LSize;
	typedef LRect_t<LInt> LRect;
}

using util::LRect;
using util::LSize;
using util::LPoint;

#endif // LGRAPHIC_H

// End of File
