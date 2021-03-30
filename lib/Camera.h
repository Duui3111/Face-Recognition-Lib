#pragma once

#include <Windows.h>
#include <dshow.h>
#include <mfapi.h>
#include <mfidl.h>
#include <gdiplus.h>

// windows libs
#pragma comment(lib, "strmbase.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "dxva2.lib")
#pragma comment(lib, "evr.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "strmbase.lib")
#pragma comment(lib, "mfplay.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "Gdiplus.lib")


#include "dlib_image_rec.h"

#include <iostream>
#include <fstream>
#include <string>

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>


#pragma warning(disable : 4996)


static const IID IID_ISampleGrabberCB = { 0x0579154A, 0x2B53, 0x4994, { 0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85 } };
static const IID IID_ISampleGrabber = { 0x6B652FFF, 0x11FE, 0x4fce, { 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F } };
static const CLSID CLSID_SampleGrabber = { 0xC1F400A0, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

interface ISampleGrabberCB : public IUnknown
{
    virtual STDMETHODIMP SampleCB(double SampleTime, IMediaSample* pSample) = 0;
    virtual STDMETHODIMP BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen) = 0;
};

interface ISampleGrabber : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL OneShot) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMediaType(const AM_MEDIA_TYPE* pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(AM_MEDIA_TYPE* pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL BufferThem) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(long* pBufferSize, long* pBuffer) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(IMediaSample** ppSample) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetCallback(ISampleGrabberCB* pCallback, long WhichMethodToCallback) = 0;
};

void SaveFrame(BITMAPINFO bi, BYTE* data, unsigned long size, std::string filepath, std::string fileface)
{
    DWORD bufsize = size;
    BITMAPFILEHEADER bfh;
    memset(&bfh, 0, sizeof(bfh));
    bfh.bfType = 'MB';
    bfh.bfSize = sizeof(bfh) + bufsize + sizeof(BITMAPINFOHEADER);
    bfh.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);

    BITMAPINFOHEADER bih;
    memset(&bih, 0, sizeof(bih));
    bih.biSize = sizeof(bih);
    bih.biWidth = bi.bmiHeader.biWidth;
    bih.biHeight = bi.bmiHeader.biHeight;
    bih.biPlanes = 1;
    bih.biBitCount = 24;


    if (!filepath.empty())
    {
        char FileName[512];
        sprintf(FileName, "capture_%d.bmp", (int)GetTickCount());


        std::experimental::filesystem::path path{ filepath };
        path /= FileName;
        std::experimental::filesystem::create_directories(path.parent_path());

        std::ofstream ofs(path, std::ios::binary);
        if (ofs)
        {
            ofs.write((const char*)&bfh, sizeof(bfh));
            ofs.write((char*)&bih, sizeof(bih));
            ofs.write((char*)data, bufsize);
            ofs.close();

            if (!fileface.empty())
            {
                while (true)
                {
                    Face_Image_Recognition(fileface, path.string());
                    break;
                }
            }

            std::cout << path.string() << std::endl;
        }
    }
}

class CSampleGrabberCB : public ISampleGrabberCB
{
public:
    DWORD lastTime;
    long Width;
    long Height;
    const char* filepathcap = "";
    const char* fileface = "";

    HANDLE BufferEvent;
    LONGLONG prev, step;

    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }

    CSampleGrabberCB() { lastTime = 0; }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown)
        {
            *ppv = (void*) static_cast<ISampleGrabberCB*> (this);
            return NOERROR;
        }

        return E_NOINTERFACE;
    }

    STDMETHODIMP SampleCB(double SampleTime, IMediaSample* pSample) { return 0; }

    STDMETHODIMP BufferCB(double SampleTime, BYTE* pBuffer, long BufferSize)
    {
        BITMAPFILEHEADER bfh;
        memset(&bfh, 0, sizeof(bfh));
        bfh.bfType = 'MB';
        bfh.bfSize = sizeof(bfh) + BufferSize + sizeof(BITMAPINFOHEADER);
        bfh.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);

        DWORD Written = 0;

        BITMAPINFOHEADER bih;
        memset(&bih, 0, sizeof(bih));
        bih.biSize = sizeof(bih);
        bih.biWidth = Width;
        bih.biHeight = Height;
        bih.biPlanes = 1;
        bih.biBitCount = 24;

        BITMAPINFO bi;

        bi.bmiHeader = bih;

        DWORD newTime = GetTickCount();
        if (newTime - lastTime > 1000)
        {
            lastTime = newTime;
            SaveFrame(bi, pBuffer, BufferSize, filepathcap, fileface);
        }

        return 0;
    }
};


