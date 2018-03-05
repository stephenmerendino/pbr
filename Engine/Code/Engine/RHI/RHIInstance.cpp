#include "Engine/RHI/RHIInstance.hpp"
#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/log.h"
#include "Engine/Core/Config.hpp"
#include "Engine/Profile/gpu_profile.h"
#include "Engine/Math/MathUtils.hpp"

RHIInstance& RHIInstance::GetInstance()
{
	static RHIInstance* instance = new RHIInstance();
	return *instance;
}

static unsigned int ConfigureDeviceFlags()
{
	unsigned int deviceFlags = 0U;

	#if defined(_DEBUG)
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	return deviceFlags;
}

static DXGI_SWAP_CHAIN_DESC ConfigureSwapChainDescription(Window* window, const anti_aliasing_desc_t& aa_settings)
{
	UNUSED(aa_settings);

	// Setup our Swap Chain
	// For options, see;
	// https://www.google.com/webhp?sourceid=chrome-instant&ion=1&espv=2&ie=UTF-8#safe=off&q=DXGI_SWAP_CHAIN_DESC
	DXGI_SWAP_CHAIN_DESC swapChainDescription;
	memset(&swapChainDescription, 0, sizeof(swapChainDescription));

	// fill the swap chain description struct
	swapChainDescription.BufferCount = 2;                                    // two buffers (one front, one back?)

	swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT       // how swap chain is to be used
		| DXGI_USAGE_BACK_BUFFER;
	swapChainDescription.OutputWindow = window->GetHandle();              // the window to be copied to on present

	//if(aa_settings.m_msaa_enabled && aa_settings.m_msaa_sample_count > 1){
	//	swapChainDescription.SampleDesc.Quality = aa_settings.m_msaa_quality;                               // how many multisamples (1 means no multi sampling)
	//	swapChainDescription.SampleDesc.Count = aa_settings.m_msaa_sample_count;                               // how many multisamples (1 means no multi sampling)
	//} else{
		swapChainDescription.SampleDesc.Quality = 0;                               // how many multisamples (1 means no multi sampling)
		swapChainDescription.SampleDesc.Count = 1;                               // how many multisamples (1 means no multi sampling)
	//}

	swapChainDescription.Windowed = TRUE;                                    // windowed/full-screen mode
	swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	swapChainDescription.BufferDesc.Width = window->GetWidth();
	swapChainDescription.BufferDesc.Height = window->GetHeight();

	return swapChainDescription;
}

