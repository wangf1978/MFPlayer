#include "stdafx.h"
#include "MFPlayer.h"

static REGHANDLE		g_ETWHandle = NULL;
static BOOL				g_bETWEnabled = FALSE;
static UCHAR			g_nETWLevel = 0;

std::tuple<UINT, const TCHAR*> session_caps[] = {
	DECL_TUPLE(MFSESSIONCAP_START),
	DECL_TUPLE(MFSESSIONCAP_SEEK),
	DECL_TUPLE(MFSESSIONCAP_PAUSE),
	DECL_TUPLE(MFSESSIONCAP_RATE_FORWARD),
	DECL_TUPLE(MFSESSIONCAP_RATE_REVERSE),
	DECL_TUPLE(MFSESSIONCAP_DOES_NOT_USE_NETWORK),
};

float g_supported_forward_rates[] = {
	2.f, 8.f, 16.f, 100.f
};

float g_supported_rewind_rates[] = {
	-1.f, -2.f, -10.f, -100.f
};

float FindNextForwardRate(float flRate)
{
	int idx = 0;
	for (; idx < _countof(g_supported_forward_rates) && (flRate + 0.01f >= g_supported_forward_rates[idx]); idx++);
	if (idx >= _countof(g_supported_forward_rates))
		return 1.f;	// restore to normal playback rate

	return g_supported_forward_rates[idx];
}

float FindNextRewindRate(float flRate)
{
	int idx = 0;
	for (; idx < _countof(g_supported_rewind_rates) && (flRate <= g_supported_rewind_rates[idx] + 0.01f); idx++);
	if (idx >= _countof(g_supported_rewind_rates))
		return 1.f;	// restore to normal playback rate

	return g_supported_rewind_rates[idx];
}

void NTAPI EnableETWCallback(
	LPCGUID /*SourceId*/,
	ULONG IsEnabled,
	UCHAR Level,
	ULONGLONG /*MatchAnyKeyword*/,
	ULONGLONG /*MatchAllKeywords*/,
	PEVENT_FILTER_DESCRIPTOR /*FilterData*/,
	PVOID /*CallbackContext*/
)
{
	switch (IsEnabled)
	{
	case EVENT_CONTROL_CODE_ENABLE_PROVIDER:
		DPA0("EVENT_CONTROL_CODE_ENABLE_PROVIDER.\r\n");
		g_bETWEnabled = TRUE;
		g_nETWLevel = Level;
		break;

	case EVENT_CONTROL_CODE_DISABLE_PROVIDER:
		DPA0("EVENT_CONTROL_CODE_DISABLE_PROVIDER.\r\n");
		g_bETWEnabled = FALSE;
		g_nETWLevel = 0;
		break;
	}
}

void CMediaSessionPlayer::RegisterETW()
{
	// Initialize the ETW
	// {7A1C37DA-95F8-4B98-88D9-E1FF2D94FD4C}
	static const GUID AMPSDK_Provider_ID =
	{ 0x7a1c37da, 0x95f8, 0x4b98,{ 0x88, 0xd9, 0xe1, 0xff, 0x2d, 0x94, 0xfd, 0x4c } };
	EventRegister(&AMPSDK_Provider_ID, EnableETWCallback, NULL, &g_ETWHandle);
}

void CMediaSessionPlayer::UnregisterETW()
{
	if (g_ETWHandle)
	{
		EventUnregister(g_ETWHandle);
		g_ETWHandle = NULL;
	}
}