int __rectangleX = 0;
int __rectangleY = 0;
int __rectangleA = 0;
int __rectangleR = 0;
int __rectangleG = 0;
int __rectangleB = 0;
int __rectangleT = 0;


class videocap4
{
private:
    struct VideoInfo
    {
        unsigned int numodevices;
        wchar_t devname[256];
        wchar_t devdescription[256];
        wchar_t devpath[256];
        wchar_t devclsid[256];
    };

public:
    bool IsCapOpened = false;
    bool IsFaceRec = false;

    VideoInfo VideoInfo;
    videocap4();
    ~videocap4();

    void LoadFace(const char*);
    void SaveBitmapCap(const char*);
    int ShowVideoWindow(const wchar_t*, int, int, int, int);
    void Rectangle(int, int, int, int, int, int, int);
    void Show();

private:
    IGraphBuilder* GraphBuilder = nullptr;
    ICaptureGraphBuilder2* CaptureGraphBuilder2 = nullptr;
    IMediaControl* MediaControl = nullptr;
    IBaseFilter* DeviceFilter = nullptr;
    IBaseFilter* GrabberF = nullptr;
    ISampleGrabber* SampleGrabber = nullptr;
    ICreateDevEnum* CreateDevEnum = nullptr;
    IEnumMoniker* EnumMoniker = nullptr;
    IMoniker* Moniker = nullptr;
    IVideoWindow* VideoWindow = nullptr;
    IVMRWindowlessControl* WinlessContr = nullptr;
    IPropertyBag* PropertyBag = nullptr;
    IMFActivate** imfactivate = nullptr;
    IMFAttributes* imfAttributes = nullptr;

    HWND paranthwnd;

    const char* filepathcap = "";
    const char* fileface = "";

    bool SartBitMapRecord();
    void StartDeviceInfo();
};

videocap4::videocap4()
{
    CoInitialize(NULL);
    CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (PVOID*)&CreateDevEnum);
    CreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &EnumMoniker, 0);

    if (EnumMoniker == NULL)
        MessageBox(0, L"No Divices Found", L"Error", MB_ICONERROR);

    EnumMoniker->Reset();

    EnumMoniker->Next(1, &Moniker, NULL);
    Moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&PropertyBag);
    Moniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&DeviceFilter);

    IsCapOpened = true;

    StartDeviceInfo();
}

videocap4::~videocap4()
{
    IsCapOpened = false;

    EnumMoniker->Release();
    CreateDevEnum->Release();

    Moniker->Release();
    PropertyBag->Release();
    MediaControl->Release();
    CaptureGraphBuilder2->Release();
    GraphBuilder->Release();
    VideoWindow->Release();

    CoUninitialize();
}

void videocap4::SaveBitmapCap(const char* filepath)
{
    this->filepathcap = filepath;
}

void videocap4::LoadFace(const char* fileface)
{
    this->fileface = fileface;
}

bool videocap4::SartBitMapRecord()
{
    CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&GrabberF);
    GrabberF->QueryInterface(IID_ISampleGrabber, (void**)&SampleGrabber);
    CaptureGraphBuilder2->SetFiltergraph(GraphBuilder);

    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB24;
    SampleGrabber->SetMediaType(&mt);
    SampleGrabber->SetBufferSamples(TRUE);

    GraphBuilder->AddFilter(GrabberF, L"Sample Grabber");
    CaptureGraphBuilder2->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, DeviceFilter, GrabberF, 0);
    HRESULT hr = SampleGrabber->GetConnectedMediaType(&mt);

    VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)mt.pbFormat;

    CSampleGrabberCB* CB = new CSampleGrabberCB();
    CB->filepathcap = this->filepathcap;
    CB->fileface = this->fileface;

    if (!FAILED(hr))
    {
        CB->Width = vih->bmiHeader.biWidth;
        CB->Height = vih->bmiHeader.biHeight;
    }

    SampleGrabber->SetCallback(CB, 1);

    if (this->filepathcap == "")
        return false;
    else
        return true;
}

