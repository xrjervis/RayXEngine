#pragma once
#include "Engine/Renderer/d3d11/RHIDevice.hpp"
#include "Engine/Renderer/d3d11/ImmediateRenderer.hpp"
#include "Engine/Renderer/d3d11/FontRenderer.hpp"
#include <memory>
#include <vector>
#include <map>

class Window;
class ImmediateRenderer;

class RHIInstance {
public:
	RHIInstance();
	~RHIInstance();

	RHIDevice*			GetDevice() const { return m_device.get(); }
	ImmediateRenderer*	GetImmediateRenderer() const { return m_immediateRenderer.get(); }
	FontRenderer*		GetFontRenderer() const { return m_fontRenderer.get(); }

private:
	bool				CreateDeviceAndContext(const HardwareDeviceInfo_t& hardware_info);
	bool				CreateFactory();
	void				FindHardwareDevices();
	void				CreateImmediateRenderer();
	void				CreateFontRenderer();

public:
	std::unique_ptr<RHIDevice>			m_device;
	std::unique_ptr<ImmediateRenderer>	m_immediateRenderer;
	std::unique_ptr<FontRenderer>		m_fontRenderer;

	std::vector<HardwareDeviceInfo_t>	m_hardwareDevices;
	ComPtr<IDXGIFactory2>				m_dxgiFactory;
	ComPtr<ID3D11Debug>					m_debugInterface;
};

