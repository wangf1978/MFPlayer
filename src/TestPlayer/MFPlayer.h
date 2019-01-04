#include <Windows.h>
#include <wrl/client.h>
#include <mfidl.h>

#pragma once

using namespace Microsoft::WRL;

class CMediaSessionPlayer : public IMFAsyncCallback
{
public:
	CMediaSessionPlayer();
	virtual ~CMediaSessionPlayer();

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);
	virtual ULONG   STDMETHODCALLTYPE AddRef(void);
	virtual ULONG   STDMETHODCALLTYPE Release(void);

	virtual HRESULT Open(HWND hwnd);
	virtual HRESULT Stop();
	virtual HRESULT Play();
	virtual HRESULT Pause(BOOL bPauseOff = FALSE);
	virtual HRESULT SetRate(float flRate);
	virtual HRESULT FastForward();
	virtual HRESULT Rewind();
	virtual HRESULT GetTime(MFTIME* phnsTime);

	//
	// Interface IMFAsyncCallback
	//
	virtual HRESULT STDMETHODCALLTYPE GetParameters(DWORD *pdwFlags, DWORD *pdwQueue);
	virtual HRESULT STDMETHODCALLTYPE Invoke(IMFAsyncResult *pAsyncResult);

public:
	static void RegisterETW();
	static void UnregisterETW();

private:
	HRESULT		UpdateSARPrefType();

public:
	ComPtr<IMFMediaSession>	m_spMediaSession;
	ComPtr<IMFMediaSource>	m_spSource;
	ComPtr<IMFActivate>		m_spEVRActivate;
	ComPtr<IMFActivate>		m_spSARActivate;
	ComPtr<IMFMediaType>	m_SARPrefType;
	ComPtr<IMFPresentationClock> 
							m_spSessionClock;

	BOOL					m_bTopologFailed;
	LONG volatile			m_cRef;

	HANDLE					m_hOpenReadyEvent;
	HANDLE					m_hRateReady;
	HANDLE					m_hStopEvent;

	BOOL					m_bPaused;
	float					m_flRate;

	TOPOID					m_SARID;

	MFTIME					m_hnsStartTime;
	MFTIME					m_hnsOffsetTime;
	MFTIME					m_hnsMediastartOffset;
	BOOL					m_fReceivedTime;
};
