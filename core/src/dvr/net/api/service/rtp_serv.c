/*
 * http_serv.c
 */
#include "net_serv.h"
#include "amn_event.h"
#include "amn_module.h"
#include "amn_osal.h"
#include "netapp.h"
#include "uartShell.h"
#include "mmpf_streaming.h"
#include "AHC_Common.h"
	#include "AHC_DebugEx.h"
#define DBGL_DBGS DBGL_DEBUG
#include "includes.h"
#include "rtp.h"

#define BUF_LEN (1024)
static osal_thread_t handle_request(
	/*! [in] Request Message to be handled. */
	void *args)
{
	int ret_code;
	http_message_t *hmsg = NULL;
	char buf[BUF_LEN];
	int timeout;
	struct resp_message_t *resp = (struct resp_message_t *)args;
	SOCKET connfd = resp->connfd;
	int num_read;

	osal_dprint(DBGS, "mini_dserver fd=%d: INIT", connfd );
	/* parser_request_init( &parser ); */ /* LEAK_FIX_MK */
	hmsg = &resp->parser.msg;
	ret_code = sock_init_with_ip( &resp->info, connfd, (struct sockaddr *)&resp->foreign_sockaddr);
	if (ret_code != UPNP_E_SUCCESS) {
		osal_dprint(ERR, "err:%d", ret_code);
		httpmsg_destroy(hmsg);
		osal_free(resp);
		return osal_thread_RETURN;
	}
	ADD_SOCKINFO_REF(&resp->info);
	do {
		/* read */
		//timeout = HTTP_DEFAULT_TIMEOUT;
		//"timeout = 0" means to use SOCK_READ_WRITE_DEFAULT_TIMEOUT as timeout
		int pts;
		unsigned int now;
		timeout = 0;
		osal_dprint(VERBOSE, "sock:%d ref:%d port:%d", resp->info.socket, resp->info.ref, resp->info.foreign_sockaddr.ss_port);
		num_read = sock_read(&(resp->info), buf, BUF_LEN, &timeout);
		if (num_read > 0 && num_read < 7) {
			sscanfl(buf, "%d", &pts);
			now = MMPF_StreamingTimer_GetMs();
			//osal_dprint(DBGS, "tsk:%d read %d bytes:%s in time:%d (%d)", AHC_DBG_GetCurPrioEx(), num_read, buf, now, (now - pts));
			osal_dprint(INFO, "Get pts:%s at %d, diff:%d", buf, now, (now - pts));
		} else {
			osal_dprint(INFO, "tsk:%d read %d bytes:%s in time:%d", AHC_DBG_GetCurPrioEx(), num_read, buf, now);
		}
		break;
#if 0
		ret_code = http_RecvMessage( &resp->info, &resp->parser, HTTPMETHOD_UNKNOWN, &timeout, &http_error_code);
		if (ret_code != 0) {
			if (ret_code == UPNP_E_TIMEDOUT) {
				goto L_continue_http_loop;
			}
			if ((DBGL_DBGS >= DBGL_DEBUG) || //if customer log filter was set, then only print out error case
			    (resp->persistant == 0 && ret_code != UPNP_E_SOCKET_ERROR && http_error_code != HTTP_BAD_REQUEST)) {
				osal_dprint(INFO, "mini_dserver fd=%d: socket may be closed, err(%d)", connfd, ret_code);
			}
			goto error_handler;
		}

		osal_dprint(DBGS, "mini_dserver fd=%d: PROCESS...", connfd);
		/* dispatch */
		http_error_code = dispatch_request(&resp->parser, resp);
		if (http_error_code != 0) {
			osal_dprint(INFO, "mini_dserver fd=%d: http_err=%d", connfd, http_error_code);
			goto error_handler;
		}
		/* auto release sock for POST method */
		if (resp->req->method == HTTPMETHOD_POST)
			goto error_handler;

L_continue_http_loop:
		http_error_code = 0;
		httpmsg_destroy(hmsg);

#endif
		if(nhw_get_status() == NETAPP_NET_STATUS_DOWN)
			break;
	} while (resp->persistant);
	osal_dprint(DBGS, "mini_dserver fd=%d: END...", connfd);
	if (REL_SOCKINFO_REF(&resp->info) == 0) {
		// printc("FREE %d\r\n", resp->info.socket);
		sock_destroy(&resp->info, SD_BOTH);
		osal_free(resp);
	}
	return osal_thread_RETURN;
}

#define FREAD_BUF_LEN (1540)
#define RTP_MAX_PKT_LEN FREAD_BUF_LEN

