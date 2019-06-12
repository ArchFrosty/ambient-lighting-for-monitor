#include "Processing.h"
#include "bmp.h"

#define NLEDS_LEFT 16
#define NLEDS_TOP 32
#define NLEDS_RIGHT 16
#define NLEDS_BOTTOM 32

#pragma pack(push, 1)
struct RGBA
{
	BYTE r;
	BYTE g;
	BYTE b;
	BYTE a;
};
#pragma pack(pop)

struct RGB
{
	BYTE r;
	BYTE g;
	BYTE b;
};

//coordinate transformation macros
//get row number from vector index
#define r1dt2d(x) max(0, ceil(x/ImageWidth) - 1)
//get column number from vector index
#define c1dt2d(x) (x % ImageWidth)
//get vector index from row and column indices
#define a2dt1d(row, col) (row * ImageWidth + col)

char* portName = "COM3";
SerialPort *arduino = NULL;
UINT ImageWidth = 0;
UINT ImageHeight = 0;

//TODO cleanup, call ProcessFailiure correctly in desktop duplication - return error
HRESULT textureToPixels(_Inout_ ID3D11Texture2D* SharedSurf, _In_ DXGI_OUTPUT_DESC* DeskDesc, ID3D11Device* device, ID3D11DeviceContext* context, BYTE** pixels)
{
	//Variable Declaration
	ID3D11Texture2D*        lDestImage = NULL;
	D3D11_TEXTURE2D_DESC description;
	UCHAR*                  g_iMageBuffer = nullptr;
	ImageWidth = DeskDesc->DesktopCoordinates.right - DeskDesc->DesktopCoordinates.left;
	ImageHeight = DeskDesc->DesktopCoordinates.bottom - DeskDesc->DesktopCoordinates.top;

	SharedSurf->GetDesc(&description);
	description.BindFlags = 0;
	description.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	description.Usage = D3D11_USAGE_STAGING;
	description.MiscFlags = 0; // because fuck you thats why

	HRESULT hr = device->CreateTexture2D(&description, NULL, &lDestImage);
	if (FAILED(hr))
	{
		if (hr == E_OUTOFMEMORY) {
			printf("GetImageData - CreateTexture2D - OUT OF MEMORY \n");
		}
		if (lDestImage)
		{
			lDestImage->Release();
			lDestImage = NULL;
		}
		return hr;
	}

	// Copy image into GDI drawing texture
	context->CopyResource(lDestImage, SharedSurf);

	// Copy GPU Resource to CPU
	D3D11_TEXTURE2D_DESC desc;
	lDestImage->GetDesc(&desc);
	D3D11_MAPPED_SUBRESOURCE resource;
	UINT subresource = D3D11CalcSubresource(0, 0, 0);

	hr = context->Map(lDestImage, subresource, D3D11_MAP_READ, 0, &resource);
	if (FAILED(hr))
	{
		printf("GetImageData - Map - FAILED \n");
		lDestImage->Release();
		lDestImage = NULL;
		return hr;
	}

	std::unique_ptr<BYTE> pBuf(new BYTE[resource.RowPitch*desc.Height]);
	UINT lBmpRowPitch = ImageWidth * 4;
	BYTE* sptr = reinterpret_cast<BYTE*>(resource.pData);
	BYTE* dptr = pBuf.get() + resource.RowPitch*desc.Height - lBmpRowPitch;
	UINT lRowPitch = std::min<UINT>(lBmpRowPitch, resource.RowPitch);

	for (size_t h = 0; h < ImageHeight; ++h)
	{
		memcpy_s(dptr, lBmpRowPitch, sptr, lRowPitch);
		sptr += resource.RowPitch;
		dptr -= lBmpRowPitch;
	}

	context->Unmap(lDestImage, subresource);
	lDestImage->Release();
	long g_captureSize = lRowPitch * desc.Height;
	//g_iMageBuffer = new UCHAR[g_captureSize]; <- was this guy smoking crack ? wtf did this even do ?
	g_iMageBuffer = (UCHAR*)malloc(g_captureSize);

	//Copying to UCHAR buffer 
	memcpy(g_iMageBuffer, pBuf.get(), g_captureSize);
	pBuf.reset();
	*pixels = g_iMageBuffer;
	return hr;
}