HRESULT CMediaSessionPlayer::UpdateSARPrefType()
{
	HRESULT hr = E_UNEXPECTED;
	DWORD count = 0;
	ComPtr<IMFMediaType> spType;
	ComPtr<IMFActivate> spRendererActivate;
	ComPtr<IMFMediaSink> spMediaSink;
	ComPtr<IMFStreamSink> spStreamSink;
	ComPtr<IMFMediaTypeHandler> spHandler;

	CHECK_HR(hr = MFCreateAudioRendererActivate(&spRendererActivate));
	CHECK_HR(hr = spRendererActivate->SetUINT32(MF_AUDIO_RENDERER_ATTRIBUTE_ENDPOINT_ROLE, eMultimedia));
	CHECK_HR(hr = spRendererActivate->ActivateObject(IID_IMFMediaSink, (void**)&spMediaSink));
	CHECK_HR(hr = spMediaSink->GetStreamSinkByIndex(0, &spStreamSink));
	CHECK_HR(hr = spStreamSink->GetMediaTypeHandler(&spHandler));
	CHECK_HR(hr = spHandler->GetMediaTypeCount(&count));
	//CHECK_HR(hr = pHandler->GetMediaTypeByIndex(count - 1, &pType));

	// There may be a spatial format at the end. Look for the last PCM format.
	for (DWORD index = count; index > 0; --index)
	{
		GUID SubType = { 0 };
		CHECK_HR(hr = spHandler->GetMediaTypeByIndex(index - 1, &spType));
		CHECK_HR(hr = spType->GetGUID(MF_MT_SUBTYPE, &SubType));
		if (IsEqualGUID(SubType, MEDIASUBTYPE_PCM) ||
			IsEqualGUID(SubType, MFAudioFormat_Float))
		{
			break;
		}
	}
	if (!spType)
	{
		CHECK_HR(hr = E_NOTFOUND);
	}


	CHECK_HR(hr = MFCreateMediaType(&m_SARPrefType));

	// MF_MT_AUDIO_NUM_CHANNELS, MF_MT_AUDIO_SAMPLES_PER_SECOND & MF_MT_AUDIO_CHANNEL_MASK
	CHECK_HR(hr = spType->CopyAllItems(m_SARPrefType.Get()));

	UINT32 nChannels = 0;
	UINT32 nBlockAlign = 0;
	UINT32 nSamplesPerSec = 0;
	UINT32 wBitsPerSample = 0;
	UINT32 nAvgBytesPerSec = 0;

	// now always 16 bits per sample
	CHECK_HR(hr = spType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &nChannels));
	CHECK_HR(hr = spType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &nSamplesPerSec));
	wBitsPerSample = 16;

	nBlockAlign = nChannels * wBitsPerSample / 8;
	nAvgBytesPerSec = nSamplesPerSec * nBlockAlign;

	CHECK_HR(hr = m_SARPrefType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, wBitsPerSample));
	CHECK_HR(hr = m_SARPrefType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, nBlockAlign));
	CHECK_HR(hr = m_SARPrefType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, nAvgBytesPerSec));
	CHECK_HR(hr = m_SARPrefType->SetGUID(MF_MT_SUBTYPE, MEDIASUBTYPE_PCM));

	CHECK_HR(hr = spHandler->IsMediaTypeSupported(m_SARPrefType.Get(), NULL));
	CHECK_HR(hr = spMediaSink->Shutdown());

done:
	return hr;
}

CMediaSessionPlayer::CMediaSessionPlayer()
	: m_bTopologFailed(FALSE)
	, m_cRef(1)
	, m_bPaused(FALSE)
	, m_flRate(1.f)
	, m_SARID(0)
	, m_fReceivedTime(FALSE)
	, m_hnsMediastartOffset(0)
{
	m_hOpenReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
	m_hRateReady = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
	m_hStopEvent = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
}

CMediaSessionPlayer::~CMediaSessionPlayer()
{
	if (m_hOpenReadyEvent)
	{
		CloseHandle(m_hOpenReadyEvent);
		m_hOpenReadyEvent = NULL;
	}

	if (m_hRateReady)
	{
		CloseHandle(m_hRateReady);
		m_hRateReady = NULL;
	}

	if (m_hStopEvent)
	{
		CloseHandle(m_hStopEvent);
		m_hStopEvent = NULL;
	}
}

HRESULT CMediaSessionPlayer::QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (ppvObject == NULL)
		return E_POINTER;

	if (riid == IID_IMFAsyncCallback)
	{
		*ppvObject = (IMFAsyncCallback*)this;
	}
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG CMediaSessionPlayer::AddRef(void)
{
	LONG lRef = InterlockedIncrement(&m_cRef);
	assert(lRef > 0);
	return (ULONG)m_cRef;
}

ULONG CMediaSessionPlayer::Release(void)
{
	/* If the reference count drops to zero delete ourselves */

	LONG lRef = InterlockedDecrement(&m_cRef);
	assert(lRef >= 0);

	if (lRef == 0)
	{
		m_cRef++;

		delete this;
		return ULONG(0);
	}
	else
	{
		return ULONG(lRef);
	}
}

