////////////////////////////////////////////////////////////////////////
// author: AirSmith
// 2015-10-12 : orignal edition
// author: AirSmith
// 2016-10-18 : add some annotation and an example
////////////////////////////////////////////////////////////////////////

#ifndef IMAGE_H
#define IMAGE_H

#include <iostream>

//TODO
//添加JPG支持
//做一个类似Array类成为公共接口类

namespace img
{
enum ImageType{IMG_COLOR,IMG_GRAY};

//a single pix
typedef struct tag_Pix
{
	unsigned char Blue;
	unsigned char Green;
	unsigned char Red;
}Pix;

//a single element
/*
image would store element as data-structure
when the image is IMG_COLOR the element would orgnized as BGRA
when the image is IMG_GRAY the element would orgnized as a float

we design this to let no matter the image is gray or color
the total zise would be equal to float32
*/
typedef union tag_Element
{
	float value;
	struct
	{
		unsigned char Blue;
		unsigned char Green;
		unsigned char Red;
		unsigned char Alpha;
	}channels;
}
Element;

//data structure
/*
a refcount counting the refence of same memory block
when the count is 0
it will auto-free the memory block
*/
class DATA
{
public:
	int refcount;
	ImageType type;
	Element* data;
};

class Image
{
public:
    //construct a image default is gray-image
	Image(ImageType _type=IMG_GRAY);
	Image(int _width,int _height,ImageType _type=IMG_GRAY);
	Image(int _width,int _height,float* _datas,ImageType _type=IMG_GRAY);
	~Image();
	
    //force-free the image
	void free();

    //create a new image
	void create(int _width,int _height,ImageType _type=IMG_GRAY);

    //clone the image to current image 
	void clone(Image &_img);

    //overload () to handle the pix at (x,y)
	Element* operator() (int _x,int _y);

    //overload = to copy a image
	void operator = (Image & _img);

	int getWidth(){return width;}
	int getHeight(){return height;}
	ImageType getType(){return datas->type;}
	bool isEmpty(){return (!this->datas)?true:false;}

	int refCount();

private:
	int width;
	int height;
	DATA* datas;
};

    //common method to load image
	int imgRead(Image& _imgR,Image& _imgG,Image& _imgB,std::string _name);
	int imgRead(Image& _img,std::string _name);

    //save image
	int imgSave(Image& _imgR,Image& _imgG,Image& _imgB,std::string _name);
	int imgSave(Image& _img,std::string _name);

    //expand image edge
	enum BoarderType{IMG_ZERO,IMG_NEAR};
	int makeBroader(Image& _srcImg,Image& _dstImg,BoarderType _type=IMG_NEAR,int _broaderSize=1);

    //add noise
	enum noiseType{IMG_UNIFORM,IMG_GAUSS,IMG_SALT};
	int addNoise(Image& _srcImg,Image& _dstImg,noiseType _type=IMG_GAUSS,float _percent=0.2);

    //split image to 3 channel
	int split(Image& _srcImg,Image& _imgR,Image& _imgG,Image& _imgB);
    //merge 3 channel to single image
	int merge(Image& _imgR,Image& _imgG,Image& _imgB,Image& _dstImg);

    //conver RGB imgae to gray or otherwise
	enum cvtType{IMG_RGB2GRAY};
	int cvtColor(Image&_srcImg,Image & _dstImg,cvtType _type=IMG_RGB2GRAY);

    //normalize image pix
	int normalize(Image& _srcImg,Image& _dstImg,int normalSize=255);
}
#endif