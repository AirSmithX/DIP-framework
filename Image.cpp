#include "Image.h"
#include <assert.h>
#include <memory>
#include <Windows.h>
#include <fstream>
#include <limits>
#include <time.h>
#include <math.h>
#include <vector>

#include<iostream>

namespace img
{
	Image::Image(ImageType _type):width(0),height(0),datas(0)
	{

	}

	Image::Image(int _width,int _height,ImageType _type):width(0),height(0),datas(0)
	{
		this->create(_width,_height,_type);
	}

	Image::~Image()
	{
		free();
	}

	Image::Image(int _width,int _height,float* _datas,ImageType _type):width(0),height(0),datas(0)
	{
		//assert((sizeof(*_datas)/sizeof(float))==(_width*_height));
		this->create(_width,_height,_type);
		std::cout<<this->getWidth()<<std::endl;
		for(int i=0;i<this->height;i++)
			for(int j=0;j<this->height;j++)
				this->datas->data[i*this->width+j].value=_datas[i*this->width+j];
	}

	void Image::free()
	{
		this->width=0;
		this->height=0;
		if(!this->datas)
			return ;
		this->datas->refcount--;
		if(!this->datas->refcount)
		{
			std::free(this->datas->data);
			std::free(this->datas);
		}
		this->datas=0;
	}

	int Image::refCount()
	{
		if(!this->datas)
			return 0;
		return this->datas->refcount;
	}

	void Image::create(int _width,int _height,ImageType _type)
	{
		assert(_type==IMG_COLOR || _type==IMG_GRAY);
		assert(_width>0 && _height>0);

		if(!this->isEmpty())
			if(width==_width && height==_height && this->datas->type==_type && _width!=0)
				return ;

		free();
		width=_width;
		height=_height;

		this->datas=(DATA*)malloc(sizeof(DATA));
		this->datas->type=_type;
		this->datas->refcount=1;
		this->datas->data=(Element*)malloc(sizeof(Element)*this->width*this->height);
	}

	void Image::operator=(Image& _img)
	{
		//assert(!_img.isEmpty());
		if(_img.isEmpty())
		{
			this->width=0;
			this->height=0;
			this->datas=0;
			return ;
		}
		free();
		this->width=_img.width;
		this->height=_img.height;
		this->datas=_img.datas;
		this->datas->refcount++;
	}

	Element* Image::operator()(int _x,int _y)
	{
		//assert(_x<this->getWidth() );
		//assert(_y<this->getHeight() );
		return this->datas->data+(width*_x+_y);
	}

	void Image::clone(Image& _img)
	{
		Image temp(this->width,this->height,this->datas->type);
		std::memcpy(temp.datas->data,this->datas->data,(width*height*sizeof(Element)));
		_img=temp;
	}

	int imgRead(Image& _imgR,Image& _imgG,Image& _imgB,std::string _name)
	{
		Image temp(IMG_COLOR);
		imgRead(temp,_name);
		split(temp,_imgR,_imgG,_imgB);
		return 0;
	}

	int imgRead(Image& _img,std::string _name)
	{
		//assert(_img.getType()==IMG_COLOR);
		std::ifstream inFile(_name, std::fstream::in | std::ios_base::binary);
		assert(inFile.is_open());

		BITMAPFILEHEADER fileHeader;
		BITMAPINFOHEADER infoHeader;
		inFile.read((char*)&fileHeader,sizeof(BITMAPFILEHEADER));
		inFile.read((char*)&infoHeader,sizeof(BITMAPINFOHEADER));

		assert(fileHeader.bfType==0x4d42);

		std::cout<<"width is:"<<infoHeader.biWidth<<" height is:"<<infoHeader.biHeight<<std::endl;
		std::cout<<"offset is:"<<fileHeader.bfOffBits<<std::endl;

		int i,j;
		Image img(infoHeader.biWidth,infoHeader.biHeight,IMG_COLOR);
		int points=3*img.getWidth();
		int off=points;
		while(points%4!=0)
			points++;
		off=points-off;
		inFile.seekg(fileHeader.bfOffBits,inFile.beg);
		for(i=img.getHeight()-1;i>-1;i--)
		{
			for(j=0;j<img.getWidth();j++)
			{
				Pix bmpRGB;
				inFile.read((char*)&bmpRGB,sizeof(bmpRGB));
				img(i,j)->channels.Red=bmpRGB.Red;
				img(i,j)->channels.Blue=bmpRGB.Blue;
				img(i,j)->channels.Green=bmpRGB.Green;
				img(i,j)->channels.Alpha=1;			
			}
			inFile.seekg(off,inFile.cur);
		}
		inFile.close();
		_img=img;
		return 0;
	}