//RTP receiver end point. refer struct rtp_endpoint
struct rtp_rep {
	int payload;
	int max_data_size;
	unsigned int ssrc;
	unsigned int start_timestamp;
	unsigned int last_timestamp;
	int seqnum;
	int packet_count;
	int octet_count;
	int len;
	char marker;
} rep;

struct rtp_frame {
	unsigned int pts;
	unsigned int size;
	unsigned int seq;
	unsigned char *d;
};

struct rtp_pkt {
	int seqnum;//actually used as UINT16 other word are used as information
		#define UNUSED_SEQ (-1)
	char marker;
	int len;
	unsigned char data[RTP_MAX_PKT_LEN];
};

#define RTP_NBUF_N_PKT (100)
struct rtp_nbuf {
	int idx;
	int cur_seq;
	struct rtp_frame cur_frm;
	struct rtp_pkt pkt[RTP_NBUF_N_PKT];
} rtp_nbuf;

struct rtp_log {
	char pkt;
	char malloc;
};
struct rtp_log log = {0};

//#include "protocol.h"
#define GET_16(p)   (((p)[0]<<8)|(p)[1])
#define GET_32(p)   (((p)[0]<<24)|((p)[1]<<16)|((p)[2]<<8)|(p)[3])

int rtp_parse_rtp(struct rtp_rep *ep, char* buf, int len)
{
	char *rtphdr;
	//struct rtp_rep *ep = &rep;
	//char payload;
	//char marker = 0;
	rtphdr = buf;
	if (rtphdr[0] != 2 << 6 /*version*/) {
		printc("Invalid header %02X (packet len:%d)\r\n", rtphdr[0], len);
		return -1;
	}
	ep->payload = rtphdr[1] & 0x7F;
	if (ep->payload < 60 /*payload_type*/) {
		printc("Invalid payload %d for H264 (packet len:%d)\r\n", ep->payload, len);
		return -1;
	}
	if (rtphdr[1] & 0x80) {
		ep->marker = 1;
	} else ep->marker = 0;
	ep->seqnum = GET_16( rtphdr + 2);
	ep->last_timestamp = GET_32( rtphdr + 4);
	ep->ssrc = GET_32( rtphdr + 8);
	ep->len = len;

	if (log.pkt)
		printc("SEQ:%d, PTS:%d, len:%d marker:%d\r\n", ep->seqnum, ep->last_timestamp, len, ep->marker);

	return 0;
}

int parse_h264_nalu_type(u8_t* buf);

int rtp_append_h264(struct rtp_pkt *pkt, unsigned char *outbuf, int *cur_len, int *max_len)
{
	unsigned char *p;
	int nalu_type;
	unsigned int len_to_copy;
	char nri;
	char s_bit, e_bit;
	if (pkt == NULL || outbuf == NULL || max_len == NULL) {
		printd(BG_RED("NULL pointer")"%s\r\n", __MODULE__);
	}
	p = pkt->data + 12;
	nalu_type = parse_h264_nalu_type(p);
	nri = p[0] & 0x60;
	s_bit = p[1] & 0x80;
	e_bit = p[1] & 0x40;
	switch (nalu_type) {
	case 5/*NALU_PARSE__FUA*/:
		nalu_type = parse_h264_nalu_type(p + 1);
		if (nalu_type != 3 && nalu_type != 4) {
			printd(BG_RED("invalid FU header")"\r\n");
		}
		/*
		if ((*cur_len == 0 && s_bit != 0) ||
			(*cur_len != 0 && s_bit == 0)) {
			printd(BG_RED("Invalid s bit %02X")", cur_len:%d\r\n", *cur_len, s_bit);
		}*/
		if (s_bit) {
			outbuf[*cur_len + 0] = 0;
			outbuf[*cur_len + 1] = 0;
			outbuf[*cur_len + 2] = 0;
			outbuf[*cur_len + 3] = 1;
			outbuf[*cur_len + 4] = nri | (p[1] & 0x1F);
//printc("cur_len:%d + %d\r\n", *cur_len, 5);
			*cur_len += 5;
		}
		p+=2;
		len_to_copy = pkt->len - 2 - 12;
		if (len_to_copy + *cur_len >= *max_len) {
			printd(BG_RED("not enough frame buf size")"\r\n");
			return -1;
		}
		MEMCPY(outbuf + *cur_len, p, len_to_copy);
//	MemoryDump(outbuf + *cur_len, 48);
//printc("cur_len:%d + %d\r\n", *cur_len, len_to_copy);
		*cur_len += len_to_copy;
		break;
	case 3: /*NALU_PARSE__I_FRAMES*/
	case 4: /*NALU_PARSE__P_FRAMES*/
	case 21:/*NALU_PARSE__SPS_FRAMES*/
	case 22:/*NALU_PARSE__PPS_FRAMES*/
			outbuf[*cur_len + 0] = 0;
			outbuf[*cur_len + 1] = 0;
			outbuf[*cur_len + 2] = 0;
			outbuf[*cur_len + 3] = 1;
//printc("cur_len:%d + %d\r\n", *cur_len, 4);
			*cur_len += 4;
		len_to_copy = pkt->len - 12;
		if (len_to_copy + *cur_len >= *max_len) {
			printd(BG_RED("not enough frame buf size")"\r\n");
			return -1;
		}
		MEMCPY(outbuf + *cur_len, p, len_to_copy);
//	MemoryDump(outbuf + *cur_len, 48);
//printc("cur_len:%d + %d\r\n", *cur_len, len_to_copy);
		*cur_len += len_to_copy;
		break;
	default:
		break;
	}
if (e_bit) {
//	MemoryDump(outbuf, 48);
}
	return 0;
}