HRESULT process(_Inout_ ID3D11Texture2D* SharedSurf, _In_ DXGI_OUTPUT_DESC* DeskDesc, ID3D11Device* device, ID3D11DeviceContext* context) {
	BYTE* rawImage = NULL;
	int AVGING_SIZE = 32; //size of the averaging kernel the size is a x by x square

	HRESULT hr = textureToPixels(SharedSurf, DeskDesc, device, context, &rawImage);
	if (FAILED(hr)) {
		free(rawImage);
		return hr;
	}

	//make these local variables in attempt to make the code faster - glabal variables are never assigned to registers
	int locImH = ImageHeight;
	int locImW = ImageWidth;

	//covert raw byte array to array of pixel structs, so that it is easier to manipulate
	RGBA *image = reinterpret_cast<RGBA*>(rawImage);





	/*
	//save image as bmp for debug
	RGB *rgbImage = (RGB*)malloc(locImH * locImW * 3);

	for (int i = 0; i < locImH * locImW; i++)
	{
		rgbImage[i].r = image[i].b;
		rgbImage[i].g = image[i].g;
		rgbImage[i].b = image[i].r;
	}

	raw2bmp((void*)rgbImage, "C:\\Users\\igorf\\Desktop\\C++\\fuckmedaddy.bmp", locImW, locImH);
	*/



	//NOTE: WE CANNOT AVERAGE CORRECTLY IN CORNERS SO THE VALUES ARE JUST COPIED FROM THE LAST CORRECT AVERAGE
	//		ALSO ALL CORNER PIXELS ARE IN THE FINAL ARRAY TWICE SO WE CAN HAVE NICE NUMBERS TO WORK WITH PER SIDE - THE ACTUAL RESOLUTION OF MONITOR

	RGB* averagedEdge = (RGB*)malloc((locImH * 2 + locImW * 2) * 3);
	UINT apos = 0, r = 0, g = 0, b = 0, a = 0;

	UINT AVG_COUNT = pow(AVGING_SIZE, 2);


	//!!!! TODO remove asserts
	//Todo maybe try interpoaltion instead of duplicating values

	int SKIP = 8; //number of pixels to skip when averaging

	// AT THIS POINT THE IMAGE IS UPSIDE DOWN AND R AND B COLOR CHANNELS ARE SWAPPED - why? I have no fucking idea, just deal with it
	//----------------------------------------------
	//from TOP LEFT to BOTTOM LEFT
	//the condition in this outer loop is arbitrary, because the loop will get terminated with a break anyway
	for (int i = 0; true; i += SKIP)
	{
		r = 0;
		g = 0;
		b = 0;
		//columns
		for (int j = 0; j < AVGING_SIZE; j++)
		{
			//rows
			for (int k = i; k < i + AVGING_SIZE; k++)
			{
				assert(k >= 0 && j >= 0);
				r += image[a2dt1d(k, j)].r;
				g += image[a2dt1d(k, j)].g;
				b += image[a2dt1d(k, j)].b;
			}
		}
		averagedEdge[apos].r = round(b / AVG_COUNT);
		averagedEdge[apos].g = round(g / AVG_COUNT);
		averagedEdge[apos].b = round(r / AVG_COUNT);
		apos++;

		//duplicate averages of skipped pixels
		for (int s = 0; s < SKIP - 1; s++)
		{
			averagedEdge[apos].r = averagedEdge[apos - 1].r;
			averagedEdge[apos].g = averagedEdge[apos - 1].g;
			averagedEdge[apos].b = averagedEdge[apos - 1].b;
			apos++;
		}
		//if this is the last loop, just duplicate values until the end of this edge and break the outer loop
		if (i + SKIP > locImH - AVGING_SIZE) {
			for (int s = 0; s < locImH - i - SKIP; s++)
			{
				averagedEdge[apos].r = averagedEdge[apos - 1].r;
				averagedEdge[apos].g = averagedEdge[apos - 1].g;
				averagedEdge[apos].b = averagedEdge[apos - 1].b;
				apos++;
			}
			break;
		}
	}

	//--------------------------------------------------
	//from BOTTOM LEFT to BOTTOM RIGHT
	//the condition in this outer loop is arbitrary, because the loop will get terminated with a break anyway
	for (int i = 0; true; i += SKIP)
	{
		r = 0;
		g = 0;
		b = 0;
		//columns
		for (int j = i; j < i + AVGING_SIZE; j++)
		{
			//rows
			for (int k = locImH - 1; k > locImH - 1 - AVGING_SIZE; k--)
			{
				assert(k >= 0 && j >= 0);
				r += image[a2dt1d(k, j)].r;
				g += image[a2dt1d(k, j)].g;
				b += image[a2dt1d(k, j)].b;
			}
		}
		averagedEdge[apos].r = round(b / AVG_COUNT);
		averagedEdge[apos].g = round(g / AVG_COUNT);
		averagedEdge[apos].b = round(r / AVG_COUNT);
		apos++;

		//duplicate averages of skipped pixels
		for (int s = 0; s < SKIP - 1; s++)
		{
			averagedEdge[apos].r = averagedEdge[apos - 1].r;
			averagedEdge[apos].g = averagedEdge[apos - 1].g;
			averagedEdge[apos].b = averagedEdge[apos - 1].b;
			apos++;
		}
		//if this is the last loop, just duplicate values until the end of this edge and break the outer loop
		if (i + SKIP > locImW - AVGING_SIZE) {
			for (int s = 0; s < locImW - i - SKIP; s++)
			{
				averagedEdge[apos].r = averagedEdge[apos - 1].r;
				averagedEdge[apos].g = averagedEdge[apos - 1].g;
				averagedEdge[apos].b = averagedEdge[apos - 1].b;
				apos++;
			}
			break;
		}
	}

	//--------------------------------------------------
	//from BOTTOM RIGHT to TOP RIGHT
	for (int i = locImH - 1; true; i -= SKIP)
	{
		r = 0;
		g = 0;
		b = 0;
		//columns
		for (int j = locImH - 1; j > locImH - AVGING_SIZE; j--)
		{
			//rows
			for (int k = i; k > i - AVGING_SIZE; k--)
			{
				assert(k >= 0 && j >= 0);
				r += image[a2dt1d(k, j)].r;
				g += image[a2dt1d(k, j)].g;
				b += image[a2dt1d(k, j)].b;
			}
		}
		averagedEdge[apos].r = round(b / AVG_COUNT);
		averagedEdge[apos].g = round(g / AVG_COUNT);
		averagedEdge[apos].b = round(r / AVG_COUNT);
		apos++;

		//duplicate averages of skipped pixels
		for (int s = 0; s < SKIP - 1; s++)
		{
			averagedEdge[apos].r = averagedEdge[apos - 1].r;
			averagedEdge[apos].g = averagedEdge[apos - 1].g;
			averagedEdge[apos].b = averagedEdge[apos - 1].b;
			apos++;
		}
		//if this is the last loop, just duplicate values until the end of this edge and break the outer loop
		if (i - SKIP < AVGING_SIZE) {
			for (int s = 0; s < i - SKIP + 1; s++)
			{
				averagedEdge[apos].r = averagedEdge[apos - 1].r;
				averagedEdge[apos].g = averagedEdge[apos - 1].g;
				averagedEdge[apos].b = averagedEdge[apos - 1].b;
				apos++;
			}
			break;
		}
	}

	//--------------------------------------------------
	//from TOP RIGHT to TOP LEFT
	//the condition in this outer loop is arbitrary, because the loop will get terminated with a break anyway
	for (int i = locImW - 1; true; i -= SKIP)
	{
		r = 0;
		g = 0;
		b = 0;
		//columns
		for (int j = i; j > i - AVGING_SIZE; j--)
		{
			//rows
			for (int k = 0; k < AVGING_SIZE; k++)
			{
				assert(k >= 0 && j >= 0);
				r += image[a2dt1d(k, j)].r;
				g += image[a2dt1d(k, j)].g;
				b += image[a2dt1d(k, j)].b;
			}
		}
		averagedEdge[apos].r = round(b / AVG_COUNT);
		averagedEdge[apos].g = round(g / AVG_COUNT);
		averagedEdge[apos].b = round(r / AVG_COUNT);
		apos++;

		//duplicate averages of skipped pixels
		for (int s = 0; s < SKIP - 1; s++)
		{
			averagedEdge[apos].r = averagedEdge[apos - 1].r;
			averagedEdge[apos].g = averagedEdge[apos - 1].g;
			averagedEdge[apos].b = averagedEdge[apos - 1].b;
			apos++;
		}
		//if this is the last loop, just duplicate values until the end of this edge and break the outer loop
		if (i - SKIP < AVGING_SIZE) {
			for (int s = 0; s < i - SKIP + 1; s++)
			{
				averagedEdge[apos].r = averagedEdge[apos - 1].r;
				averagedEdge[apos].g = averagedEdge[apos - 1].g;
				averagedEdge[apos].b = averagedEdge[apos - 1].b;
				apos++;
			}
			break;
		}
	}

	//original image is not needed anymore
	free(image);

	//when calculating final led color values, ignore corners - ie corners have no leds, this also solves the issue of corners being not averaged correctly

	RGB* ledColors = (RGB*)malloc((NLEDS_RIGHT + NLEDS_TOP + NLEDS_LEFT + NLEDS_BOTTOM) * 3);
	int ledIndex = 0;

	//who the fuck thought that it is sensible for round() to return double ? fuck.

	//RIGHT
	double samplePitch = locImH / (NLEDS_RIGHT + 1);
	double currentOffset = 0;

	for (size_t i = 0; i < NLEDS_RIGHT; i++)
	{
		currentOffset += samplePitch;
		int index = round(currentOffset);
		ledColors[ledIndex].r = averagedEdge[index].r;
		ledColors[ledIndex].g = averagedEdge[index].g;
		ledColors[ledIndex].b = averagedEdge[index].b;
		ledIndex++;
	}

	//TOP
	samplePitch = locImW / (NLEDS_TOP + 1);
	currentOffset = locImH;

	for (size_t i = 0; i < NLEDS_TOP; i++)
	{
		currentOffset += samplePitch;
		int index = round(currentOffset);
		ledColors[ledIndex].r = averagedEdge[index].r;
		ledColors[ledIndex].g = averagedEdge[index].g;
		ledColors[ledIndex].b = averagedEdge[index].b;
		ledIndex++;
	}

	//LEFT
	samplePitch = locImH / (NLEDS_LEFT + 1);
	currentOffset = locImH + locImW;

	for (size_t i = 0; i < NLEDS_LEFT; i++)
	{
		currentOffset += samplePitch;
		int index = round(currentOffset);
		ledColors[ledIndex].r = averagedEdge[index].r;
		ledColors[ledIndex].g = averagedEdge[index].g;
		ledColors[ledIndex].b = averagedEdge[index].b;
		ledIndex++;
	}

	//BOTTOM
	samplePitch = locImW / (NLEDS_BOTTOM + 1);
	currentOffset = locImH + locImW + locImH;

	for (size_t i = 0; i < NLEDS_BOTTOM; i++)
	{
		currentOffset += samplePitch;
		int index = round(currentOffset);
		ledColors[ledIndex].r = averagedEdge[index].r;
		ledColors[ledIndex].g = averagedEdge[index].g;
		ledColors[ledIndex].b = averagedEdge[index].b;
		ledIndex++;
	}

	// averaged edge is not needed anymore
	free(averagedEdge);

	//!!!!!!!!!TODO this might be an expensive call
	if(!arduino)
		arduino = new SerialPort(portName);
	if (arduino->isConnected()) {
		bool hasWritten = arduino->writeSerialPort((char*)(ledColors), (NLEDS_RIGHT + NLEDS_TOP + NLEDS_LEFT + NLEDS_BOTTOM) * 3);
	}
	else {

	}

	printf("dicks");
}