void videocap4::StartDeviceInfo()
{
    VARIANT var;

    MFCreateAttributes(&imfAttributes, 1);
    imfAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    MFEnumDeviceSources(imfAttributes, &imfactivate, &VideoInfo.numodevices);
    imfAttributes->Release();

    var.vt = VT_BSTR;
    PropertyBag->Read(L"FriendlyName", &var, 0);
    WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, (LPSTR)VideoInfo.devname, sizeof(VideoInfo.devname), 0, 0);
    VariantClear(&var);

    var.vt = VT_BSTR;
    PropertyBag->Read(L"Description", &var, 0);
    WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, (LPSTR)VideoInfo.devdescription, sizeof(VideoInfo.devdescription), 0, 0);
    VariantClear(&var);

    var.vt = VT_BSTR;
    PropertyBag->Read(L"DevicePath", &var, 0);
    WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, (LPSTR)VideoInfo.devpath, sizeof(VideoInfo.devpath), 0, 0);
    VariantClear(&var);

    var.vt = VT_BSTR;
    PropertyBag->Read(L"CLSID", &var, 0);
    WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, (LPSTR)VideoInfo.devclsid, sizeof(VideoInfo.devclsid), 0, 0);
    VariantClear(&var);
}

void OnPaint_(HDC hdc)
{
    if (__rectangleX != 0 || __rectangleY != 0)
    {
        Gdiplus::Graphics graphics(hdc);
        Gdiplus::Pen pen(Gdiplus::Color(__rectangleA, __rectangleR, __rectangleG, __rectangleB), __rectangleT);
        graphics.DrawRectangle(&pen, 10, 10, 240, 240); //240
    }
}

LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        OnPaint_(hdc);
        EndPaint(hWnd, &ps);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

int videocap4::ShowVideoWindow(const wchar_t* Caption, int Left, int Top, int Width, int Height)
{
    HINSTANCE hinstance = GetModuleHandle(NULL);
    RegisterClass(new WNDCLASS({ CS_HREDRAW | CS_VREDRAW, _WndProc, 0L, 0L, hinstance, LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_CROSS), (HBRUSH)GetStockObject(WHITE_BRUSH), 0L, L"CAPWINDOW" }));
    paranthwnd = CreateWindowExW(0, L"CAPWINDOW", 0, WS_OVERLAPPEDWINDOW, 500, 150, 600, 500, 0, 0, hinstance, 0);

    CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (LPVOID*)&GraphBuilder);
    CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (LPVOID*)&CaptureGraphBuilder2);
    CaptureGraphBuilder2->SetFiltergraph(GraphBuilder);
    GraphBuilder->QueryInterface(IID_IMediaControl, (LPVOID*)&MediaControl);
    GraphBuilder->AddFilter(DeviceFilter, L"Device Filter");
    if (SartBitMapRecord() == true) SartBitMapRecord();
    CaptureGraphBuilder2->RenderStream(&PIN_CATEGORY_PREVIEW, NULL, DeviceFilter, NULL, NULL);
    GraphBuilder->QueryInterface(IID_IVideoWindow, (LPVOID*)&VideoWindow);
    SetWindowTextW(paranthwnd, Caption);
    VideoWindow->put_Owner((OAHWND)paranthwnd);
    VideoWindow->SetWindowPosition(Left, Top, Width, Height);
    VideoWindow->put_WindowStyle(WS_CLIPCHILDREN);

    RECT rc;
    GetClientRect(paranthwnd, &rc);
    VideoWindow->SetWindowPosition(0, 0, rc.right, rc.bottom);

    VideoWindow->put_Visible(OATRUE);
    ShowWindow(paranthwnd, SW_SHOWDEFAULT);
    MediaControl->Run();

    if (__rectangleX != 0 || __rectangleY != 0)
    {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        RegisterClass(new WNDCLASS({ 0, _WndProc, 0L, 0L, hinstance, LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_CROSS), (HBRUSH)GetStockObject(BLACK_BRUSH), 0L, L"FACEBOXCAP" }));
        HWND hWnd = CreateWindowExW(WS_EX_LAYERED, L"FACEBOXCAP", 0, WS_POPUP | WS_VISIBLE | WS_SYSMENU, __rectangleX /*650*/, __rectangleY /*300*/, 320, 320, paranthwnd, 0, hinstance, 0);
        SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_COLORKEY | LWA_ALPHA);
        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);
        Gdiplus::GdiplusShutdown(gdiplusToken);
    }

    IsFaceRec = __IsFaceRec;

    return 0;
}

void videocap4::Show()
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void videocap4::Rectangle(int X, int Y, int A, int R, int G, int B, int thinkness)
{
    __rectangleX = X;
    __rectangleY = Y;
    __rectangleA = A;
    __rectangleR = R;
    __rectangleG = B;
    __rectangleB = B;
    __rectangleT = thinkness;
}