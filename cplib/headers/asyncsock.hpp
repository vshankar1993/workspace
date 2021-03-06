/*
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * */
#ifndef __ASYNCSOCK_H
#define __ASYNCSOCK_H

#include <map>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <unistd.h>
#include "cplib.hpp"
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/un.h>
#include <boost/function.hpp>
#include <sstream>
#ifdef __debug_print123
#define dbgprint(msg) cout << msg << endl
#else
#define dbgprint(msg)
#endif
using namespace std;
namespace xaxaxa
{
	namespace Sockets
	{
		typedef FILEDES SOCKET;
#define CreateSocket(domain,type,protocol) socket(domain,type,protocol);

		struct IPAddress
		{
			in_addr a;
			inline IPAddress()
			{
			}
			inline IPAddress(const char* addr/*NOT hostname*/)
			{
				inet_pton(AF_INET, addr, &a.s_addr);
			}
			inline IPAddress(const in_addr& a)
			{
				this->a = a;
			}
			inline bool operator<(const IPAddress& other) const
			{
				return ntohl(a.s_addr) < ntohl(other.a.s_addr);
			}
			inline bool operator>(const IPAddress& other) const
			{
				return ntohl(a.s_addr) > ntohl(other.a.s_addr);
			}
			inline bool operator<=(const IPAddress& other) const
			{
				return ntohl(a.s_addr) <= ntohl(other.a.s_addr);
			}
			inline bool operator>=(const IPAddress& other) const
			{
				return ntohl(a.s_addr) >= ntohl(other.a.s_addr);
			}
			inline bool operator==(const IPAddress& other) const
			{
				return a.s_addr == other.a.s_addr;
			}
			inline IPAddress operator+(const IPAddress& other) const
			{
				return IPAddress(
				{ htonl(ntohl(a.s_addr) + other.a.s_addr) });
			}
			inline IPAddress operator-(const IPAddress& other) const
			{
				return IPAddress(
				{ htonl(ntohl(a.s_addr) - other.a.s_addr) });
			}
			inline IPAddress operator+(const in_addr_t& other) const
			{
				//WARN(1,a.s_addr << " " <<ntohl(a.s_addr));
				//cout << "a" << endl;
				return IPAddress(
				{ htonl(ntohl(a.s_addr) + other) });
			}
			inline IPAddress operator-(const in_addr_t& other) const
			{
				return IPAddress(
				{ htonl(ntohl(a.s_addr) - other) });
			}
			string ToStr() const
			{
				char tmp[INET_ADDRSTRLEN];
				if (inet_ntop(AF_INET, &a, tmp, INET_ADDRSTRLEN) == NULL)
					throw Exception(errno);
				return string(tmp);
			}
		};
		struct IPv6Address
		{
			in6_addr a;
			inline IPv6Address()
			{
			}
			inline IPv6Address(const char* addr)
			{
				inet_pton(AF_INET6, addr, &a.__in6_u);
			}
			inline IPv6Address(const in6_addr& a)
			{
				this->a = a;
			}
			/*inline bool operator<(const IPv6Address& other) const
			 {
			 return a < other.a;
			 }
			 inline bool operator>(const IPv6Address& other) const
			 {
			 return a > other.a;
			 }
			 inline bool operator<=(const IPv6Address& other) const
			 {
			 return a <= other.a;
			 }
			 inline bool operator>=(const IPv6Address& other) const
			 {
			 return a >= other.a;
			 }
			 inline bool operator==(const IPv6Address& other) const
			 {
			 return a == other.a;
			 }
			 inline IPv6Address operator+(const IPv6Address& other) const
			 {
			 return IPv6Address(a + other.a);
			 }
			 inline IPv6Address operator-(const IPv6Address& other) const
			 {
			 return IPv6Address(a - other.a);
			 }
			 inline IPv6Address operator+(const in_addr& other) const
			 {
			 return IPv6Address(a + other);
			 }
			 inline IPv6Address operator-(const in_addr& other) const
			 {
			 return IPv6Address(a - other);
			 }*/
			string ToStr() const
						{
							char tmp[INET_ADDRSTRLEN];
							if (inet_ntop(AF_INET6, &a, tmp, INET6_ADDRSTRLEN) == NULL)
								throw Exception(errno);
							return string(tmp);
						}
		};
		class EndPoint: public Object
		{
		public:
			int AddressFamily;
			virtual void GetSockAddr(sockaddr* addr) const=0;
			virtual void SetSockAddr(const sockaddr* addr)=0;
			virtual int GetSockAddrSize() const=0;
			static EndPoint* FromSockAddr(const sockaddr* addr);
			static EndPoint* CreateNull(int AddressFamily);
			virtual void Clone(EndPoint& to) const=0;
			virtual ~EndPoint()
			{
			}
			static vector<Property<EndPoint> > LookupHost(const char* hostname, const char* port,
					int flags = 0, int socktype = 0, int proto = 0)
			{
				vector<Property<EndPoint> > tmp;
				addrinfo hints, *result, *rp;
				memset(&hints, 0, sizeof(struct addrinfo));
				hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
				hints.ai_socktype = socktype;
				hints.ai_flags = flags;
				hints.ai_protocol = proto;

				int s = getaddrinfo(hostname, port, &hints, &result);
				if (s != 0)
				{
					throw Exception(gai_strerror(s));
				}
				for (rp = result; rp != NULL; rp = rp->ai_next)
				{
					EndPoint* ep = FromSockAddr(rp->ai_addr);
					tmp.push_back(ep);
					ep->Release();
				}
				freeaddrinfo(result);
				return tmp;
			}
			//static EndPoint Resolve(
		};
		class IPEndPoint: public EndPoint
		{
		public:
			IPAddress Address;
			int Port;
			IPEndPoint()
			{
				this->AddressFamily = AF_INET;
			}
			IPEndPoint(IPAddress address, int port)
			{
				this->AddressFamily = AF_INET;
				this->Address = address;
				this->Port = port;
			}
			void set_addr(const sockaddr_in& addr)
			{
				this->AddressFamily = AF_INET;
				this->Address = IPAddress(addr.sin_addr);
				this->Port = ntohs(addr.sin_port);
			}
			virtual void SetSockAddr(const sockaddr* addr)
			{
				if (addr->sa_family != AF_INET)
					throw Exception(
							"attemting to set the address of an IPEndPoint to a sockaddr that is not AF_INET");
				set_addr(*(sockaddr_in*) addr);
			}
			IPEndPoint(const sockaddr_in& addr)
			{
				set_addr(addr);
			}
			virtual void GetSockAddr(sockaddr* addr) const
			{
				sockaddr_in* addr_in = (sockaddr_in*) addr;
				addr_in->sin_family = AF_INET;
				addr_in->sin_port = htons(Port);
				addr_in->sin_addr = Address.a;
			}
			virtual int GetSockAddrSize() const
			{
				return sizeof(sockaddr_in);
			}
			virtual void Clone(EndPoint& to) const
			{
				if (to.AddressFamily != AddressFamily)
					throw Exception(
							"attempting to clone an EndPoint to another EndPoint with a different AddressFamily");
				IPEndPoint& tmp((IPEndPoint&) to);
				tmp.Address = Address;
				tmp.Port = Port;
			}
			virtual string ToStr() const
			{
				stringstream s;
				s << Address.ToStr() << ':' << Port;
				return s.str();
			}
		};
		class IPv6EndPoint: public EndPoint
		{
		public:
			IPv6Address Address;
			in_port_t Port;
			uint32_t FlowInfo;
			uint32_t ScopeID;
			IPv6EndPoint()
			{
				this->AddressFamily = AF_INET6;
			}
			IPv6EndPoint(IPv6Address address, in_port_t port)
			{
				this->AddressFamily = AF_INET6;
				this->Address = address;
				this->Port = port;
			}
			void set_addr(const sockaddr_in6& addr)
			{
				this->AddressFamily = AF_INET6;
				this->Address = IPv6Address(addr.sin6_addr);
				this->Port = ntohs(addr.sin6_port);
				FlowInfo = addr.sin6_flowinfo;
				ScopeID = addr.sin6_scope_id;
			}
			IPv6EndPoint(const sockaddr_in6& addr)
			{
				set_addr(addr);
			}
			virtual void SetSockAddr(const sockaddr* addr)
			{
				if (addr->sa_family != AF_INET6)
					throw Exception(
							"attemting to set the address of an IPv6EndPoint to a sockaddr that is not AF_INET6");
				set_addr(*(sockaddr_in6*) addr);
			}
			virtual void GetSockAddr(sockaddr* addr) const
			{
				sockaddr_in6* addr_in = (sockaddr_in6*) addr;
				addr_in->sin6_family = AF_INET6;
				addr_in->sin6_port = htons(Port);
				addr_in->sin6_addr = Address.a;
				addr_in->sin6_flowinfo = FlowInfo;
				addr_in->sin6_scope_id = ScopeID;
			}
			virtual int GetSockAddrSize() const
			{
				return sizeof(sockaddr_in);
			}
			virtual void Clone(EndPoint& to) const
			{
				if (to.AddressFamily != AddressFamily)
					throw Exception(
							"attempting to clone an EndPoint to another EndPoint with a different AddressFamily");
				IPv6EndPoint& tmp((IPv6EndPoint&) to);
				tmp.Address = Address;
				tmp.Port = Port;
				tmp.FlowInfo = FlowInfo;
				tmp.ScopeID = ScopeID;
			}
			virtual string ToStr() const
			{
				stringstream s;
				s << '[' << Address.ToStr() << "]:" << Port;
				return s.str();
			}
		};
		class UNIXEndPoint: public EndPoint
		{
		public:
			string Name;
			UNIXEndPoint()
			{
				this->AddressFamily = AF_UNIX;
			}
			UNIXEndPoint(string name)
			{
				this->AddressFamily = AF_UNIX;
				Name = name;
			}
			void set_addr(const sockaddr_un& addr)
			{
				this->AddressFamily = AF_UNIX;
				this->Name = addr.sun_path;
			}
			UNIXEndPoint(const sockaddr_un& addr)
			{
				set_addr(addr);
			}
			virtual void SetSockAddr(const sockaddr* addr)
			{
				if (addr->sa_family != AF_UNIX)
					throw Exception(
							"attemting to set the address of an UNIXEndPoint to a sockaddr that is not AF_UNIX");
				set_addr(*(sockaddr_un*) addr);
			}
			virtual void GetSockAddr(sockaddr* addr) const
			{
				sockaddr_un* a = (sockaddr_un*) addr;
				a->sun_family = AF_UNIX;
				strncpy(a->sun_path, Name.c_str(), Name.length());
				a->sun_path[Name.length()] = '\0';
			}
			virtual int GetSockAddrSize() const
			{
				return sizeof(sa_family_t) + Name.length() + 1;
			}
			virtual void Clone(EndPoint& to) const
			{
				if (to.AddressFamily != AddressFamily)
					throw Exception(
							"attempting to clone an EndPoint to another EndPoint with a different AddressFamily");
				UNIXEndPoint& tmp((UNIXEndPoint&) to);
				tmp.Name = Name;
			}
		};
		struct Socket: public File
		{
			//SOCKET _s;
			/*bool autoClose;
			 inline Socket NoDestruct()
			 {
			 Socket s=*this;
			 s.autoClose=false;
			 return s;
			 }*/
			inline Socket()
			{
			}
			inline Socket(int domain, int type, int protocol = 0)
			{
				_f = CreateSocket(domain,type,protocol);
				if (_f < 0)
					throw Exception(errno);
				//int set = 1;
				//setsockopt(_s, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
				dbgprint("socket " << _f << " created");
				//this->autoClose=autoClose;
			}
			inline Socket(SOCKET s)
			{
				_f = s;
				//int set = 1;
				//setsockopt(_s, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
				//this->autoClose=autoClose;
			}
			/*inline void Close()
			 {
			 //throw Exception();
			 dbgprint("socket " << _f << " closed");
			 if(_s!=-1)close(_f);
			 _s=-1;
			 }*/
			inline void Shutdown(int how)
			{
				shutdown(_f, how);
			}
			inline int Send(const BufferRef& buf, Int flags = 0)
			{
				retry: Int tmp = send(_f, buf.Data, buf.Length, flags);
				if (tmp < 0)
				{
					if (errno == 4)
						goto retry;
					throw Exception(errno);
				}
				else
					return tmp;
			}
			inline int Recv(const BufferRef& buf, Int flags = 0)
			{
				retry: Int tmp = recv(_f, buf.Data, buf.Length, flags);
				if (tmp < 0)
				{
					if (errno == 4)
						goto retry;
					throw Exception(errno);
				}
				else
					return tmp;
			}
			inline int RecvFrom(const BufferRef& buf, EndPoint& ep, Int flags = 0)
			{
				socklen_t size = ep.GetSockAddrSize();
				uint8_t addr[size];
				//ep->GetSockAddr((sockaddr*)tmp);
				retry: Int tmp = recvfrom(_f, buf.Data, buf.Length, flags, (sockaddr*) addr, &size);
				if (tmp < 0)
				{
					if (errno == 4)
						goto retry;
					throw Exception(errno);
				}
				else
				{
					ep.SetSockAddr((sockaddr*) addr);
					return tmp;
				}
			}
			inline int SendTo(const BufferRef& buf, const EndPoint& ep, Int flags = 0)
			{
				socklen_t size = ep.GetSockAddrSize();
				uint8_t addr[size];
				ep.GetSockAddr((sockaddr*) addr);
				retry: Int tmp = sendto(_f, buf.Data, buf.Length, flags, (sockaddr*) addr, size);
				if (tmp < 0)
				{
					if (errno == 4)
						goto retry;
					throw Exception(errno);
				}
				else
				{
					//ep.SetSockAddr((sockaddr*)addr);
					return tmp;
				}
			}
			inline Socket Accept(Int flags = 0)
			{
				retry: SOCKET s = accept4(_f, NULL, NULL, flags);
				if (s == -1)
				{
					if (errno == 4)
						goto retry;
					throw Exception(errno);
				}
				return Socket(s);
			}
			inline void Connect(sockaddr *addr, int addr_size = 0)
			{
				retry: Int tmp = connect(_f, addr, (addr_size == 0 ? sizeof(sockaddr) : addr_size));
				if (tmp != 0 && errno != 115)
				{
					if (errno == 4)
						goto retry;
					throw Exception(errno);
				}
			}
			inline void Bind(sockaddr *addr, Int addr_size = 0)
			{
				int tmp12345 = 1;
				setsockopt(_f, SOL_SOCKET, SO_REUSEADDR, &tmp12345, sizeof(tmp12345));
				if (::bind(_f, addr, (Int) (addr_size == 0 ? sizeof(sockaddr) : addr_size)) != 0)
					throw Exception(errno);
			}
			inline void Bind(EndPoint *ep)
			{
				//cout << _f << endl;
				int tmp12345 = 1;
				setsockopt(_f, SOL_SOCKET, SO_REUSEADDR, &tmp12345, sizeof(tmp12345));
				Int size = ep->GetSockAddrSize();
				uint8_t tmp[size];
				ep->GetSockAddr((sockaddr*) tmp);
				if (::bind(_f, (sockaddr*) tmp, size) != 0)
					throw Exception(errno);
			}
			inline void Bind(const EndPoint& ep)
			{
				Bind((EndPoint*) &ep);
			}
			inline void Bind(const char* hostname, const char* port, int socktype, int proto = 0)
			{
				if (_f != 0)
					throw Exception(
							"Socket::Bind(const char* hostname, const char* port) creates a socket, but the socket is already constructed; use the default constructor instead.");
				auto hosts = EndPoint::LookupHost(hostname, port, AI_PASSIVE, socktype, proto);
				UInt i;
				for (i = 0; i < hosts.size(); i++)
				{
					_f = CreateSocket(hosts[i]->AddressFamily,socktype,proto);
					if (_f < 0)
						continue;
					int tmp12345 = 1;
					setsockopt(_f, SOL_SOCKET, SO_REUSEADDR, &tmp12345, sizeof(tmp12345));
					Int size = hosts[i]->GetSockAddrSize();
					uint8_t tmp[size];
					hosts[i]->GetSockAddr((sockaddr*) tmp);
					if (::bind(_f, (sockaddr*) tmp, size) == 0)
						goto aaaaa;
					else
					{
						close(_f);
						continue;
					}
				}
				throw Exception(errno);
				aaaaa: ;
			}
			inline void Connect(const char* hostname, const char* port, int socktype, int proto = 0)
			{
				if (_f != 0)
					throw Exception(
							"Socket::Connect(const char* hostname, const char* port) creates a socket, but the socket is already constructed; use the default constructor instead.");
				auto hosts = EndPoint::LookupHost(hostname, port, 0, socktype, proto);
				UInt i;
				for (i = 0; i < hosts.size(); i++)
				{
					_f = CreateSocket(hosts[i]->AddressFamily,socktype,proto);
					if (_f < 0)
						continue;
					Int size = hosts[i]->GetSockAddrSize();
					uint8_t tmp[size];
					hosts[i]->GetSockAddr((sockaddr*) tmp);
					if (::connect(_f, (sockaddr*) tmp, size) == 0)
						goto aaaaa;
					else
					{
						close(_f);
						continue;
					}
				}
				throw Exception(errno);
				aaaaa: ;
			}
			inline void Connect(EndPoint *ep)
			{
				int size = ep->GetSockAddrSize();
				uint8_t asdf[size];
				ep->GetSockAddr((sockaddr*) asdf);
				retry: int tmp = connect(_f, (sockaddr*) asdf, size);
				if (tmp != 0 && errno != 115)
				{
					if (errno == 4)
						goto retry;
					throw Exception(errno);
				}
			}
			inline void Connect(const EndPoint& ep)
			{
				Connect((EndPoint*) &ep);
			}
			inline void Listen(int backlog)
			{
				if (listen(_f, backlog) != 0)
					throw Exception(errno);
			}
			inline int GetFlags()
			{
				return fcntl(_f, F_GETFL, 0);
			}
			inline void SetFlags(int f)
			{
				if (fcntl(_f, F_SETFL, f) < 0)
					throw Exception(errno, "could not set socket flags");
			}
			inline void GetPeerEndPoint(EndPoint& ep)
			{
				socklen_t size = ep.GetSockAddrSize();
				uint8_t addr[size];
				getpeername(_f, (sockaddr*)&addr, &size);
				ep.SetSockAddr((sockaddr*)&addr);
			}
			/*inline ~Socket()
			 {
			 if(autoClose)Close();
			 }*/
		};
		class SocketManager;

