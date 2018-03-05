#pragma once

#include "Engine/RHI/RHITypes.hpp"
#include "Engine/Core/types.h"

class RHIDevice;
class Window;
struct IDXGISwapChain;
class RHITexture2D;
class RHITextureBase;

struct anti_aliasing_desc_t
{
	bool m_msaa_enabled;
	uint m_msaa_sample_count;
	uint m_msaa_quality;

	bool m_ssaa_enabled;
	uint m_ssaa_level;

	bool m_fxaa_enabled;
};

class RHIOutput
{
public:
	RHIDevice*			m_device;
	Window*				m_window;
	IDXGISwapChain*		m_dxSwapChain;
	RHITexture2D*		m_renderTarget;

	anti_aliasing_desc_t m_aa_settings;

public:
	RHIOutput(RHIDevice *owner, Window *window, IDXGISwapChain* dx11SwapChain, const anti_aliasing_desc_t& aa_settings);
	~RHIOutput();

	void Present();
	void Close();

public:
	unsigned int	GetWidth() const;
	unsigned int	GetHeight() const;
	float			GetAspectRatio() const;
	bool			IsOpen() const;

	RHIDevice*		GetRHIDevice() { return m_device; }
	Window*			GetWindow() { return m_window; }
	RHITexture2D*	GetRenderTarget() { return m_renderTarget; }

public:
	void			SetDisplayMode(const RHIOutputMode rhiDisplayMode);
	bool			SetDisplaySize(int width, int height);
	void			SetDisplayTitle(const char* title);
	void			CenterDisplay();
	void			ProcessMessages();

public:
	static void		ConfigureWindow(Window** out_window, int width, int height, RHIOutputMode outputMode = RHI_OUTPUT_MODE_WINDOWED);

private:
	void			CreateRenderTarget();
};