#include "Camera.h"

int main()
{
    videocap4 vcap;

    std::cout
    << "Found A Divice\n" <<
    "Number of Divice " << vcap.VideoInfo.numodevices << '\n' <<
    "Divice Name " << (const char*)vcap.VideoInfo.devname << '\n' <<
    "Divice Description " << (const char*)vcap.VideoInfo.devdescription << '\n' <<
    "Divice Path " << (const char*)vcap.VideoInfo.devpath << '\n' <<
    "Divice CLSID " << (const char*)vcap.VideoInfo.devclsid << std::endl;

    if (vcap.IsCapOpened)
    {
        vcap.LoadFace("./me.bmp");    
        vcap.SaveBitmapCap("./captures");     
        vcap.Rectangle(650, 300, 255, 255, 0, 0, 2);
        
        if (vcap.IsFaceRec)
            std::cout << "face rec" << std::endl;
        
        vcap.ShowVideoWindow(L"Video Window", 500, 150, 600, 500);
        vcap.Show();
    }
        
    return 0;
}
