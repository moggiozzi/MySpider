#ifndef BT_SPP_H_
#define BT_SPP_H_

#define NO_BLUETOOTH 1

#ifdef NO_BLUETOOTH

class CBtSpp {
public:
	CBtSpp(){}
	virtual ~CBtSpp(){}

	void RegisterService(void){}
	void UnregisterService(void){}
	void RfcommOpen(void){}
	void RfcommClose(void){}
	bool NoneBlockingRead(char *pBuffer, int nBufferSize, int *prx_len){return false;}
};

#else

//#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
//#include <bluetooth/rfcomm.h>

class CBtSpp {
protected:

	sdp_session_t* m_sdp_session;
	int m_socket;
	int m_client;


public:
	CBtSpp();
	virtual ~CBtSpp();
	
	void RegisterService(void);
	void UnregisterService(void);
	void RfcommOpen(void);
	void RfcommClose(void);
	bool NoneBlockingRead(char *pBuffer, int nBufferSize, int *prx_len){return false}


};

#endif
#endif /* BT_SPP_H_ */
