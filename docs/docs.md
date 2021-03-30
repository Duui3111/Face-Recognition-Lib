## classes `videocap4`

# The Functions

# The First Function
```cpp
void LoadFace(
    const char*
);
```

# The Second Function
```cpp
void LoadFace(
    const char* filepath
);
```

# The Thied Function
```cpp
void SaveBitmapCap(
    const char* filepath
);
```

# The Fourth Function
```cpp
int ShowVideoWindow(
    const wchar_t* Caption, 
    int Left, int Top, 
    int Width, int Height
);
```

# The Fifth Function
```cpp
void Rectangle(
    int X, int Y, 
    int A, int R, int G, int B, 
    int thinkness
);
```

# The Six Function
```cpp
void Show(

);
```

# The Structs

```cpp
struct VideoInfo
{
    unsigned int numodevices;
    wchar_t devname[256];
    wchar_t devdescription[256];
    wchar_t devpath[256];
    wchar_t devclsid[256];
};  
```