	int imgSave(Image& _imgR,Image& _imgG,Image& _imgB,std::string _name)
	{
		Image temp(IMG_COLOR);
		merge(_imgR,_imgG,_imgB,temp);
		imgSave(temp,_name);
		return 0;
	}

	int imgSave(Image& _img,std::string _name)
	{
		assert(_img.getType()==IMG_COLOR);
		std::ofstream outFile(_name, std::fstream::out | std::ios_base::binary);
		assert(outFile.is_open());

		int points=_img.getWidth()*3;
		int off=points;
		while(points%4!=0)
			points++;
		off=points-off;

		BITMAPFILEHEADER fileHeader;
		BITMAPINFOHEADER infoHeader;

		fileHeader.bfType=0x4d42;
		fileHeader.bfSize=static_cast<DWORD>(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+points*_img.getHeight());
		fileHeader.bfOffBits=static_cast<DWORD>(54);
	
		infoHeader.biSize=static_cast<DWORD>(40);
		infoHeader.biWidth=static_cast<long>(_img.getWidth());
		infoHeader.biHeight=static_cast<long>(_img.getHeight());
		infoHeader.biPlanes=static_cast<WORD>(1);
		infoHeader.biBitCount=static_cast<WORD>(24);
		infoHeader.biCompression=static_cast<DWORD>(0);
		infoHeader.biSizeImage=static_cast<DWORD>(sizeof(Pix)*_img.getWidth()*_img.getHeight());
		infoHeader.biXPelsPerMeter=static_cast<long>(0);
		infoHeader.biYPelsPerMeter=static_cast<long>(0);
		infoHeader.biClrUsed=static_cast<DWORD>(0);
			
			
		outFile.write((char*)&fileHeader,sizeof(BITMAPFILEHEADER));
		outFile.write((char*)&infoHeader,sizeof(BITMAPINFOHEADER));
			
		int i,j;
		for(i=_img.getHeight()-1;i>-1;i--)
		{
			for(j=0;j<_img.getWidth();j++)
		    {
		        Pix bmpRGB;
				bmpRGB.Blue=_img(i,j)->channels.Blue;
				bmpRGB.Green=_img(i,j)->channels.Green;
				bmpRGB.Red=_img(i,j)->channels.Red;
				outFile.write((char*)&bmpRGB,sizeof(Pix));
		    }
			outFile.write("0000",off);
		}
			
		std::cout<<off<<" skiped"<<std::endl;
			
		return 0;
	}	

	int __makeBroader(Image& _srcImg,Image& _dstImg,BoarderType _type)
	{
		assert(_srcImg.getHeight()>3 && _srcImg.getWidth()>3);
		assert( _type==IMG_ZERO || _type==IMG_NEAR );
		Image temp(_srcImg.getWidth()+2,_srcImg.getHeight()+2,_srcImg.getType());
		int i,j;

		for(i=0;i<temp.getHeight();i++)
			for(j=0;j<temp.getWidth();j++)
			{	
				int _x=(i-1)<0?0:(i-1)==_srcImg.getHeight()?(i-2):(i-1);
				int _y=(j-1)<0?0:(j-1)==_srcImg.getWidth()?(j-2):(j-1);
				*temp(i,j)=*_srcImg(_x,_y);
			}

		if(_type==IMG_ZERO)
		{
			Element p;
			p.channels.Blue=0;p.channels.Red=0;p.channels.Green=0;
			for(i=0;i<temp.getHeight();i++)
				for(j=0;j<temp.getWidth();j++)
				{	
					if((i-1)<0 || (j-1)<0 || (i-1)==_srcImg.getHeight() || (j-1)==_srcImg.getWidth())
						*temp(i,j)=p;
				}
		}
		_dstImg=temp;
		return 0;
	}

	int makeBroader(Image& _srcImg,Image& _dstImg,BoarderType _type,int _broaderSize)
	{
		Image temp;
		__makeBroader(_srcImg,temp,_type);
		for(int i=1;i<_broaderSize;i++)
			__makeBroader(temp,temp,_type);
		_dstImg=temp;
		return 0;
	}

