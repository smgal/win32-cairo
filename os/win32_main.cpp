
#pragma comment(lib, "libcairo.lib")

#include <cairo.h>
#include <windows.h>

namespace win32
{
	void prepareWindow(int width, int height);
	void flush(cairo_t* p_cairo);
	void wait(void);
}

extern "C" void cairo_main(void);

#define	WINDOW_CLASS "CairoWindowClass"

namespace
{
	HWND g_h_wnd = 0;
	HDC g_h_dc  = 0;
	bool g_not_terminated = true;

	LRESULT CALLBACK WndProc(HWND g_h_wnd, UINT message, WPARAM w_param, LPARAM l_param)
	{
		switch (message)
		{
			//case WM_ERASEBKGND:
			case WM_PAINT:
			{
				PAINTSTRUCT ps;
				g_h_dc = BeginPaint(g_h_wnd, &ps);

				cairo_main();

				EndPaint(g_h_wnd, &ps);
				g_h_dc = 0;

				break;
			}

			case WM_CLOSE:
				g_not_terminated = false;
				break;

			default:
				break;
		}

		return DefWindowProc(g_h_wnd, message, w_param, l_param);
	}

	HWND CreateNativeWindow(int screen, unsigned int width, unsigned int height, unsigned int depth)
	{
		HINSTANCE h_instance = GetModuleHandle(NULL);

		// Register windows class
		{
			WNDCLASS window_class;

			window_class.style         = CS_HREDRAW | CS_VREDRAW;
			window_class.lpfnWndProc   = WndProc;
			window_class.cbClsExtra    = 0;
			window_class.cbWndExtra    = 0;
			window_class.hInstance     = h_instance;
			window_class.hIcon         = 0;
			window_class.hCursor       = 0;
			window_class.lpszMenuName  = 0;
			window_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
			window_class.lpszClassName = WINDOW_CLASS;

			ATOM registerClass         = RegisterClass(&window_class);

			if (!registerClass)
			{
				MessageBox(0, "Failed to register the window class", "Error", MB_OK | MB_ICONEXCLAMATION);
			}
		}

		// Create window
		{
			::RECT	window_rect;
			SetRect(&window_rect, 0, 0, width, height);

			AdjustWindowRectEx(&window_rect, WS_CAPTION | WS_SYSMENU, false, 0);

			int x = 0;
			int y = 0;

			g_h_wnd = CreateWindow
			(
				WINDOW_CLASS
				, "Cairo graphics"
				, WS_THICKFRAME | WS_VISIBLE | WS_SYSMENU
				, x, y, window_rect.right-window_rect.left, window_rect.bottom-window_rect.top
				, NULL, NULL
				, (HINSTANCE)h_instance, NULL
			);
		}

		return g_h_wnd;
	}

	void DestroyNativeWindow(void)
	{
		DestroyWindow(g_h_wnd);
		g_h_wnd = 0;
	}

	bool ProcessMessage(unsigned long delay_time)
	{
		MSG msg;

		while (::PeekMessage(&msg, g_h_wnd, NULL, NULL, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		::Sleep(delay_time);

		return true;
	}

}

void win32::prepareWindow(int width, int height)
{
	CreateNativeWindow(0, width, height, 32);
}

void win32::flush(cairo_t* p_cairo)
{
	cairo_surface_t* p_cairo_surface = cairo_get_target(p_cairo);

	unsigned long* p_buffer = (unsigned long*)cairo_image_surface_get_data(p_cairo_surface);
	int width = cairo_image_surface_get_width(p_cairo_surface);
	int height = cairo_image_surface_get_height(p_cairo_surface);
	int stride = cairo_image_surface_get_stride(p_cairo_surface);
	cairo_format_t format = cairo_image_surface_get_format(p_cairo_surface);
	int pixel_bytes = 0;

	switch (format)
	{
	case CAIRO_FORMAT_ARGB32:
	case CAIRO_FORMAT_RGB24:
		pixel_bytes = 4;
		break;
	case CAIRO_FORMAT_A8:
		pixel_bytes = 1;
		break;
	case CAIRO_FORMAT_A1:
		pixel_bytes = 1;
		break;
	}

	unsigned char bitmap_info_buffer[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
	memset(bitmap_info_buffer, 0, sizeof(bitmap_info_buffer));
	BITMAPINFO& bitmap_info = *(BITMAPINFO*)(&bitmap_info_buffer);

	{
		memset(&bitmap_info, 0, sizeof(bitmap_info));

		bitmap_info.bmiHeader.biSize = sizeof(bitmap_info);
		bitmap_info.bmiHeader.biWidth = width;
		bitmap_info.bmiHeader.biHeight = -height;
		bitmap_info.bmiHeader.biPlanes = 1;
		bitmap_info.bmiHeader.biBitCount = pixel_bytes * 8;
		bitmap_info.bmiHeader.biCompression = BI_RGB;
		bitmap_info.bmiHeader.biSizeImage = stride * height;
		bitmap_info.bmiHeader.biClrUsed = 0;

		if (pixel_bytes == 1)
		{
			RGBQUAD* p_rgb = &bitmap_info.bmiColors[0];
			RGBQUAD* p_end = p_rgb + 256;

			unsigned char alpha = 0;

			for ( ; p_rgb < p_end; ++p_rgb, ++alpha)
			{
				p_rgb->rgbRed = p_rgb->rgbGreen = p_rgb->rgbBlue = alpha;
				p_rgb->rgbReserved = 0xFF;
			}

		}
	}

	::SetDIBitsToDevice
	(
		g_h_dc
		, 0, 0, width, height
		, 0, 0, 0, height
		, p_buffer, (BITMAPINFO*)&bitmap_info, DIB_RGB_COLORS
	);
}

void win32::wait(void)
{
	while (g_not_terminated && ProcessMessage(10))
		;

	DestroyNativeWindow();
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	win32::prepareWindow(500, 500);

	win32::wait();

	return 0;
}
