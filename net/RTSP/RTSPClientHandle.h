#pragma once

#include <memory>
#include "TcpClientHandle.h"
#include "RTSPMessage.h"
//#include "RTSPServer.h"
#include "DigestAuthentication.h"
//#include "RTPClientHandle.h"
#include "RtpConnection.h"
//客户端分两种，一种是推流客户端、一种是拉流客户端

class RTSPServer;
class RTSPSession;

enum RTSP_METHOD {
	OPTIONS = 1,
	DESCRIBE,
	ANNOUNCE,
	SETUP,
	PLAY,
	PAUSE,
	TEARDOWN,
	GET_PARAMETER,
	SET_PARAMETER,
	REDIRECT,
	RECORD,
	Embedded
};

class RTSPClientHandle : public TcpClientHandle {
public:
	enum ConnectionState
	{
		START_CONNECT,
		START_PLAY,
		START_PUSH
	};
	RTSPClientHandle();
	RTSPClientHandle(std::shared_ptr<RTSPServer> server);
	~RTSPClientHandle() {};

	virtual void readHandle() override;
	virtual void closeHandle() override;

	void setServer();
	void setSession();

	bool HandleRtspRequest();

	void HandleCmdOption();
	void HandleCmdDescribe();
	void HandleCmdSetup();
	void HandleCmdPlay();
	void HandleCmdTeardown();
	void HandleCmdGetParamter();
	bool HandleAuthentication();

	void SendRtspMessage(char* data, int size);
private:
	bool m_hasAuth = true;
	std::string m_nonce;
	std::unique_ptr<DigestAuthentication> m_authInfo;

	MediaSessionId  m_sessionId = 0;

	std::shared_ptr<RtpConnection> m_rtpClient;
	std::shared_ptr<RtpConnection> m_rtpClients;
	std::shared_ptr<RtspRequest> m_rtspReq;
	std::shared_ptr<RtspResponse> m_rtspRes;

	std::shared_ptr<RTSPSession> m_Session;
	std::shared_ptr<RTSPServer> m_server;

	ConnectionState conn_state_ = START_CONNECT;
	MediaSessionId  session_id_ = 0;
};

////推流客户端
//class PushRTSPClientHandle : public enable_shared_from_this<PushRTSPClientHandle> {};
//
////拉流客户端
//class PULLRTSPClientHandle : public enable_shared_from_this<PULLRTSPClientHandle> {};