	int split(Image& _srcImg,Image& _imgR,Image& _imgG,Image& _imgB)
	{
		assert(_srcImg.getHeight()>0 && _srcImg.getWidth()>0);
		assert(_srcImg.getType()==IMG_COLOR);
		_imgR.create(_srcImg.getWidth(),_srcImg.getHeight(),IMG_GRAY);
		_imgG.create(_srcImg.getWidth(),_srcImg.getHeight(),IMG_GRAY);
		_imgB.create(_srcImg.getWidth(),_srcImg.getHeight(),IMG_GRAY);
		int i,j;
		for(i=0;i<_srcImg.getHeight();i++)
			for(j=0;j<_srcImg.getWidth();j++)
			{
				_imgR(i,j)->value=static_cast<float>(_srcImg(i,j)->channels.Red);
				_imgG(i,j)->value=static_cast<float>(_srcImg(i,j)->channels.Green);
				_imgB(i,j)->value=static_cast<float>(_srcImg(i,j)->channels.Blue);
			}
		return 0;
	}

	int merge(Image& _imgR,Image& _imgG,Image& _imgB,Image& _dstImg)
	{
		assert(_imgR.getWidth()==_imgG.getWidth() && _imgR.getWidth()==_imgB.getWidth());
		assert(_imgR.getHeight()==_imgG.getHeight() && _imgR.getHeight()==_imgB.getHeight());
		assert(_imgR.getHeight()>0 && _imgR.getWidth()>0);
		assert(_imgR.getType()==IMG_GRAY && _imgG.getType()==IMG_GRAY && _imgB.getType()==IMG_GRAY);




		_dstImg.create(_imgR.getWidth(),_imgR.getHeight(),IMG_COLOR);
		int i,j;
		for(i=0;i<_dstImg.getHeight();i++)
			for(j=0;j<_dstImg.getWidth();j++)
			{
				Element temp;
				temp.channels.Red=static_cast<unsigned char>(_imgR(i,j)->value);
				temp.channels.Green=static_cast<unsigned char>(_imgG(i,j)->value);
				temp.channels.Blue=static_cast<unsigned char>(_imgB(i,j)->value);
				*_dstImg(i,j)=temp;
			}
		return 0;
	}

	int cvtColor(Image&_srcImg,Image & _dstImg,cvtType _type)
	{
		assert(_srcImg.getHeight()>0 && _srcImg.getWidth()>0);
		assert(_type==IMG_RGB2GRAY);

		if(_type==IMG_RGB2GRAY)
		{
			assert(_srcImg.getType() == IMG_COLOR);
			Image temp(_srcImg.getWidth(),_srcImg.getHeight(),IMG_GRAY);
			int i,j;
			for(i=0;i<_srcImg.getHeight();i++)
				for(j=0;j<_srcImg.getWidth();j++)
				{
					Element tempPix;
					float R=static_cast<float>(_srcImg(i,j)->channels.Red);
					float G=static_cast<float>(_srcImg(i,j)->channels.Green);
					float B=static_cast<float>(_srcImg(i,j)->channels.Blue);
					float gray=0.299*R+0.587*G+0.114*B;
					tempPix.value=gray;
					*temp(i,j)=tempPix;
				}
			_dstImg=temp;
		}
		
		return 0;
	}

	float __findMax(Image& _srcImg)
	{
		float max=-(std::numeric_limits<float>::max)();
		int i,j;
		for(i=0;i<_srcImg.getHeight();i++)
			for(j=0;j<_srcImg.getWidth();j++)
			{
				if(_srcImg(i,j)->value>max)
					max=_srcImg(i,j)->value;
			}
		return max;
	}

	float __findMin(Image& _srcImg)
	{
		float min=(std::numeric_limits<float>::max)();
		int i,j;
		for(i=0;i<_srcImg.getHeight();i++)
			for(j=0;j<_srcImg.getWidth();j++)
			{
				if(_srcImg(i,j)->value<min)
					min=_srcImg(i,j)->value;
			}
		return min;
	}

	int normalize(Image& _srcImg,Image& _dstImg,int normalSize)
	{
		assert(_srcImg.getHeight()>0 && _srcImg.getWidth()>0);
		assert(_srcImg.getType()==IMG_GRAY);

		Image temp(_srcImg.getWidth(),_srcImg.getHeight(),_srcImg.getType());
		float max=__findMax(_srcImg);
		float min=__findMin(_srcImg);
		float range=max-min;

		int i,j;
		for(i=0;i<temp.getHeight();i++)
			for(j=0;j<temp.getWidth();j++)
			{
				temp(i,j)->value=_srcImg(i,j)->value-min;
			}

		for(i=0;i<temp.getHeight();i++)
			for(j=0;j<temp.getWidth();j++)
			{
				temp(i,j)->value=(_srcImg(i,j)->value*static_cast<float>(normalSize))/range;
			}

		_dstImg=temp;
		return 0;
	}


