#include "CommonTypes.h"
#include "SerialPort.h"
#include <memory>
#include <algorithm>
#include <math.h>



HRESULT textureToPixels(_Inout_ ID3D11Texture2D* SharedSurf, _In_ DXGI_OUTPUT_DESC* DeskDesc, ID3D11Device* device, ID3D11DeviceContext* context, BYTE** pixels);
HRESULT process(_Inout_ ID3D11Texture2D* SharedSurf, _In_ DXGI_OUTPUT_DESC* DeskDesc, ID3D11Device* device, ID3D11DeviceContext* context);

