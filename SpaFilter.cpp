#include "SpaFilter.h"
#include <iostream>
#include <assert.h>


namespace  img
{
	int laplaceFilter(Image& _srcImg,Image& _dstImg,int _range)
	{
		float kernal_data[]={1,4,1,4,-20,4,1,4,1};
		Image kernal(3,3,kernal_data);
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
				kernal(i,j)->value/=_range;
		Image temp;
		spaFilter(_srcImg,temp,kernal);
		_dstImg=temp;
		return 0;
	}

	int spaFilter(Image& _srcImg,Image& _dstImg,Image& _kernal)
	{
		Image temp;
		if(_srcImg.getType()==IMG_GRAY)
		{
			convolution(_srcImg,temp,_kernal);
			_dstImg=temp;
			return 0;
		}
		else if(_srcImg.getType()==IMG_COLOR)
		{
			Image t[3];
			split(_srcImg,t[0],t[1],t[2]);
			for(int i=0;i<3;i++)
				convolution(t[i],t[i],_kernal);
			merge(t[0],t[1],t[2],temp);
			_dstImg=temp;
			return 0;
		}
		return 1;
	}

	int convolution(Image& _srcImg,Image& _dstImg,Image& _kernal)
	{
		assert(_kernal.getHeight()%2==1 && _kernal.getWidth()%2==1);
		assert(!_srcImg.isEmpty() && !_kernal.isEmpty());
		assert(_srcImg.getHeight()>0 && _srcImg.getWidth()>0);
		assert(_kernal.getHeight()>0 && _kernal.getWidth()>0);
		assert(_kernal.getHeight()==_kernal.getWidth());
		assert(_srcImg.getHeight()>_kernal.getHeight() && _srcImg.getWidth()>_kernal.getWidth());
		assert(_srcImg.getType()==IMG_GRAY);

		int width=_srcImg.getWidth();
		int height=_srcImg.getHeight();
		int kernalSize=_kernal.getHeight();
		int fixSize=kernalSize/2;
		Image temp(width,height);
		Image broaderImg;
		_srcImg.clone(broaderImg);
		int broderSize=_kernal.getHeight()/2;
		for(int i=0;i<broderSize;i++)
			makeBroader(broaderImg,broaderImg);
		for(int i=0;i<height;i++)
			for(int j=0;j<width;j++)
			{
				float convResult=0;
				for(int k=0;k<kernalSize;k++)
					for(int l=0;l<kernalSize;l++)
					{
						int _xFix=k-fixSize;
						int _yFix=l-fixSize;
						convResult+=(broaderImg(i+k,j+l)->value*_kernal(l,k)->value);
					}
				temp(i,j)->value=convResult;
			}
		_dstImg=temp;
		return 0;
	}
}