	static bool setted=false;
	float __radomGenerator(noiseType _type)
	{
		if(!setted)
		{
			setted=true;
			std::srand((unsigned)time(NULL));
		}
		static double V1, V2, S;
		static int phase = 0;
		float possible;
		possible=static_cast<double>(rand()) / RAND_MAX;
		switch(_type)
		{
			case(IMG_UNIFORM):
				return (possible-0.5)*2;
				break;
			case(IMG_GAUSS):
				double X;
			    if ( phase == 0 ) 
				{
					do {
						double U1 = static_cast<double>(rand()) / RAND_MAX;
						double U2 = static_cast<double>(rand()) / RAND_MAX;
             
						V1 = 2 * U1 - 1;
						V2 = 2 * U2 - 1;
						S = V1 * V1 + V2 * V2;
			        } while(S >= 1 || S == 0);
					X = V1 * sqrt(-2 * log(S) / S);
			    } 
				else
					X = V2 * sqrt(-2 * log(S) / S);
				phase = 1 - phase;
				return X = X * 0.1;
				break;
			case(IMG_SALT):
				return possible;
				break;
		}
		return 0;
	}

	int __generateNoiseMap(int width,int height,Image& _dstImg,noiseType _type)
	{
		_dstImg.create(width,height);
		int i,j;
		for(i=0;i<_dstImg.getHeight();i++)
			for(j=0;j<_dstImg.getWidth();j++)
			{
				_dstImg(i,j)->value=__radomGenerator(_type);
				float tt=_dstImg(i,j)->value;
			}
		return 0;
	}

	int __addNoise(Image& _srcImg,Image& _dstImg,noiseType _type,float _percent,Image& _noise)
	{
		float max=__findMax(_srcImg);
		float min=__findMin(_srcImg);
		float uniform=(max-min)*_percent;

		Image temp(_srcImg.getWidth(),_srcImg.getHeight());
		if(_noise.isEmpty())
			__generateNoiseMap(_srcImg.getWidth(),_srcImg.getHeight(),_noise,_type);
		int i,j;
		int count=0;
		for(i=0;i<_srcImg.getHeight();i++)
			for(j=0;j<_srcImg.getWidth();j++)
			{
				if(_type==IMG_SALT)
				{
					if(_noise(i,j)->value<_percent)
						temp(i,j)->value=max;
					else if(_noise(i,j)->value>1-_percent)
						temp(i,j)->value=min;
					else
					{
						count++;
						temp(i,j)->value=_srcImg(i,j)->value;
					}
				}
				else
					temp(i,j)->value=_srcImg(i,j)->value+_noise(i,j)->value*uniform;
			}
		_dstImg=temp;
		std::cout<<count<<std::endl;
		return 0;
	}

	int addNoise(Image& _srcImg,Image& _dstImg,noiseType _type,float _percent)
	{
		assert(_srcImg.getHeight()>0 && _srcImg.getWidth()>0);
		Image temp(_srcImg.getWidth(),_srcImg.getHeight(),_srcImg.getType());

		if(_srcImg.getType()==IMG_COLOR)
		{
			Image noise[3];
			if(_type==IMG_SALT)
			{
				__generateNoiseMap(_srcImg.getWidth(),_srcImg.getHeight(),noise[0],_type);
				noise[1]=noise[0];noise[2]=noise[0];
			}
			Image img[3];
			split(_srcImg,img[0],img[1],img[2]);
			for(int i=0;i<3;i++)
			{
				__addNoise(img[i],img[i],_type,_percent,noise[i]);
				normalize(img[i],img[i]);
			}
			merge(img[0],img[1],img[2],temp);
		}
		if(_srcImg.getType()==IMG_GRAY)
		{
			Image noise;
			if(_type==IMG_SALT)
				__generateNoiseMap(_srcImg.getWidth(),_srcImg.getHeight(),noise,_type);
			__addNoise(_srcImg,temp,_type,_percent,noise);
			normalize(_srcImg,_srcImg);
		}
		_dstImg=temp;
		return 0;
	}
}