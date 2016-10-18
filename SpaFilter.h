////////////////////////////////////////////////////////////////////////
// author: AirSmith
// 2015-10-12 : orignal edition
///////////////////////////////////////////////////////////////////////
#ifndef SPAFILTER_H
#define SPAFILTER_H

#include "Image.h"

namespace img
{
    //a laplacefilter example
	int laplaceFilter(Image& _srcImg,Image& _dstImg,int _range=128);


	int spaFilter(Image& _srcImg,Image& _dstImg,Image& _kernal);
	int convolution(Image& _srcImg,Image& _dstImg,Image& _kernal);
}


#endif