struct rtp_serv {
	int (*use_frame)(struct rtp_frame *frame);
};


//compare frame data with prestored SD card
int rtp_cmp_frame_sd(struct rtp_frame *rtp_frm)
{
	printc(FG_GREEN("[frm %d]") " 264:%d pts:%d\r\n", rtp_frm->seq, rtp_frm->size, rtp_frm->pts);
	{
		char fn[64];
		OSAL_FILE *fd;
		snprintf(fn, 64, "SD:\\RTP\\264.%04d", rtp_frm->seq);
		fd = osal_fopen(fn, "rb");
		if (fd != NULL) {
			int f_len;
			unsigned char *buf;

			f_len = osal_getfilesize(fd);
			printc("file size to read %d\r\n", f_len);

			buf = osal_malloc(f_len);
printc("malloc:%d to %p, line %d\r\n", f_len, buf, __LINE__);
			if (buf == NULL) {
				printd(BG_RED("fopen error ")__MODULE__"\r\n");
				return -1;
			}
			osal_fseek(fd, 0, SEEK_SET);
			osal_flush_dcache();
			osal_fread(buf, 1, f_len, fd);
			osal_invalidate_dcache(buf, f_len);
			if (memcmp(buf, rtp_frm->d, f_len) == 0) {
				printd(BG_GREEN("OK")": frm %d\r\n", rtp_frm->seq);
			} else {
				int i;
				printd(BG_RED("NG")": frm %d\r\n", rtp_frm->seq);
				printc("ans:\r\n");
				MemoryDump((char*)buf, 48);
				printc("rebuilt:\r\n");
				MemoryDump((char*)rtp_frm->d, 48);
				for (i = 0; i < f_len; ++i) {
					if (buf[i] != rtp_frm->d[i]) {
						break;
					}
				}
				if (i != f_len) {
					printc("not the same at position %d 0x%X\r\n", i, i);
					MemoryDump((char*)(buf + i - 16), 32);
					printc("rebuilt:\r\n");
					MemoryDump((char*)(rtp_frm->d + i - 16), 32);
				}
			}
			osal_fclose(fd);
			osal_free(buf);
		}
	}
	return 0;
}

int rtp_use_frame(struct rtp_frame *rtp_frm)
{
	printc(FG_GREEN("[frm %d]") " 264:%d pts:%d\r\n", rtp_frm->seq, rtp_frm->size, rtp_frm->pts);
	return 0;
}