HRESULT CMediaSessionPlayer::Open(HWND hWnd)
{
	HRESULT hr = S_OK;
	ComPtr<IMFSourceResolver> spResolver;
	ComPtr<IUnknown> spUnk;
	ComPtr<IMFPresentationDescriptor> spPD;
	ComPtr<IMFTopology> spTopo;
	ComPtr<IMFClock> spClock;

	DWORD cStream = 0;
	DWORD nCreatedStream = 0;
	MF_OBJECT_TYPE outType;
	PROPVARIANT var;

	UpdateSARPrefType();

	CHECK_HR(hr = MFCreateSourceResolver(&spResolver));

	CHECK_HR(hr = spResolver->CreateObjectFromURL(L"D:\\FILEZ058.mp4", MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE, nullptr, &outType, &spUnk));
	CHECK_HR(hr = spUnk.As(&m_spSource));

	CHECK_HR(hr = MFCreateTopology(&spTopo));

	// Create the presentation descriptor for the media source.
	CHECK_HR(hr = m_spSource->CreatePresentationDescriptor(&spPD));

	// Get the number of streams in the media source.
	CHECK_HR(hr = spPD->GetStreamDescriptorCount(&cStream));

	for (DWORD idxStream = 0; idxStream < cStream; idxStream++)
	{
		GUID guidMajorType = GUID_NULL;
		BOOL fSelected = FALSE;
		ComPtr<IMFStreamDescriptor> spSD;
		ComPtr<IMFMediaTypeHandler> spMediaTypeHandler;
		CHECK_HR(hr = spPD->GetStreamDescriptorByIndex(idxStream, &fSelected, &spSD));

		if (!fSelected)
			continue;

		if (FAILED(spSD->GetMediaTypeHandler(&spMediaTypeHandler)))
			continue;

		if (FAILED(spMediaTypeHandler->GetMajorType(&guidMajorType)))
			continue;

		if (guidMajorType != MFMediaType_Video && guidMajorType != MFMediaType_Audio)
			continue;

		ComPtr<IMFTopologyNode> spSourceNode;
		// Create the source-stream node.
		CHECK_HR(hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &spSourceNode));

		// Set attribute: Pointer to the media source.
		CHECK_HR(hr = spSourceNode->SetUnknown(MF_TOPONODE_SOURCE, spUnk.Get()));

		// Set attribute: Pointer to the presentation descriptor.
		CHECK_HR(hr = spSourceNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, spPD.Get()));

		// Set attribute: Pointer to the stream descriptor.
		CHECK_HR(hr = spSourceNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, spSD.Get()));
		CHECK_HR(hr = spTopo->AddNode(spSourceNode.Get()));

		// Create a downstream node.
		ComPtr<IMFTopologyNode> spSinkNode;
		CHECK_HR(hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &spSinkNode));

		ComPtr<IMFActivate> spRendererActivate;
		if (MFMediaType_Video == guidMajorType)
		{
			CHECK_HR(hr = MFCreateVideoRendererActivate(hWnd, &spRendererActivate));

			m_spEVRActivate = spRendererActivate;
			spSinkNode->SetUINT32(MF_TOPONODE_STREAMID, 0);

		}
		else if (MFMediaType_Audio == guidMajorType)
		{
			CHECK_HR(hr = MFCreateAudioRendererActivate(&spRendererActivate));
			CHECK_HR(hr = spRendererActivate->SetUINT32(MF_AUDIO_RENDERER_ATTRIBUTE_ENDPOINT_ROLE, eMultimedia));
			//CHECK_HR(hr = spRendererActivate->SetUINT32(MF_AUDIO_RENDERER_ATTRIBUTE_FILL_SILENCE_WHEN_STARVING, TRUE));

			CHECK_HR(hr = spSinkNode->GetTopoNodeID(&m_SARID));

			m_spSARActivate = spRendererActivate;
		}
		CHECK_HR(hr = spSinkNode->SetObject(spRendererActivate.Get()));

		CHECK_HR(hr = spTopo->AddNode(spSinkNode.Get()));

		CHECK_HR(spSourceNode->ConnectOutput(0, spSinkNode.Get(), 0));
	}

	WORD cNodes;
	CHECK_HR(hr = spTopo->GetNodeCount(&cNodes));
	for (WORD i = 0; i < cNodes; i++)
	{
		ComPtr<IMFTopologyNode> spNode;
		MF_TOPOLOGY_TYPE tidType;

		if (FAILED(spTopo->GetNode(i, &spNode)))
			continue;

		if (FAILED(spNode->GetNodeType(&tidType)))
			continue;

		// We need to find the source node so we can get the IMFMediaSource for this playback
		if (MF_TOPOLOGY_SOURCESTREAM_NODE == tidType)
		{
			if (m_spSource == nullptr)
			{
				if (FAILED(spNode->GetUnknown(MF_TOPONODE_SOURCE, IID_IMFMediaSource, (void**)&m_spSource)))
					continue;
			}

			MFTIME hnsMediastart = 0;
			(void)spNode->GetUINT64(MF_TOPONODE_MEDIASTART, (UINT64*)&hnsMediastart);
			if (hnsMediastart < m_hnsMediastartOffset)
			{
				m_hnsMediastartOffset = hnsMediastart;
			}
		}
	}

	DP(_T("[MFPlayer] Begin calling MFCreateMediaSession().\n"));
	CHECK_HR(MFCreateMediaSession(NULL, &m_spMediaSession));
	DP(_T("[MFPlayer] End calling MFCreateMediaSession.\n"));

	m_spMediaSession->BeginGetEvent((IMFAsyncCallback*)this, NULL);

	m_fReceivedTime = FALSE;
	m_hnsMediastartOffset = INT64_MAX;
	DP(_T("[MFPlayer] Begin calling IMFMediaSession::SetTopology().\n"));
	CHECK_HR(hr = m_spMediaSession->SetTopology(0, spTopo.Get()));
	DP(_T("[MFPlayer] End calling IMFMediaSession::SetTopology().\n"));

	WaitForSingleObjectEx(m_hOpenReadyEvent, INFINITE, FALSE);

	// Update the clock
	CHECK_HR(hr = m_spMediaSession->GetClock(&spClock));
	CHECK_HR(hr = spClock.As(&m_spSessionClock));

	PropVariantInit(&var);
	var.vt = VT_I8;
	var.hVal.QuadPart = 0LL;
	DP(_T("[MFPlayer] Begin calling IMFMediaSession::Start().\n"));
	CHECK_HR(m_spMediaSession->Start(NULL, &var));
	DP(_T("[MFPlayer] End calling IMFMediaSession::Start().\n"))