bool RHIInstance::CreateOuput(RHIDeviceContext** out_rhiDeviceContext,
							  RHIOutput** out_rhiOutput,
							  const unsigned int width,
							  const unsigned int height,
							  RHIOutputMode initialOutputMode)
{
	Window*	window;
	RHIOutput::ConfigureWindow(&window, width, height, initialOutputMode);
	window->Open();

	ID3D11Device*			dxDevice		= nullptr;
	ID3D11DeviceContext*	dxDeviceContext	= nullptr;
	IDXGISwapChain*			dxSwapChain		= nullptr;

	HRESULT hr;

	// create device
	unsigned int deviceFlags = ConfigureDeviceFlags();

	const D3D_FEATURE_LEVEL features[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	//--------------------------------------------------------------------------------------------------------------------

	/**/

	D3D_FEATURE_LEVEL device_feature_level;
	hr = D3D11CreateDevice(0,
						   D3D_DRIVER_TYPE_HARDWARE,
						   0,
		                   deviceFlags, 
						   features, 2, 
						   D3D11_SDK_VERSION, 
						   &dxDevice, 
						   &device_feature_level, 
						   &dxDeviceContext);

	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create dx device");

	// get aa settings
	anti_aliasing_desc_t aa_settings;
	MemZero(&aa_settings);

	ConfigGetBool(&aa_settings.m_msaa_enabled, "msaa_enabled");
	ConfigGetInt((int*)&aa_settings.m_msaa_sample_count, "msaa_sample_count");

	ConfigGetBool(&aa_settings.m_ssaa_enabled, "ssaa_enabled");
	ConfigGetInt((int*)&aa_settings.m_ssaa_level, "ssaa_level");

	if(false == aa_settings.m_ssaa_enabled){
		aa_settings.m_ssaa_level = 1;
	}

	ConfigGetBool(&aa_settings.m_fxaa_enabled, "fxaa_enabled");

	// create swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDescription = ConfigureSwapChainDescription(window, aa_settings);

	IDXGIDevice* dxgiDevice = nullptr;
	hr = dxDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to get dxgi device from d3d device");

	IDXGIAdapter* dxgiAdapter = nullptr;
	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to get dxgi adapter from dxgi device");

	IDXGIFactory* dxgiFactory = nullptr;
	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to get dxgi factory from dxgi adapter");

	hr = dxgiFactory->CreateSwapChain(dxDevice, &swapChainDescription, &dxSwapChain);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to create swap chain");

	/*/

	//--------------------------------------------------------------------------------------------------------------------
	// Actually Create
	DXGI_SWAP_CHAIN_DESC swapChainDescription = ConfigureSwapChainDescription(window, false, 0);
	hr = D3D11CreateDeviceAndSwapChain( nullptr,					// Adapter, if nullptr, will use adapter window is primarily on.
										D3D_DRIVER_TYPE_HARDWARE,	// Driver Type - We want to use the GPU (HARDWARE)
										nullptr,					// Software Module - DLL that implements software mode (we do not use)
										deviceFlags,				// device creation options
										features,                    // feature level (use default)
										2U,                         // number of feature levels to attempt
										D3D11_SDK_VERSION,          // SDK Version to use
										&swapChainDescription,      // Description of our swap chain
										&dxSwapChain,             // Swap Chain we're creating
										&dxDevice,                // [out] The device created
										nullptr,                    // [out] Feature Level Acquired
										&dxDeviceContext);        // Context that can issue commands on this pipe.

	/**/

	// query for msaa support
	if(aa_settings.m_msaa_enabled){
		hr = dxDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, aa_settings.m_msaa_sample_count, &aa_settings.m_msaa_quality);

		if(0 == aa_settings.m_msaa_quality){
			aa_settings.m_msaa_enabled = false;
			aa_settings.m_msaa_sample_count = 0;
			log_printf("Device does not support %ux msaa, falling back to no msaa", aa_settings.m_msaa_sample_count);
		}

		aa_settings.m_msaa_quality -= 1;
	} else if(aa_settings.m_msaa_sample_count > 1){
		log_printf("MSAA is disabled in config, but sample count is still set to %ux. Disabling MSAA and setting sample count to 1.", aa_settings.m_msaa_sample_count);
		aa_settings.m_msaa_sample_count = 1;
		aa_settings.m_msaa_quality = 0;
	}

	if(SUCCEEDED(hr)){
		RHIDevice* device		= new RHIDevice(dxDevice);
		*out_rhiDeviceContext	= new RHIDeviceContext(device, dxDeviceContext);
		*out_rhiOutput			= new RHIOutput(device, window, dxSwapChain, aa_settings);

        gpu_profile_init(device);

		device->m_dxDevice = dxDevice;
		device->m_immediateContext = *out_rhiDeviceContext;

		return true;
	} else{
		DebuggerPrintf("Failed to Create Device and Swap Chain [Error: %u]\n", GetLastError());
		SAFE_DELETE(window);
		return false;
	}
}

Matrix4 RHIInstance::CreateOrthoProjection(float nx, float fx, float ny, float fy, float nz, float fz)
{
	float sx = 1.0f / (fx - nx);
	float sy = 1.0f / (fy - ny);
	float sz = 1.0f / (fz - nz);

	//float projection[] = {
	//	2.0f * sx,			0.0f,				0.0f,			0.0f,
	//	0.0f,				2.0f * sy,			0.0f,			0.0f,
	//	0.0f,				0.0f,				sz,				0.0f,
	//	-(fx + nx) * sx, 	-(fy + ny) * sy,	-nz * sz,		1.0f
	//};

	float projection[] = {
		2.0f * sx,			0.0f,				0.0f,			-(fx + nx) * sx,
		0.0f,				2.0f * sy,			0.0f,			-(fy + ny) * sy,
		0.0f,				0.0f,				sz,				-nz * sz,
		0.0f, 				0.0f,				0.0f,			1.0f
	};

	return Matrix4(projection);
}

Matrix4 RHIInstance::CreatePerspectiveProjection(float nz, float fz, float viewingAngleDegrees, float aspect)
{
	float halfViewingAngleDegrees = viewingAngleDegrees / 2.0f;
	float inverseAspect = 1.0f / aspect;
	float oneOverTanView = 1.0f / TanDegrees(halfViewingAngleDegrees);

	//float perspective[] = {
	//	oneOverTanView * inverseAspect,		0.0f,				0.0f,						0.0f,
	//	0.0f,								oneOverTanView,		0.0f,						0.0f,
	//	0.0f,								0.0f,				fz / (fz - nz),				1.0f,
	//	0.0f,								0.0f,				-nz * fz / (fz - nz),		0.0f
	//};

	float perspective[] = {
		oneOverTanView * inverseAspect,		0.0f,				0.0f,						0.0f,
		0.0f,								oneOverTanView,		0.0f,						0.0f,
		0.0f,								0.0f,				fz / (fz - nz),				-nz * fz / (fz - nz),
		0.0f,								0.0f,				1.0f,						0.0f
	};
	
	return Matrix4(perspective);
}