int rtp_enq_packet(struct rtp_serv* serv, char* buf, int len)
{
	int ret = 0;
	struct rtp_nbuf *nbuf = &rtp_nbuf;
	struct rtp_rep *info = &rep;
	int offset;
	int i;
	//printc("len:%d\r\n", len);
	ret = rtp_parse_rtp(info, buf, len);
	if (ret == 0 /*OK*/) {
		//first packet must be got first.
		if (nbuf->cur_seq == UNUSED_SEQ) {
			nbuf->cur_seq = info->seqnum;
		}

		offset = info->seqnum - nbuf->cur_seq + nbuf->idx;
		if (offset >= RTP_NBUF_N_PKT) {
			offset -= RTP_NBUF_N_PKT;
			if (offset >= RTP_NBUF_N_PKT) {
				printd(BG_RED("[ERR] packet offset is way too big: stored:%d cur:%d")"\r\n", nbuf->cur_seq, info->seqnum);
				return -1;
			}
		}
		if (nbuf->pkt[offset].seqnum != UNUSED_SEQ) {
			printd(FG_YELLOW("idx:%d is taken")"\r\n", offset);
			return -2;
		}
		//copy frames
		nbuf->pkt[offset].seqnum = info->seqnum;
		nbuf->pkt[offset].marker = info->marker;
		nbuf->pkt[offset].len    = info->len;
		MEMCPY(nbuf->pkt[offset].data, buf, len);

		//generate frame
		{
			int frm_done = 0;
			int pkt_cnt;
			i = nbuf->idx;
			pkt_cnt = 0;

			do {
				if (nbuf->pkt[i].marker == 1) {
					frm_done = 1;
					pkt_cnt++;
					break;
				}
				if (nbuf->pkt[i].seqnum <= UNUSED_SEQ) {
//printc("break [%d]==%d\r\n", i, nbuf->pkt[i].seqnum);
					break;
				}
				if (nbuf->pkt[i].seqnum != nbuf->cur_seq + pkt_cnt) {
					printd(FG_YELLOW("invalid seq:%d, expected %d")"\r\n", nbuf->pkt[i].seqnum,
							nbuf->cur_seq + pkt_cnt);
				}
				i++;
				if (i >= RTP_NBUF_N_PKT)
					i -= RTP_NBUF_N_PKT;
				pkt_cnt++;
			} while (1);

			if (frm_done) {
				int frame_len, j, used;
				int h264_len;
				unsigned char *frame_buf;
				int max_buf_len;
				int cur_len;
				h264_len = 0;
				frame_len = 0;
				i = nbuf->idx;
				cur_len = 0;
				#define MAX_BUF_LEN (150 * 1024)
				max_buf_len = MAX_BUF_LEN;
				frame_buf = osal_malloc(max_buf_len);

				if (log.malloc)
					printc("malloc:%d to %p, line %d\r\n", max_buf_len, frame_buf, __LINE__);

				if (frame_buf == NULL) {
					printc(BG_RED("[Out of Memory]")" frame_buf\r\n");
					return -3;
				}
				for (j = 0; j < pkt_cnt; ++j) {
					frame_len += nbuf->pkt[i].len;
					rtp_append_h264(&nbuf->pkt[i], frame_buf, &cur_len, &max_buf_len);
					//memcpy //copied twice!
					used = i;
					nbuf->cur_seq = nbuf->pkt[i].seqnum; //could be optimized later
					nbuf->pkt[i].seqnum = UNUSED_SEQ;
					nbuf->pkt[i].marker = 0;
					nbuf->pkt[i].len = 0;
					i++;
					if (i >= RTP_NBUF_N_PKT)
						i -= RTP_NBUF_N_PKT;
				}
				nbuf->cur_seq++;
//					i++;
//					if (i >= RTP_NBUF_N_PKT)
//						i -= RTP_NBUF_N_PKT;
				nbuf->idx = i;
				//printc("reset idx to %d  ", i);
				//printc(FG_GREEN("[frm_done]") " len:%d 264:%d pkt:%d pts:%d\r\n", frame_len, cur_len, pkt_cnt,
				//		info->last_timestamp);
				{
					nbuf->cur_frm.pts = info->last_timestamp;
					nbuf->cur_frm.d = frame_buf;
					nbuf->cur_frm.size = cur_len;
					if (serv && serv->use_frame) {
						serv->use_frame(&nbuf->cur_frm);//rtp_use_frame(&nbuf->cur_frm);
					}
					nbuf->cur_frm.seq++;
				}
				if (log.malloc)
					printc("free: %p, line %d\r\n", frame_buf, __LINE__);
				osal_free(frame_buf);
				frame_buf = NULL;
			}
		}
	}
	return ret;
}

void rtp_reset_nbuf(void)
{
	int i;
	struct rtp_nbuf *nbuf = &rtp_nbuf;
	nbuf->idx = 0;
	nbuf->cur_seq = UNUSED_SEQ;//It could be reported from RTSP PLAY::REPLY
	MEMSET0(&(nbuf->cur_frm));
	for (i = 0; i < RTP_NBUF_N_PKT; ++i) {
		nbuf->pkt[i].seqnum = UNUSED_SEQ;
		nbuf->pkt[i].marker = 0;
	}
}

