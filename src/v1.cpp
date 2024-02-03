#include <iostream>
#include <windows.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

bool IsRedPixel(const Vec3b& pixel)
{
    return ((pixel[0] >= 0 && pixel[0] <= 2)) &&
        (pixel[1] >= 140 && pixel[1] <= 255) &&
        (pixel[2] >= 0 && pixel[2] <= 255);
}

void HDCToMemory(HDC Context, RECT Area, uint16_t BitsPerPixel = 24)
{
    uint32_t Width = 200;
    uint32_t Height = 200;

    int centerX = (Area.left + Area.right - Width) / 2;
    int centerY = (Area.top + Area.bottom - Height) / 2;

    BITMAPINFO Info = { 0 };
    Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    Info.bmiHeader.biWidth = Width;
    Info.bmiHeader.biHeight = -static_cast<long>(Height);  // Negative height for a top-down bitmap
    Info.bmiHeader.biPlanes = 1;
    Info.bmiHeader.biBitCount = BitsPerPixel;
    Info.bmiHeader.biCompression = BI_RGB;

    while (true) {
        char* Pixels = nullptr;
        HDC MemDC = CreateCompatibleDC(Context);
        HBITMAP Section = CreateDIBSection(Context, &Info, DIB_RGB_COLORS, reinterpret_cast<void**>(&Pixels), nullptr, 0);
        SelectObject(MemDC, Section);

        if (BitBlt(MemDC, 0, 0, Width, Height, Context, centerX, centerY, SRCCOPY))
        {
            int totalRedPixels = 0;
            int totalX = 0;
            int totalY = 0;

            Mat image(Height, Width, CV_8UC3, Pixels);

            Mat hsvImage;
            cvtColor(image, hsvImage, COLOR_BGR2HSV);

            for (int y = 0; y < Height; ++y)
            {
                for (int x = 0; x < Width; ++x)
                {
                    Vec3b pixel = hsvImage.at<Vec3b>(y, x);
                    if (IsRedPixel(pixel))
                    {
                        totalX += x;
                        totalY += y;
                        totalRedPixels++;
                    }
                }
            }

            if (GetAsyncKeyState(VK_SHIFT) & 0x8000 && totalRedPixels > 5)
            {
                int averageX = totalX / totalRedPixels;
                int averageY = totalY / totalRedPixels;


                int deltaX = (averageX + centerX) - (Area.left + Area.right) / 2;
                int deltaY = (averageY + centerY) - (Area.top + Area.bottom) / 2;
                try {
                    mouse_event(MOUSEEVENTF_MOVE, deltaX, deltaY - 10, 0, 0);
                }
                catch (...){}
            }


        }
    }
}

int main()
{
    HWND win = GetDesktopWindow();
    HDC dc = GetDC(win);

    HDCToMemory(dc, { 0, 0, 1920, 1080 });
}