		class SocketManager: public Object
		{
		public:
			static SocketManager* __def_sockmanager;
			inline static SocketManager* GetDefault()
			{
				if (__def_sockmanager == NULL)
					__def_sockmanager = new SocketManager();
				return __def_sockmanager;
			}
			//typedef boost::function<void (SocketManager*, Socket)> Callback;
			FUNCTION_DECLARE(Callback,void,SocketManager*,Socket);
			int epollfd;
			//arraylist<Socket> r,w,x;
			//arraylist<Socket> r2,w2,x2;
			//map<Socket,void*> r_info;
			//map<Socket,void*> w_info;
			//map<Socket,void*> x_info;
			/*enum taskType
			 {
			 read,write,accept
			 };*/

			struct taskInfo
			{
				//bool initialized;
				int bits; //1: init; 2: read_fill; 4: write_fill;
				SOCKET s;
				Callback cb_recv, cb_send;
				__uint32_t events;
				__uint32_t new_events;
				Buffer r_buf, w_buf;
				int r_length, w_length;
				int r_errno, w_errno;
				inline taskInfo()
				{
					bits = 0;
				}
				//bool inLoop;//reserved; do not modify
			};
			map<SOCKET, taskInfo> info;
			//ArrayList<SOCKET> tmp_erased;
			map<SOCKET, bool> tmp_erased;
			SocketManager();
			~SocketManager();
			virtual void EventLoop();
			void BeginRecv(Socket s, const Buffer& buf, const Callback& cb, bool fill = false);
			void BeginRead(Socket s, const Buffer& buf, const Callback& cb, bool fill = false);
			int EndRecv(Socket s);
			int EndRead(Socket s);
			int EndRecvFrom(Socket s, EndPoint& ep);
			void BeginAccept(Socket s, const Callback& cb);
			Socket EndAccept(Socket s, int flags = 0);
			void BeginSend(Socket s, const Buffer& buf, const Callback& cb, bool fill = false);
			void BeginWrite(Socket s, const Buffer& buf, const Callback& cb, bool fill = false);
			int EndSend(Socket s);
			int EndWrite(Socket s);
			int EndSendTo(Socket s, EndPoint& ep);
			void BeginConnect(Socket s, EndPoint* ep, const Callback& cb);
			void EndConnect(Socket s);
			taskInfo* __current_task_;
			void Cancel(Socket s);
			void CancelRead(Socket s);
			void CancelWrite(Socket s);
		};
		class SocketStream: public Stream
		{
		public:
			Socket socket;
			SocketManager* m;
			SocketStream() :
					closed(false)
			{
			}
			SocketStream(Socket socket, SocketManager *m = NULL) :
					closed(false)
			{
				this->socket = socket;
				if (m == NULL)
					m = SocketManager::GetDefault();
				this->m = m;
			}
			virtual ~SocketStream()
			{
				Close();
			}
			virtual int Read(const BufferRef& buf)
			{
				return socket.Recv(buf, 0);
			}
			virtual void Write(const BufferRef& buf)
			{
				int bw = 0;
				int off = 0;
				while (off < buf.Length && (bw = socket.Send(buf.SubBuffer(off))) > 0)
					off += bw;
			}
			virtual void Flush()
			{

			}
			Callback r_cb;
			Callback w_cb;
			bool closed;
			virtual void Close()
			{
				r_cb.obj = NULL;
				w_cb.obj = NULL;
				m->Cancel(socket);
				if (closed)
					return;
				closed = true;
				socket.Close();
			}
			FUNCTION_DECLWRAPPER(_r_cb,void,SocketManager* m,Socket sock)
			{
				FUNCTION_CALL(((SocketStream*)obj)->r_cb, (SocketStream*)obj);
			}
			FUNCTION_DECLWRAPPER(_w_cb,void,SocketManager* m,Socket sock)
			{
				FUNCTION_CALL(((SocketStream*)obj)->w_cb, (SocketStream*)obj);
			}
			virtual void BeginRead(const Buffer& buf, Callback cb)
			{
				dbgprint("SocketStream::BeginRead()");
				r_cb = cb;
				m->BeginRecv(socket, buf, SocketManager::Callback(_r_cb, this), false);
			}
			;
			virtual void BeginWrite(const Buffer& buf, Callback cb)
			{
				dbgprint("SocketStream::BeginWrite()");
				w_cb = cb;
				m->BeginSend(socket, buf, SocketManager::Callback(_w_cb, this), true);
			}
			virtual int EndRead()
			{
				return m->EndRecv(socket);
			}
			virtual void EndWrite()
			{
				m->EndSend(socket);
			}
			;
		};
		class SOCKS5
		{
		public:
			//typedef boost::function<void (Stream*, void*)> Callback;
			FUNCTION_DECLARE(Callback,void,Stream*,void*);
			struct tmp
			{
				Buffer b;
				//StringBuilder* sb;
				//int addrlen;
				Callback cb;
				Callback sent_cb;
				//int br;
				Exception* ex;
				//void* state;
			};
			struct socks_auth_response
			{
				Byte version, auth;
			}__attribute__((packed));
			struct socks_response
			{
				Byte version, status, reserved, addrtype, addrlen;
			}__attribute__((packed));
			static void socks_connect(Stream* s, const char* host, uint16_t port, Callback cb,
					Callback sent_cb);
			static void socks_connect(Stream* s, EndPoint* ep, Callback cb, Callback sent_cb =
					Callback::null);
			static void auth_cb(void* obj, Stream* s);
			static void auth_received_cb(void* obj, Stream* s);
			static void socks_endconnect(void* v);
			static void cb1(void* obj, Stream* s);
			static void cb2(void* obj, Stream* s);
			static void cb2_1(void* obj, Stream* s);
		};

	}
}

#endif