done:
	PropVariantClear(&var);
	return hr;
}

HRESULT CMediaSessionPlayer::Stop()
{
	HRESULT hr = E_FAIL;
	if (m_spMediaSession)
	{
		DP(_T("[MFPlayer] Begin stopping media session.\n"));
		hr = m_spMediaSession->Stop();
		DP(_T("[MFPlayer] Finish stopping media session {hr: 0X%X}.\n"), hr);
	}

	if (FAILED(hr))
	{
		DP(_T("[MFPlayer] Failed to Stop player.\n"));
	}

	return hr;
}

HRESULT CMediaSessionPlayer::Pause(BOOL bPauseOff)
{
	HRESULT hr = E_FAIL;
	if (m_spMediaSession)
	{
		DP(_T("[MFPlayer] Begin %s media session.\n"), bPauseOff ? _T("starting") : _T("pausing"));
		if (bPauseOff)
		{
			PROPVARIANT varStart;
			PropVariantInit(&varStart);
			varStart.vt = VT_EMPTY;
			hr = m_spMediaSession->Start(NULL, &varStart);
			PropVariantClear(&varStart);
		}
		else
		{
			hr = m_spMediaSession->Pause();
		}
		DP(_T("[MFPlayer] Finish %s media session {hr: 0X%X}.\n"), bPauseOff ? _T("starting") : _T("pausing"), hr);
	}

	if (FAILED(hr))
	{
		DP(_T("[MFPlayer] Failed to Stop player.\n"));
	}

	return hr;
}

