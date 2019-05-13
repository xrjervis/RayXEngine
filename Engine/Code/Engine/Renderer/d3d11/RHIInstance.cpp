#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Window.hpp"
#include <vector>

RHIInstance::RHIInstance() {
	CreateFactory();
	FindHardwareDevices();
	CreateDeviceAndContext(m_hardwareDevices[0]);
	CreateImmediateRenderer();
	CreateFontRenderer();
}

RHIInstance::~RHIInstance() {
	m_device.reset();
	m_fontRenderer.reset();
	m_immediateRenderer.reset();
#if defined(_DEBUG)  
	m_debugInterface->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
}

bool RHIInstance::CreateDeviceAndContext(const HardwareDeviceInfo_t& hardwareInfo) {
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)  
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Determine DirectX hardware feature levels this app will support.
	std::vector<D3D_FEATURE_LEVEL> featureLevels = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	UINT numFeatureLevels = (UINT)featureLevels.size();
	// Create the Direct3D 11 API device object and a corresponding context.
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGIAdapter1> adapter;

	// Get adapter from index
	m_dxgiFactory->EnumAdapters1(hardwareInfo.adapterIndex, adapter.ReleaseAndGetAddressOf());

	HRESULT hr = ::D3D11CreateDevice(
		adapter.Get(),
		D3D_DRIVER_TYPE_UNKNOWN,
		nullptr,
		creationFlags,
		featureLevels.data(),
		numFeatureLevels,
		D3D11_SDK_VERSION,
		device.GetAddressOf(),  // Returns the Direct3D device created.
		nullptr,     // Returns feature level of device created.
		context.GetAddressOf()  // Returns the device immediate context.
	);

	if (FAILED(hr)) {
		ERROR_RECOVERABLE("Could not create debug device. Perhaps need to install the Windows 10 SDK?");
		return false;
	}

	m_device = std::make_unique<RHIDevice>(this, hardwareInfo);

	HR(device.As(&m_device->m_d3d11Device));
	HR(context.As(&m_device->m_d3d11Context));

	//--------------------------------------------------------------------
	// Acquire the debug interface
	HR(m_device->m_d3d11Device->QueryInterface(IID_PPV_ARGS(m_debugInterface.GetAddressOf())));

	return true;
}

bool RHIInstance::CreateFactory() {
	HRESULT hr = ::CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf()));
	return SUCCEEDED(hr);
}

void RHIInstance::FindHardwareDevices() {
	UINT idx = 0;
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIFactory6> factory;
	HR(m_dxgiFactory.As(&factory));

	while(true){
		HRESULT hr = factory->EnumAdapterByGpuPreference(idx, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
		if(hr == DXGI_ERROR_NOT_FOUND){
			break;
		}
		else{
			HardwareDeviceInfo_t info;
			info.adapterIndex = idx;
			adapter->GetDesc1(&info.adapterDesc);

			// Don't select the Basic Render Driver adapter.
			if ((info.adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == false) {
				m_hardwareDevices.push_back(info);
			}
			idx++;
		}
	}

	if(m_hardwareDevices.empty()){
		ERROR_AND_DIE("Could not find any physical rendering device.");
	}
}

void RHIInstance::CreateImmediateRenderer() {
	m_immediateRenderer = std::make_unique<ImmediateRenderer>(m_device.get());
}

void RHIInstance::CreateFontRenderer() {
	m_fontRenderer = std::make_unique<FontRenderer>();
}

