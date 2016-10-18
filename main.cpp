#include<iostream>
#include"Image.h"
#include"SpaFilter.h"

using namespace img;
int main(int argc, char *argv[])
{
#ifdef QT_USED
    QCoreApplication a(argc, argv);
#endif

    //define a img test1 and img test2
	Image test1;
    Image test2;

	//read the image to image1
	imgRead(test1,"borderland.bmp");
	
    //copy test2 to test1
    //important every modified in test2  woulrd refluence test1
    test2=test1;
    //if you want just copy the data to test2 using
    test2.clone(test1);

    //conver RGB 2 gray image
	cvtColor(test1,test1,IMG_RGB2GRAY);

    //explan adge
	makeBroader(test1,test1,IMG_NEAR,10);
	laplaceFilter(test1,test1,256);
	
	imgSave(test1,"levelMirror.bmp");
	
//good luck fuckers :)
}