HRESULT CMediaSessionPlayer::Play()
{
	HRESULT hr = E_FAIL;
	if (m_spMediaSession == nullptr)
		goto done;

	if (m_flRate < 0.01f || m_flRate > 1.99f)
	{
		ResetEvent(m_hStopEvent);

		// From rewind or fast-forward to normal playback.
		m_spMediaSession->Stop();

		WaitForSingleObjectEx(m_hStopEvent, INFINITE, FALSE);

		ComPtr<IMFRateControl> spRateControl;
		m_spMediaSession.As(&spRateControl);
		if (spRateControl == nullptr)
			goto done;

		DP(_T("[MFPlayer] Begin setting rate to %f.\n"), 1.0f);
		hr = spRateControl->SetRate(FALSE, 1.0f);
		DP(_T("[MFPlayer] Finish setting rate to %f {hr: 0X%X}.\n"), 1.0f, hr);


		PROPVARIANT var;
		PropVariantInit(&var);
		var.vt = VT_I8;
		var.hVal.QuadPart = 0LL;
		DP(_T("[MFPlayer] Begin calling IMFMediaSession::Start().\n"));
		CHECK_HR(m_spMediaSession->Start(NULL, &var));
		DP(_T("[MFPlayer] End calling IMFMediaSession::Start().\n"))

	}
	else
	{
		if (!IS_RATE(m_flRate, 1.0f))
		{
			ComPtr<IMFRateControl> spRateControl;
			m_spMediaSession.As(&spRateControl);
			if (spRateControl == nullptr)
				goto done;

			DP(_T("[MFPlayer] Begin setting rate to %f.\n"), 1.0f);
			hr = spRateControl->SetRate(FALSE, 1.0f);
			DP(_T("[MFPlayer] Finish setting rate to %f {hr: 0X%X}.\n"), 1.0f, hr);
		}

		ComPtr<IMFClock> spClock;
		if (SUCCEEDED(m_spMediaSession->GetClock(&spClock)))
		{
			LONGLONG llClockTime = 0;
			MFTIME hnsSystemTime = 0;
			spClock->GetCorrelatedTime(0, &llClockTime, &hnsSystemTime);


		}

		DP(_T("[MFPlayer] Begin starting media session from the current position.\n"));
		PROPVARIANT varStart;
		PropVariantInit(&varStart);
		varStart.vt = VT_EMPTY;
		hr = m_spMediaSession->Start(NULL, &varStart);
		PropVariantClear(&varStart);
		DP(_T("[MFPlayer] Finish starting media session {hr: 0X%X}.\n"), hr);
	}

done:
	if (FAILED(hr))
	{
		DP(_T("[MFPlayer] Failed to Stop player.\n"));
	}

	return hr;
}

HRESULT CMediaSessionPlayer::SetRate(float flRate)
{
	HRESULT hr = E_FAIL;
	if (m_spMediaSession)
	{
		if (!IS_RATE(m_flRate, flRate))
		{
			ComPtr<IMFRateControl> spRateControl;
			m_spMediaSession.As(&spRateControl);
			if (spRateControl == nullptr)
				goto done;

			ComPtr<IMFRateSupport> spRateSupport;
			spRateControl.As(&spRateSupport);

			BOOL fThin = FALSE;
			float flNearestSupportedRate = flRate;
			if (FAILED(hr = spRateSupport->IsRateSupported(FALSE, flRate, &flNearestSupportedRate)))
			{
				if (hr == MF_E_UNSUPPORTED_RATE)
				{
					DP(_T("[MFPlayer] Does not support rate: %f, nearest supported rate: %f under non-thinning mode {hr = 0X%X}.\n"), flRate, flNearestSupportedRate, hr);
				}
				else
				{
					DP(_T("[MFPlayer] Does not support rate: %f under non-thinning mode {hr = 0X%X}.\n"), flRate, hr);
				}

				if (FAILED(hr = spRateSupport->IsRateSupported(TRUE, flRate, &flNearestSupportedRate)))
				{
					if (hr == MF_E_UNSUPPORTED_RATE)
					{
						DP(_T("[MFPlayer] Does not support rate: %f, nearest supported rate: %f under thinning mode {hr = 0X%X}.\n"), flRate, flNearestSupportedRate, hr);
					}
					else
					{
						DP(_T("[MFPlayer] Does not support rate: %f under thinning mode {hr = 0X%X}.\n"), flRate, hr);
					}

					goto done;
				}

				fThin = TRUE;
			}

			DP(_T("[MFPlayer] Begin setting rate to %f.\n"), flRate);
			ResetEvent(m_hRateReady);
			hr = spRateControl->SetRate(fThin, flRate);
			if (SUCCEEDED(hr))
				WaitForSingleObject(m_hRateReady, INFINITE);
			DP(_T("[MFPlayer] Finish setting rate to %f {hr: 0X%X}.\n"), flRate, hr);
		}
	}

done:
	if (FAILED(hr))
	{
		DP(_T("[MFPlayer] Failed to Stop player.\n"));
	}

	return hr;
}

