// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mmdeviceapi.h>
#include <mferror.h>
#include <stdio.h>
#include <assert.h>
#include <Evntprov.h>
#include <evntrace.h>
#include <iostream>
#include <sstream>
#include <tuple>
#include <uuids.h>

#define DECL_TUPLE(x)	{x, _T(#x)}

#define CHECK_HR(hr)	if(FAILED((hr)))goto done

#define MediaEventType_Name(x)	(\
	(x) == MEUnknown? _T("MEUnknown"):(\
	(x) == MEError?_T("MEError"):(\
	(x) == MEExtendedType?_T("MEExtendedType"):(\
	(x) == MENonFatalError?_T("MENonFatalError"):(\
	(x) == MESessionUnknown?_T("MESessionUnknown"):(\
	(x) == MESessionTopologySet?_T("MESessionTopologySet"):(\
	(x) == MESessionTopologiesCleared?_T("MESessionTopologiesCleared"):(\
	(x) == MESessionStarted?_T("MESessionStarted"):(\
	(x) == MESessionPaused?_T("MESessionPaused"):(\
	(x) == MESessionStopped?_T("MESessionStopped"):(\
	(x) == MESessionClosed?_T("MESessionClosed"):(\
	(x) == MESessionEnded?_T("MESessionEnded"):(\
	(x) == MESessionRateChanged?_T("MESessionRateChanged"):(\
	(x) == MESessionScrubSampleComplete?_T("MESessionScrubSampleComplete"):(\
	(x) == MESessionCapabilitiesChanged?_T("MESessionCapabilitiesChanged"):(\
	(x) == MESessionTopologyStatus?_T("MESessionTopologyStatus"):(\
	(x) == MESessionNotifyPresentationTime?_T("MESessionNotifyPresentationTime"):(\
	(x) == MENewPresentation?_T("MENewPresentation"):(\
	(x) == MELicenseAcquisitionStart?_T("MELicenseAcquisitionStart"):(\
	(x) == MELicenseAcquisitionCompleted?_T("MELicenseAcquisitionCompleted"):(\
	(x) == MEIndividualizationStart?_T("MEIndividualizationStart"):(\
	(x) == MEIndividualizationCompleted?_T("MEIndividualizationCompleted"):(\
	(x) == MEEnablerProgress?_T("MEEnablerProgress"):(\
	(x) == MEEnablerCompleted?_T("MEEnablerCompleted"):(\
	(x) == MEPolicyError?_T("MEPolicyError"):(\
	(x) == MEPolicyReport?_T("MEPolicyReport"):(\
	(x) == MEBufferingStarted?_T("MEBufferingStarted"):(\
	(x) == MEBufferingStopped?_T("MEBufferingStopped"):(\
	(x) == MEConnectStart?_T("MEConnectStart"):(\
	(x) == MEConnectEnd?_T("MEConnectEnd"):(\
	(x) == MEReconnectStart?_T("MEReconnectStart"):(\
	(x) == MEReconnectEnd?_T("MEReconnectEnd"):(\
	(x) == MERendererEvent?_T("MERendererEvent"):(\
	(x) == MESessionStreamSinkFormatChanged?_T("MESessionStreamSinkFormatChanged"):(\
	(x) == MESourceUnknown?_T("MESourceUnknown"):(\
	(x) == MESourceStarted?_T("MESourceStarted"):(\
	(x) == MEStreamStarted?_T("MEStreamStarted"):(\
	(x) == MESourceSeeked?_T("MESourceSeeked"):(\
	(x) == MEStreamSeeked?_T("MEStreamSeeked"):(\
	(x) == MENewStream?_T("MENewStream"):(\
	(x) == MEUpdatedStream?_T("MEUpdatedStream"):(\
	(x) == MESourceStopped?_T("MESourceStopped"):(\
	(x) == MEStreamStopped?_T("MEStreamStopped"):(\
	(x) == MESourcePaused?_T("MESourcePaused"):(\
	(x) == MEStreamPaused?_T("MEStreamPaused"):(\
	(x) == MEEndOfPresentation?_T("MEEndOfPresentation"):(\
	(x) == MEEndOfStream?_T("MEEndOfStream"):(\
	(x) == MEMediaSample?_T("MEMediaSample"):(\
	(x) == MEStreamTick?_T("MEStreamTick"):(\
	(x) == MEStreamThinMode?_T("MEStreamThinMode"):(\
	(x) == MEStreamFormatChanged?_T("MEStreamFormatChanged"):(\
	(x) == MESourceRateChanged?_T("MESourceRateChanged"):(\
	(x) == MEEndOfPresentationSegment?_T("MEEndOfPresentationSegment"):(\
	(x) == MESourceCharacteristicsChanged?_T("MESourceCharacteristicsChanged"):(\
	(x) == MESourceRateChangeRequested?_T("MESourceRateChangeRequested"):(\
	(x) == MESourceMetadataChanged?_T("MESourceMetadataChanged"):(\
	(x) == MESequencerSourceTopologyUpdated?_T("MESequencerSourceTopologyUpdated"):(\
	(x) == MESinkUnknown?_T("MESinkUnknown"):(\
	(x) == MEStreamSinkStarted?_T("MEStreamSinkStarted"):(\
	(x) == MEStreamSinkStopped?_T("MEStreamSinkStopped"):(\
	(x) == MEStreamSinkPaused?_T("MEStreamSinkPaused"):(\
	(x) == MEStreamSinkRateChanged?_T("MEStreamSinkRateChanged"):(\
	(x) == MEStreamSinkRequestSample?_T("MEStreamSinkRequestSample"):(\
	(x) == MEStreamSinkMarker?_T("MEStreamSinkMarker"):(\
	(x) == MEStreamSinkPrerolled?_T("MEStreamSinkPrerolled"):(\
	(x) == MEStreamSinkScrubSampleComplete?_T("MEStreamSinkScrubSampleComplete"):(\
	(x) == MEStreamSinkFormatChanged?_T("MEStreamSinkFormatChanged"):(\
	(x) == MEStreamSinkDeviceChanged?_T("MEStreamSinkDeviceChanged"):(\
	(x) == MEQualityNotify?_T("MEQualityNotify"):(\
	(x) == MESinkInvalidated?_T("MESinkInvalidated"):(\
	(x) == MEAudioSessionNameChanged?_T("MEAudioSessionNameChanged"):(\
	(x) == MEAudioSessionVolumeChanged?_T("MEAudioSessionVolumeChanged"):(\
	(x) == MEAudioSessionDeviceRemoved?_T("MEAudioSessionDeviceRemoved"):(\
	(x) == MEAudioSessionServerShutdown?_T("MEAudioSessionServerShutdown"):(\
	(x) == MEAudioSessionGroupingParamChanged?_T("MEAudioSessionGroupingParamChanged"):(\
	(x) == MEAudioSessionIconChanged?_T("MEAudioSessionIconChanged"):(\
	(x) == MEAudioSessionFormatChanged?_T("MEAudioSessionFormatChanged"):(\
	(x) == MEAudioSessionDisconnected?_T("MEAudioSessionDisconnected"):(\
	(x) == MEAudioSessionExclusiveModeOverride?_T("MEAudioSessionExclusiveModeOverride"):(\
	(x) == MECaptureAudioSessionVolumeChanged?_T("MECaptureAudioSessionVolumeChanged"):(\
	(x) == MECaptureAudioSessionDeviceRemoved?_T("MECaptureAudioSessionDeviceRemoved"):(\
	(x) == MECaptureAudioSessionFormatChanged?_T("MECaptureAudioSessionFormatChanged"):(\
	(x) == MECaptureAudioSessionDisconnected?_T("MECaptureAudioSessionDisconnected"):(\
	(x) == MECaptureAudioSessionExclusiveModeOverride?_T("MECaptureAudioSessionExclusiveModeOverride"):(\
	(x) == MECaptureAudioSessionServerShutdown?_T("MECaptureAudioSessionServerShutdown"):(\
	(x) == METrustUnknown?_T("METrustUnknown"):(\
	(x) == MEPolicyChanged?_T("MEPolicyChanged"):(\
	(x) == MEContentProtectionMessage?_T("MEContentProtectionMessage"):(\
	(x) == MEPolicySet?_T("MEPolicySet"):(\
	(x) == MEWMDRMLicenseBackupCompleted?_T("MEWMDRMLicenseBackupCompleted"):(\
	(x) == MEWMDRMLicenseBackupProgress?_T("MEWMDRMLicenseBackupProgress"):(\
	(x) == MEWMDRMLicenseRestoreCompleted?_T("MEWMDRMLicenseRestoreCompleted"):(\
	(x) == MEWMDRMLicenseRestoreProgress?_T("MEWMDRMLicenseRestoreProgress"):(\
	(x) == MEWMDRMLicenseAcquisitionCompleted?_T("MEWMDRMLicenseAcquisitionCompleted"):(\
	(x) == MEWMDRMIndividualizationCompleted?_T("MEWMDRMIndividualizationCompleted"):(\
	(x) == MEWMDRMIndividualizationProgress?_T("MEWMDRMIndividualizationProgress"):(\
	(x) == MEWMDRMProximityCompleted?_T("MEWMDRMProximityCompleted"):(\
	(x) == MEWMDRMLicenseStoreCleaned?_T("MEWMDRMLicenseStoreCleaned"):(\
	(x) == MEWMDRMRevocationDownloadCompleted?_T("MEWMDRMRevocationDownloadCompleted"):(\
	(x) == METransformUnknown?_T("METransformUnknown"):(\
	(x) == METransformNeedInput?_T("METransformNeedInput"):(\
	(x) == METransformHaveOutput? _T("METransformHaveOutput") : (\
	(x) == METransformDrainComplete? _T("METransformDrainComplete") : (\
	(x) == METransformMarker? _T("METransformMarker") : (\
	(x) == MEByteStreamCharacteristicsChanged?_T("MEByteStreamCharacteristicsChanged"):(\
	(x) == MEVideoCaptureDeviceRemoved?_T("MEVideoCaptureDeviceRemoved"):(\
	(x) == MEVideoCaptureDevicePreempted?_T("MEVideoCaptureDevicePreempted"):(\
	(x) == MEStreamSinkFormatInvalidated?_T("MEStreamSinkFormatInvalidated"):(\
	(x) == MEEncodingParameters?_T("MEEncodingParameters"):(\
	(x) == MEContentProtectionMetadata?_T("MEContentProtectionMetadata"):_T("Reserved")))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))

#define TopoStatus_Name(x)	(\
	(x) == MF_TOPOSTATUS_INVALID?_T("MF_TOPOSTATUS_INVALID"):(\
	(x) == MF_TOPOSTATUS_READY?_T("MF_TOPOSTATUS_READY"):(\
	(x) == MF_TOPOSTATUS_STARTED_SOURCE?_T("MF_TOPOSTATUS_STARTED_SOURCE"):(\
	(x) == MF_TOPOSTATUS_DYNAMIC_CHANGED?_T("MF_TOPOSTATUS_DYNAMIC_CHANGED"):(\
	(x) == MF_TOPOSTATUS_SINK_SWITCHED?_T("MF_TOPOSTATUS_SINK_SWITCHED"):(\
	(x) == MF_TOPOSTATUS_ENDED?_T("MF_TOPOSTATUS_ENDED"):_T("Unknown Status")))))))

#define IS_RATE_ZERO(rate)		((rate) > -0.01f && (rate) < 0.01f)
#define IS_UNMUTE_RATE(rate)	((rate) >  0.99f && (rate) < 1.01f)
#define IS_RATE(rate1, rate2)	((rate1 - rate2 > -0.01f) && (rate1 -rate2 < 0.01f))

template<typename T>
#ifdef _UNICODE
inline std::wstring GetBitsString(std::tuple<T, const TCHAR*>* bit_names, size_t bits_count, T bits_value)
#else
inline std::string GetBitsString(std::tuple<T, const TCHAR*>* bit_names, size_t bits_count, T bits_value)
#endif
{
	bool bFirst = true;
#ifdef _UNICODE
	std::wostringstream oss;
#else
	std::ostringstream oss;
#endif
	for (int idxBit = 0; idxBit < bits_count; idxBit++)
	{
		if (std::get<0>(bit_names[idxBit])&bits_value)
		{
			oss << (bFirst ? _T("") : _T(", ")) << std::get<1>(bit_names[idxBit]);
			if (bFirst)
				bFirst = false;
		}
	}
	oss << std::ends;
	return oss.str();
}

#define DP0(x, ...)		{\
	TCHAR szLogOutput[1024];\
	_stprintf_s(szLogOutput, 1024, x, ##__VA_ARGS__);\
	_tprintf(szLogOuptut);\
	OutputDebugString(szLogOutput);\
}

#define DPA0(x, ...)		{\
	char szLogOutput[1024];\
	sprintf_s(szLogOutput, 1024, x, ##__VA_ARGS__);\
	printf(szLogOutput);\
	OutputDebugStringA(szLogOutput);\
}

#define DP(x, ...) {\
	TCHAR szLogOutput[1024];\
	_stprintf_s(szLogOutput, 1024, x, ##__VA_ARGS__);\
	_tprintf(szLogOutput);\
	OutputDebugString(szLogOutput);\
	EventWriteString(g_ETWHandle, 0, 0, szLogOutput);\
}

#define DPA(x, ...) {\
	char szLogOutput[1024];\
	stprintf_s(szLogOutput, 1024, x, ##__VA_ARGS__);\
	printf(szLogOutput);\
	OutputDebugStringA(szLogOutput);\
	EventWriteStringA(g_ETWHandle, 0, 0, szLogOutput);\
}


// TODO: reference additional headers your program requires here