/*
void rtp_start(char* szParam)
{
	struct amn_session_t *sess;
	struct snkNode_liveRTSP *loc;
	char* path = NULL;
	struct sockaddr_in in_addr;
	char url[] = "rtsp://192.72.1.1/liveRTSP/av1";
	loc = find_rtsp_location( url, path, NULL );
	if (loc == NULL) {
		printd(BG_RED("loc == 0")"\r\n");
		return;
	}
	in_addr.sin_family = AF_INET;
	in_addr.sin_addr = 0x640148C0;//192.72.1.100
	sess = liveRTSP_open( path, loc, &in_addr.sin_addr);
}*/

void rtp_simulator(char* szParam)
{
	int i;
	char fn[64];
	OSAL_FILE *fd;
	static __align(4) char buf[FREAD_BUF_LEN];
	int rtp_len;
	int ret;

	szParam = szParam;//unused;

	MEMSET0(buf);
	rtp_reset_nbuf();

	i = 0;
	do {
		snprintf(fn, 64, "SD:\\RTP\\rtp.%04d", i);
		fd = osal_fopen(fn, "rb");
		if (fd != NULL) {
			osal_flush_dcache();
			rtp_len = osal_fread(buf, 1, FREAD_BUF_LEN, fd);
			osal_invalidate_dcache(buf, rtp_len);
			//MemoryDump(buf, 12);
			if (rtp_len == FREAD_BUF_LEN) {
				printd(BG_RED("Buffer overflow for %s")"\r\n", fn);
			}
			printc("%s packet %d: ", fn, i);
			{
				struct rtp_serv serv;
				serv.use_frame = rtp_cmp_frame_sd;
				ret = rtp_enq_packet(&serv, buf, rtp_len);
			}
			osal_fclose(fd);

			i++;
		}
	} while (fd != NULL);
}

static void do_accept( struct ammo_evmsg *ei, void *d )
{
	struct net_service_t *serv = (struct net_service_t *)d;
	int connfd, i;
	union {
		struct sockaddr_storage storage;
		struct sockaddr_in      in4;
	} clientAddr;
	struct resp_message_t *resp;
	//struct conn *c;

	i = sizeof( clientAddr );
	if( ( connfd = net_accept( serv->fd, (struct sockaddr *)&clientAddr.in4, (socklen_t*)&i ) ) < 0 ) {
		osal_dprint(WARN, "Fails accepting %s connection: %s", serv->name, osal_strerror() );
		// Wait resource
		sys_msleep(500);
		return;
	}
	osal_dprint(DBGS, "accepted %s connection from %s:%d, fd=%d, listener=%d", serv->name,
			inet_ntoa( clientAddr.in4.sin_addr ), ntohs( clientAddr.in4.sin_port ), connfd, serv->fd );

	resp = (struct resp_message_t *)osal_zmalloc( sizeof (struct resp_message_t) );

	if(!resp){
		osal_dprint(ERR, "%s : osal_zmalloc failed", __func__ );
		goto L_fail_do_accept;
	}

	if( net_fcntl( connfd, F_SETFL, O_NONBLOCK ) < 0 ) {
		osal_dprint(INFO, "Fails setting O_NONBLOCK: %s", osal_strerror() );
		goto L_fail_do_accept;
	}

	i = 1;
	if( net_setsockopt( connfd, SOL_TCP, TCP_NODELAY, &i, sizeof( i ) ) < 0 ) {
		osal_dprint(INFO, "Fails setting TCP_NODELAY: %s", osal_strerror() );
		goto L_fail_do_accept;
	}

	// LIBUPNP::miniserver.c::schedule_request_job()

	resp->connfd = connfd;
	resp->serv   = serv;
	memcpy(&resp->foreign_sockaddr, &clientAddr, sizeof(clientAddr));

	if (sys_thread_avaliable(LWIP_STK_SIZE, 0) == 0) {
		printc("++++NO AVALIABLE THREAD to do_accept++++\n");
		goto L_fail_do_accept;
	}
	osal_thread_new( handle_request, resp );
	return;
L_fail_do_accept:
	net_close(connfd);
	if (resp)
		osal_free(resp);
	return;
}