HRESULT CMediaSessionPlayer::FastForward()
{
	DP(_T("[MFPlayer] FastForward.\n"));
	float flRate = FindNextForwardRate(m_flRate);
	return SetRate(flRate);
}

HRESULT CMediaSessionPlayer::Rewind()
{
	DP(_T("[MFPlayer] Rewind.\n"));
	float flRate = FindNextRewindRate(m_flRate);
	return SetRate(flRate);
}

HRESULT CMediaSessionPlayer::GetTime(MFTIME* phnsTime)
{
	HRESULT hr = S_OK;

	CHECK_HR(hr = m_spSessionClock->GetTime(phnsTime));

	//
	// To get UI time, we need to apply the appropriate adjustment
	// to Presentation Clock time.
	//
	if (m_fReceivedTime)
	{
		*phnsTime -= m_hnsOffsetTime;
	}

	*phnsTime += m_hnsMediastartOffset;

done:
	return(hr);
}

HRESULT CMediaSessionPlayer::GetParameters(DWORD *pdwFlags, DWORD *pdwQueue)
{
	return E_NOTIMPL;
}

HRESULT CMediaSessionPlayer::Invoke(IMFAsyncResult *pAsyncResult)
{
	HRESULT hr = S_OK;
	HRESULT hrStatus = S_OK;
	MediaEventType meType = MEUnknown;
	ComPtr<IMFMediaEvent> spEvent = NULL;

	// Get the event from the event queue.
	if (m_spMediaSession == nullptr)
		return S_OK;

	CHECK_HR(hr = m_spMediaSession->EndGetEvent(pAsyncResult, &spEvent));

	// Get the event type.
	CHECK_HR(hr = spEvent->GetType(&meType));
	hr = spEvent->GetStatus(&hrStatus);
	DP(_T("[MFPlayer] Receiving the event %s (%d) with status (0X%X).\n"), MediaEventType_Name(meType), meType, hrStatus);

	if (meType != MESessionClosed)
	{
		CHECK_HR(hr = m_spMediaSession->BeginGetEvent(this, NULL));
	}

	switch (meType)
	{
	case MEUpdatedStream:
		break;
	case MENewStream:
		break;
	case MESessionTopologyStatus:
	{
		if (FAILED(hrStatus))
		{
			if (MF_E_POLICY_UNSUPPORTED == hrStatus || ERROR_GRAPHICS_OPM_NOT_SUPPORTED == hrStatus || ERROR_GRAPHICS_OPM_OUTPUT_DOES_NOT_SUPPORT_HDCP == hrStatus)
			{
				DP(_T("[MFPlayer] Output protection error happened when setting Topology {hrStatus: 0X%X}.\n"), hrStatus);
			}
			m_bTopologFailed = TRUE;
		}
		else
		{
			MF_TOPOSTATUS TopoStatus;
			// Get the status code.
			CHECK_HR(hr = spEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus));

			DP(_T("[MFPlayer] Media session topology status. Topology status = %s(%d) \n"), TopoStatus_Name(TopoStatus), TopoStatus);

			if (MF_TOPOSTATUS_READY == TopoStatus)
			{
				SetEvent(m_hOpenReadyEvent);
			}
		}
	}
	break;
	case MESessionStarted:
	{
		if (!IS_RATE_ZERO(m_flRate))	// if m_flRate is still zero, although receiving MESessionStarted, it means one frame is stepped.
			m_bPaused = FALSE;

		MFTIME hnsTopologyPresentationOffset;

		if (SUCCEEDED(spEvent->GetUINT64(MF_EVENT_PRESENTATION_TIME_OFFSET, (UINT64*)&hnsTopologyPresentationOffset)))
		{
			m_hnsOffsetTime = hnsTopologyPresentationOffset;
		}
	}
	break;
	case MEStreamSinkPrerolled:
		break;
	case MESessionPaused:
		m_bPaused = TRUE;
		break;
	case MESessionClosed:
		break;
	case MESessionStopped:
		SetEvent(m_hStopEvent);
		break;
	case MESessionRateChanged:
	{
		PROPVARIANT varRate;
		PropVariantInit(&varRate);
		if (SUCCEEDED(spEvent->GetValue(&varRate)) && varRate.vt == VT_R4)
		{
			m_flRate = varRate.fltVal;
			DP(_T("[MFPlayer] Successfully change the rate to %f.\n"), varRate.fltVal);
		}
		else
		{
			DP(_T("[MFPlayer] Failed to get the new playback rate. {hr: 0X%X}.\n"), hr);
		}
		PropVariantClear(&varRate);
		SetEvent(m_hRateReady);
	}
	break;
	case MEEndOfPresentation:
		break;
	case MESessionEnded:
		break;
	case MESessionNotifyPresentationTime:
	{
		if (FAILED(spEvent->GetUINT64(MF_EVENT_START_PRESENTATION_TIME, (UINT64*)&m_hnsStartTime)))
			break;
		if (FAILED(spEvent->GetUINT64(MF_EVENT_PRESENTATION_TIME_OFFSET, (UINT64*)&m_hnsOffsetTime)))
			break;
		m_fReceivedTime = TRUE;
	}
	break;
	case MERendererEvent:
		break;
	case MEPolicyError:
		break;
	case MEPolicyReport:
		break;
	case MEError:
		break;
	case MEExtendedType:
		break;
	case MESessionStreamSinkFormatChanged:
		break;
	case MESessionScrubSampleComplete:
		break;
	case MESourceRateChanged:
		break;
	case MEStreamSinkFormatChanged:
		break;
	case MESessionCapabilitiesChanged:
	{
		UINT32 dwNewSessionCapabilities = 0, dwSessionCapsDelta = 0;
		spEvent->GetUINT32(MF_EVENT_SESSIONCAPS, &dwNewSessionCapabilities);
		spEvent->GetUINT32(MF_EVENT_SESSIONCAPS_DELTA, &dwSessionCapsDelta);

		DP(_T("[MFPlayer] Receiving the event MESessionCapabilitiesChanged, new session caps: %s(0X%X), delta: %s(0X%X).\r\n"),
			GetBitsString<UINT>(session_caps, _countof(session_caps), dwNewSessionCapabilities).c_str(), dwNewSessionCapabilities,
			GetBitsString<UINT>(session_caps, _countof(session_caps), dwSessionCapsDelta).c_str(), dwSessionCapsDelta);
	}
	break;
	case MEStreamSinkFormatInvalidated:
	{
#if 0
		HRESULT hr1 = E_UNEXPECTED;
		ComPtr<IUnknown> spUnk;
		ComPtr<IMFTopologyNode> spNode;
		ComPtr<IMFTopology> spFullTopo;
		ComPtr<IMFStreamSink> spStreamSink;
		ComPtr<IMFMediaTypeHandler> spHandler;

		if (SUCCEEDED(hr1 = m_spMediaSession->GetFullTopology(MFSESSION_GETFULLTOPOLOGY_CURRENT, 0, &spFullTopo)))
		{
			if (SUCCEEDED(hr1 = spFullTopo->GetNodeByID(m_SARID, &spNode)))
			{
				if (SUCCEEDED(hr1 = spNode->GetObject(&spUnk)))
				{
					if (SUCCEEDED(hr1 = spUnk.As(&spStreamSink)))
					{
						if (SUCCEEDED(hr1 = spStreamSink->GetMediaTypeHandler(&spHandler)))
						{
							hr1 = spHandler->SetCurrentMediaType(m_SARPrefType.Get());
						}
					}
				}
			}
		}
#endif
	}
	break;
	}

done:
	return S_OK;
}

