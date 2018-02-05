#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <d3dcompiler.h>

//include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")

//screen resolution
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

//globals
IDXGISwapChain *swapchain;
ID3D11Device *dev;
ID3D11DeviceContext *devcon;
ID3D11RenderTargetView *backbuffer;
ID3D11InputLayout *pLayout;
ID3D11VertexShader *pVS;
ID3D11PixelShader *pPS;
ID3D11Buffer *pVBuffer;

//structs
struct VERTEX
{
	FLOAT X, Y, Z;		//position
	D3DXCOLOR Color;	//color
};


//prototypes
void InitD3D(HWND hWnd);
void RenderFrame(void);
void CleanD3D(void);
void InitPipeline(void);
void InitGraphics(void);


/*
	prototype for WndProc
*/
LRESULT CALLBACK WindowProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

/*
	Entry point for windows program
*/
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	
	HWND hWnd;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wc.hbrBackground = (HBRUSH)COLOR_WINDOW; //not needed for fullscreen
	wc.lpszClassName = (LPCSTR)"WindowClass";

	RegisterClassEx(&wc);

	RECT wr = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	hWnd = CreateWindowEx(NULL,
		(LPCSTR)"WindowClass",
		(LPCSTR)"Our First Direct3D Program",
		WS_OVERLAPPEDWINDOW,
		300,
		300,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hWnd, nShowCmd);

	//initialize D3D
	InitD3D(hWnd);

	//MSG is a struct that holds Windows events messages
	MSG msg;

	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}
		else
		{
			//run game code...
			RenderFrame();
		}	
	}
	CleanD3D();
	return msg.wParam;
}

/*
	main message handler for the program
*/
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
	DirectX 11 setup functions
*/

void InitD3D(HWND hWnd)
{
	//swap chain description struct
	DXGI_SWAP_CHAIN_DESC scd;

	//clear out structs memory to zero
	ZeroMemory(&scd, sizeof(scd));

	//fill out the swap chain description struct
	scd.BufferCount = 1; //one back buffer; could have more
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Width = SCREEN_WIDTH;
	scd.BufferDesc.Height = SCREEN_HEIGHT;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 4; //MSAA
	scd.SampleDesc.Quality = 0;
	scd.Windowed = TRUE;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //allows full-screen switching (ALT-TAB)

	//create the device, device context, and swap chain
	HRESULT error;
	error = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&swapchain,
		&dev,
		NULL,
		&devcon);

	//get address of backbuffer
	ID3D11Texture2D *pBackBuffer;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	//use the backbuffer address to create the render target
	dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	pBackBuffer->Release();

	//set the render target as the back buffer
	devcon->OMSetRenderTargets(1, &backbuffer, NULL);

	//set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;

	devcon->RSSetViewports(1, &viewport);

	InitPipeline();
	InitGraphics();
}

void InitPipeline()
{
	//load and compile shaders
	ID3D10Blob *vs, *ps;
	D3DX11CompileFromFile("shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &vs, 0, 0);
	D3DX11CompileFromFile("shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &ps, 0, 0);

	//encapsulate the shaders into shader objects
	dev->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &pVS);
	dev->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, &pPS);

	//set active shaders
	devcon->VSSetShader(pVS, 0, 0);
	devcon->PSSetShader(pPS, 0, 0);

	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	dev->CreateInputLayout(ied, 2, vs->GetBufferPointer(), vs->GetBufferSize(), &pLayout);
	devcon->IASetInputLayout(pLayout);
}

void InitGraphics()
{
	VERTEX triangleVerts[] =
	{
		{ -0.45f, -0.5f, 0.0f, D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f) },
		{ 0.0f, 0.5f, 0.0f, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f) },
	{ 0.45f, -0.5, 0.0f, D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f) },
	{ 1.0f, 0.5f, 0.0f, D3DXCOLOR(0.0f, 1.0f, 0.70f, 1.0f) },
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	dev->CreateBuffer(&bd, NULL, &pVBuffer);

	D3D11_MAPPED_SUBRESOURCE ms;
	devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, triangleVerts, sizeof(triangleVerts));
	devcon->Unmap(pVBuffer, NULL);
}

void RenderFrame(void)
{
	//clear the backbuffer to a colour
	devcon->ClearRenderTargetView(backbuffer, D3DXCOLOR(0.45f, 0.2f, 0.4f, 1.0f));

	//do 3D rendering on the backbuffer
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
	devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	devcon->Draw(4, 0);

	//switch the backbuffer and the frontbuffer
	swapchain->Present(0, 0);
}

void CleanD3D(void)
{
	swapchain->SetFullscreenState(FALSE, NULL);//switch back to windowed mode for clean up (DX is incapable of cleaning up when in FS mode)
	//release all COM objects
	pLayout->Release();
	pVS->Release();
	pPS->Release();
	pVBuffer->Release();
	swapchain->Release();
	backbuffer->Release();
	dev->Release();
	devcon->Release();
}