struct net_service_t dbg_service;
int serv_dbg_init(loreq_t *loreq)
{
	static int	_init_ = 0;
	struct net_service_t *serv = &dbg_service;
	struct sockaddr_in addr;
	//struct amn_cfgspace_value_t *cfg_val;
	//int rtsp, httpw_port, https;

	if (_init_ != 0 || loreq->nmtype != NETSERV_TYPE) return 0;
	_init_ = 1;

	osal_dprint(ERR, "================= start =================");

	//start
	serv->port = 56789;
	serv->handler = NULL;
	serv->name = "debug";
	serv->prepare = NULL;

	if( ( serv->fd = net_socket( PF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
		osal_dprint(ERR, "create NetDaemon socket: %s", osal_strerror() );
		return NULL;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = 0;
	addr.sin_port = htons( serv->port );
	if( net_bind( serv->fd, (struct sockaddr *)&addr, sizeof( addr ) ) < 0 ) {
		osal_dprint(ERR, "fail to bind socket: %s", osal_strerror() );
		net_close( serv->fd );
		return NULL;
	}

	if( net_listen( serv->fd, 5 ) < 0 ) {
		osal_dprint(ERR, "fail to setup TCP-LISTEN: %s", osal_strerror() );
		net_close( serv->fd );
		return NULL;
	}

	evmsg_new_FD( serv->fd, 0, 0, do_accept, serv );
	osal_dprint(VERBOSE, "Net Service %s: listen on port %d, fd=%d", serv->name, serv->port, serv->fd );

	//return serv;
	//netserver_new( 56789, NET_SERVICE_DBG, prepare_HTTP_response, http_handle_callback );
	return _init_;
}

#define BUFLEN (2048)
char udp_buf[BUFLEN];
static osal_thread_t handle_rtp(
	/*! [in] Request Message to be handled. */
	void *args)
{
//keep listening for data
	char* buf = udp_buf;
	//struct sockaddr_in* saddr_in = (struct sockaddr_in*) args;
	int rtpfd = (int)args;
    while(1)
    {
    	int recv_len;
    	static int last_recv_time;
    	int time;

        //printc("Waiting for data...");
        //fflush(stdout);

        //try to receive some data, this is a blocking call

        //if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        if ((recv_len = net_recv(rtpfd, buf, BUFLEN, 0)) == -1)
        {
            //die("recvfrom()");
        }
        time = osal_jiffies();
        if (time - last_recv_time > 10*1000) {
        	osal_dprint(INFO, "New RTP stream");
        	rtp_reset_nbuf();
        }
        last_recv_time = time;

        //print details of the client/peer and the data received
        //printc("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        //printc("Data: %s\n" , buf);
        //printc("get %d bytes\r\n", recv_len);
        {
        	struct rtp_serv serv;
        	serv.use_frame = rtp_use_frame;
        	rtp_enq_packet(&serv, buf, recv_len);
        }
        /*
        //now reply the client with the same data
        if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }*/
    }

    net_close(rtpfd);
}


void rtp_start(char* szParam)
{
	struct amn_session_t *sess;
	struct snkNode_liveRTSP *loc;
	char* path = NULL;
	struct sockaddr_in in_addr;
	int track = 0;
	char url[] = "rtsp://192.72.1.1/liveRTSP/v1";
	//char t[] = "RTP/AVP";
	//char trans[80];
	int cport = 45678;
	struct rtsp_session_t *rtsp_sess;

	loc = find_rtsp_location( url, path, NULL );
	if (loc == NULL) {
		printd(BG_RED("loc == 0")"\r\n");
		return;
	}

	//open
	{
		int cnt;
		char tmpstr[100];
		unsigned int port;
		cnt = sscanfl( szParam, " %s %d", tmpstr, &port);
		printc("cnt:%d str:'%s'\r\n", cnt, tmpstr);
		if (cnt > 0) {
	//in_addr.sin_addr.s_addr =
			inet_aton(tmpstr, &in_addr.sin_addr);
			szParam += strlen(tmpstr);
			printc("after parsing:'%s'\r\n", szParam);
			if (cnt >= 2) {
				cport = port;
			}
		} else {
			in_addr.sin_addr.s_addr = 0xFF0148C0;//192.72.1.255
		}
	}
	in_addr.sin_family = AF_INET;
	//in_addr.sin_addr.s_addr = 0x640148C0;//192.72.1.100
	osal_dprint(INFO, "to IP:%s:%d", inet_ntoa(in_addr.sin_addr), cport);
	sess = liveRTSP_open( path, loc, inet_ntoa(in_addr.sin_addr));
	if (loc == NULL) {
		printd(BG_RED("sess == 0")"\r\n");
		return;
	}

	nhw_set_status(NETAPP_NET_STATUS_READY);

	//describe
	{
		int sdp_len;
		struct ammo_pipe **pp;
		char temp[100];
		pp = cast_SnkNode(sess->owner)->inputs;
		#define cast_RtpMedia(p)    ((struct rtp_media *)((p)->data))
		sdp_len = cast_RtpMedia(*pp)->get_sdp( temp, sizeof(temp) - 1, 96 + track, cport, cast_RtpMedia(*pp)->private );
		if (sdp_len <= 0) {
			session_destroy( sess, "SDP-ERR" );
			return;
		}
	}

	if( liveRTSP_setup( sess, track ) < 0 ) {
		session_destroy( sess, "SETUP-ERR" );
		return;
	}

	//rtsp_udp_setup
	rtsp_sess = cast_protocol_data(sess, rtsp_session_t);
	sprintf( rtsp_sess->addr, "%s", inet_ntoa(in_addr.sin_addr) );
	rtsp_sess->kal.b_rtsptcp = 0;
	{
		int server_port = 22222;
		if( connect_udp_endpoint( rtsp_sess->ep[track], in_addr.sin_addr, cport, &server_port ) < 0 ) {
			printd(BG_RED("connect fail")"\r\n");
			session_destroy( sess, "CONNECT-ERR" );
			return ;
		}
		printc("server port:%d\r\n", server_port);
		printc("target:IP:%X port:%d\r\n", in_addr.sin_addr.s_addr, rtsp_sess->ep[track]->trans.udp.sdp_port);
		printc("rtp_fd:%d rtp_event:%p, trans_type:%d, ip:%s\r\n",
				rtsp_sess->ep[track]->trans.udp.rtp_fd,
				rtsp_sess->ep[track]->trans.udp.rtp_event,
				rtsp_sess->ep[track]->trans_type,
				rtsp_sess->ep[track]->trans.udp.sdp_addr);
		//net_send(rtsp_sess->ep[track]->trans.udp.rtp_fd, "hello\n", 7, 0);
		//session_destroy( sess, "SETUP-TEST" );
		//return;
	}
	ns_set_streaming_status(NETAPP_STREAM_NONE);

	//nhw_set_streaming_mac(ethaddr_ret->addr);
	liveRTSP_play( sess, NULL );
	ns_set_streaming_status(NETAPP_STREAM_PLAY);
}

void rtp_end(char* szParam)
{
	/*
	struct snkNode_liveRTSP *loc;
	char url[] = "rtsp://192.72.1.1/liveRTSP/v1";
	char *path = NULL;
	struct rtsp_session_t *rtsp_sess;

	loc = find_rtsp_location( url, path, NULL );
	if (loc == NULL) {
		printd(BG_RED("loc == 0")"\r\n");
		return;
	}*/
	session_destroyall();
	ns_set_streaming_status(NETAPP_STREAM_TEARDOWN);
}

static
UARTCOMMAND sUartRtpCmdsp[] =
{
	{ "rtp",    "",                        "simulate RTP ",             rtp_simulator},
	{ "rtps",   "",                        "RTP server Start",          rtp_start},
	{ "rtpe",   "",                        "RTP server End",            rtp_end},
};

//int connect_udp_endpoint( struct rtp_endpoint *ep, struct in_addr dest_ip, int dest_port, int *our_port )
#define RTP_UDP_PORT (45678)
int serv_rtp_udp(loreq_t *loreq)
{
	static int	_init_ = 0;
	static struct sockaddr_in rtpaddr;//, rtcpaddr;
	int port, success = 0, i, max_tries, rtpfd = -1;//, rtcpfd = -1;
	//int rtptos;//,rtcptos;

	if (_init_ != 0 || loreq->nmtype != NETSERV_TYPE) return 0;
	_init_ = 1;
	UartRegisterUartCmdList(sUartRtpCmdsp);

	rtpaddr.sin_family = AF_INET;
	//rtcpaddr.sin_family = AF_INET;
	rtpaddr.sin_addr.s_addr = 0;
	//rtcpaddr.sin_addr.s_addr = 0;
	rtp_reset_nbuf();
	//port = rtp_port_start + random() % ( rtp_port_end - rtp_port_start );
	port = RTP_UDP_PORT;
	if( port & 0x1 ) ++port;
	//max_tries = ( rtp_port_end - rtp_port_start + 1 ) / 2;
	max_tries = 1;

	for( i = 0; i < max_tries; ++i ) {
		//if( port + 1 > rtp_port_end ) port = rtp_port_start;
		rtpaddr.sin_port = htons( port );
		//rtcpaddr.sin_port = htons( port + 1 );
		if( rtpfd < 0 && ( rtpfd = net_socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 ) {
			osal_dprint(WARN, "unable to create RTP socket: %s", osal_strerror() );
			return -1;
		}
		/*
		if( rtcpfd < 0 && ( rtcpfd = net_socket( PF_INET, SOCK_DGRAM, 0 ) ) < 0 ) {
			osal_dprint(WARN, "unable to create RTCP socket: %s", osal_strerror() );
			net_close( rtpfd );
			return -1;
		}
		*/
		if( net_bind( rtpfd, (struct sockaddr *)&rtpaddr, sizeof( rtpaddr ) ) < 0 ) {
			if( errno == EADDRINUSE ) {
				port += 2;
				continue;
			}
			osal_dprint(WARN, "strange error when binding RTP socket: %s", osal_strerror() );
			net_close( rtpfd );
			//net_close( rtcpfd );
			return -1;
		}
		/*
		if( net_bind( rtcpfd, (struct sockaddr *)&rtcpaddr, sizeof( rtcpaddr ) ) < 0 ) {
			if( errno == EADDRINUSE ) {
				net_close( rtpfd );
				rtpfd = -1;
				port += 2;
				continue;
			}
			osal_dprint(WARN, "strange error when binding RTCP socket: %s", osal_strerror() );
			net_close( rtpfd );
			net_close( rtcpfd );
			return -1;
		}*/
		success = 1;
		break;
	}
	if( ! success ) {
		osal_dprint(WARN, "ran out of RTP ports!" );
		return -1;
	}
	osal_thread_new( handle_rtp, (void*)rtpfd );
#if 0
	rtpaddr.sin_family = AF_INET;
	//rtcpaddr.sin_family = AF_INET;
	rtpaddr.sin_addr = dest_ip;
	//rtcpaddr.sin_addr = dest_ip;
	rtpaddr.sin_port = htons( dest_port );
	//rtcpaddr.sin_port = htons( dest_port + 1 );

	//set TOS of rtp/rtcp to user priority 7 to improve throughput,it maps to AC_VO
	rtptos = 0xE0;
	net_setsockopt(rtpfd, IPPROTO_IP, IP_TOS, &rtptos, sizeof(int));
	//rtcptos = 0xE0;
	//net_setsockopt(rtcpfd, IPPROTO_IP, IP_TOS, &rtcptos, sizeof(int));

	if( net_connect( rtpfd, (struct sockaddr *)&rtpaddr, sizeof( rtpaddr ) ) < 0 ) {
		osal_dprint(WARN, "strange error when connecting RTP socket: %s",
				osal_strerror() );
		net_close( rtpfd );
		//net_close( rtcpfd );
		return -1;
	}
	/*
	if( net_connect( rtcpfd, (struct sockaddr *)&rtcpaddr, sizeof( rtcpaddr ) ) < 0 ) {
		osal_dprint(WARN, "strange error when connecting RTCP socket: %s", osal_strerror() );
		net_close( rtpfd );
		net_close( rtcpfd );
		return -1;
	}*/
	i = sizeof( rtpaddr );
	if( net_getsockname( rtpfd, (struct sockaddr *)&rtpaddr, (socklen_t*)&i ) < 0 ) {
		osal_dprint(WARN, "strange error from getsockname: %s", osal_strerror() );
		net_close( rtpfd );
		//net_close( rtcpfd );
		return -1;
	}
#endif
#if 0
	i = (PRIO_8021D_VI << IPV4_TOS_PREC_SHIFT) + IPV4_TOS_LOWDELAY;
	if (setsockopt( rtpfd, IPPROTO_IP, IP_TOS, &i, sizeof(i) ) < 0)
		osal_dprint(WARN, "error setsockopt on socket %d: %s", rtpfd, osal_strerror() );
#endif

#if 0
	ep->max_data_size = 1400; /* good guess for preventing fragmentation */
	ep->trans_type = RTP_TRANS_UDP;
	sprintf( ep->trans.udp.sdp_addr, "IP4 %s", inet_ntoa( rtpaddr.sin_addr ) );
	ep->trans.udp.sdp_port = ntohs( rtpaddr.sin_port );
	ep->trans.udp.rtp_fd = rtpfd;
	ep->trans.udp.rtcp_fd = rtcpfd;
	ep->trans.udp.rtp_event  = evmsg_new_FD( rtpfd,  0, 0, udp_rtp_read,  ep );
	ep->trans.udp.rtcp_event = evmsg_new_FD( rtcpfd, 0, 0, udp_rtcp_read, ep );

	*our_port = port;
#endif
	return 0;
}

#pragma arm section code = "netnode_init", rwdata = "netnode_init", zidata = "netnode_init"
netnode_init_t	_serv_init_dbg = serv_dbg_init;
netnode_init_t	_serv_init_udp = serv_rtp_udp;
#pragma arm section code, rwdata,  zidata
