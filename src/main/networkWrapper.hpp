// A network wrapper class modified from the tutorial:
// https://www.gamedev.net/blog/950/entry-2249317-a-guide-to-getting-started-with-boostasio/

#ifndef NETWORK_WRAPPER_HPP
#define NETWORK_WRAPPER_HPP

//-----------------------------------------------------------------------------

#include <string>
#include <vector>
#include <list>
#include <cstdint>
#include <functional>
#include <asio.hpp>
#include <chrono>

//-----------------------------------------------------------------------------

class Hive;
class Acceptor;
class Connection;

//-----------------------------------------------------------------------------

class Connection : public std::enable_shared_from_this< Connection >
{
    friend class Acceptor;
    friend class Hive;

private:
    std::shared_ptr< Hive > m_hive;
    asio::ip::tcp::socket m_socket;
    asio::io_service::strand m_io_strand;
    asio::basic_waitable_timer<std::chrono::high_resolution_clock> m_timer;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_last_time;
    std::vector< uint8_t > m_recv_buffer;
    std::list< int32_t > m_pending_recvs;
    std::list< std::vector< uint8_t > > m_pending_sends;
    int32_t m_receive_buffer_size;
    int32_t m_timer_interval;
    volatile std::atomic<bool> m_error_state;

protected:
    Connection( std::shared_ptr< Hive > hive );
    virtual ~Connection();

private:
    Connection( const Connection & rhs );
    Connection & operator =( const Connection & rhs );
    void StartSend();
    void StartRecv( int32_t total_bytes );
    void StartTimer();
    void StartError( const asio::error_code & error );
    void DispatchSend( std::vector< uint8_t > buffer );
    void DispatchRecv( int32_t total_bytes );
    void DispatchTimer( const asio::error_code & error );
    void HandleConnect( const asio::error_code & error );
    void HandleSend( const asio::error_code & error,
            std::list< std::vector< uint8_t > >::iterator itr );
    void HandleRecv( const asio::error_code & error,
            int32_t actual_bytes );
    void HandleTimer( const asio::error_code & error );

private:
    // Called when the connection has successfully connected to the local
    // host.
    virtual void OnAccept( const std::string & host, uint16_t port ) = 0;

    // Called when the connection has successfully connected to the remote
    // host.
    virtual void OnConnect( const std::string & host, uint16_t port ) = 0;

    // Called when data has been sent by the connection.
    virtual void OnSend( const std::vector< uint8_t > & buffer ) = 0;

    // Called when data has been received by the connection.
    virtual void OnRecv( std::vector< uint8_t > & buffer ) = 0;

    // Called on each timer event.
    virtual void OnTimer( const std::chrono::milliseconds & delta ) = 0;

    // Called when an error is encountered.
    virtual void OnError( const asio::error_code & error ) = 0;

public:
    // copy the object
    virtual std::shared_ptr<Connection> NewConnection() = 0;

    // Returns the Hive object.
    std::shared_ptr< Hive > GetHive();

    // Returns the socket object.
    asio::ip::tcp::socket & GetSocket();

    // Returns the strand object.
    asio::io_service::strand & GetStrand();

    // Sets the application specific receive buffer size used. For stream
    // based protocols such as HTTP, you want this to be pretty large, like
    // 64kb. For packet based protocols, then it will be much smaller,
    // usually 512b - 8kb depending on the protocol. The default value is
    // 4kb.
    void SetReceiveBufferSize( int32_t size );

    // Returns the size of the receive buffer size of the current object.
    int32_t GetReceiveBufferSize() const;

    // Sets the timer interval of the object. The interval is changed after
    // the next update is called.
    void SetTimerInterval( int32_t timer_interval_ms );

    // Returns the timer interval of the object.
    int32_t GetTimerInterval() const;

    // Returns true if this object has an error associated with it.
    bool HasError();

    // Binds the socket to the specified interface.
    void Bind( const std::string & ip, uint16_t port );

    // Starts an a/synchronous connect.
    void Connect( const std::string & host, uint16_t port );

    // Posts data to be sent to the connection.
    void Send( const std::vector< uint8_t > & buffer );

    // Posts a recv for the connection to process. If total_bytes is 0, then
    // as many bytes as possible up to GetReceiveBufferSize() will be
    // waited for. If Recv is not 0, then the connection will wait for exactly
    // total_bytes before invoking OnRecv.
    void Recv( int32_t total_bytes = 0 );

    // Posts an asynchronous disconnect event for the object to process.
    void Disconnect();
};

//-----------------------------------------------------------------------------

class Acceptor : public std::enable_shared_from_this< Acceptor >
{
    friend class Hive;

private:
    std::shared_ptr< Hive > m_hive;
    asio::ip::tcp::acceptor m_acceptor;
    asio::io_service::strand m_io_strand;
    asio::basic_waitable_timer<std::chrono::high_resolution_clock> m_timer;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_last_time;
    int32_t m_timer_interval;
    volatile std::atomic<bool> m_error_state;

private:
    Acceptor( const Acceptor & rhs );
    Acceptor & operator =( const Acceptor & rhs );
    void StartTimer();
    void StartError( const asio::error_code & error );
    void DispatchAccept( std::shared_ptr< Connection > connection );
    void HandleTimer( const asio::error_code & error );
    void HandleAccept( const asio::error_code & error,
            std::shared_ptr< Connection > connection );

protected:
    Acceptor( std::shared_ptr< Hive > hive );
    virtual ~Acceptor();

private:
    // Called when a connection has connected to the server. This function
    // should return true to invoke the connection's OnAccept function if the
    // connection will be kept. If the connection will not be kept, the
    // connection's Disconnect function should be called and the function
    // should return false.
    virtual bool OnAccept( std::shared_ptr< Connection > connection,
            const std::string & host, uint16_t port ) = 0;

    // Called on each timer event.
    virtual void OnTimer( const std::chrono::milliseconds & delta ) = 0;

    // Called when an error is encountered. Most typically, this is when the
    // acceptor is being closed via the Stop function or if the Listen is
    // called on an address that is not available.
    virtual void OnError( const asio::error_code & error ) = 0;

public:
    // Returns the Hive object.
    std::shared_ptr< Hive > GetHive();

    // Returns the acceptor object.
    asio::ip::tcp::acceptor & GetAcceptor();

    // Returns the strand object.
    asio::io_service::strand & GetStrand();

    // Sets the timer interval of the object. The interval is changed after
    // the next update is called. The default value is 1000 ms.
    void SetTimerInterval( int32_t timer_interval_ms );

    // Returns the timer interval of the object.
    int32_t GetTimerInterval() const;

    // Returns true if this object has an error associated with it.
    bool HasError();

public:
    // Begin listening on the specific network interface.
    void Listen( const std::string & host, const uint16_t & port );

    // Posts the connection to the listening interface. The next client that
    // connections will be given this connection. If multiple calls to Accept
    // are called at a time, then they are accepted in a FIFO order.
    void Accept( std::shared_ptr< Connection > connection );

    // Stop the Acceptor from listening.
    void Stop();
};

//-----------------------------------------------------------------------------

class Hive : public std::enable_shared_from_this< Hive >
{
private:
    asio::io_service m_io_service;
    std::shared_ptr< asio::io_service::work > m_work_ptr;
    volatile std::atomic<bool> m_shutdown;

private:
    Hive( const Hive & rhs );
    Hive & operator =( const Hive & rhs );

public:
    Hive();
    virtual ~Hive();

    // Returns the io_service of this object.
    asio::io_service & GetService();

    // Returns true if the Stop function has been called.
    bool HasStopped();

    // Polls the networking subsystem once from the current thread and
    // returns.
    void Poll();

    // Runs the networking system on the current thread. This function blocks
    // until the networking system is stopped, so do not call on a single
    // threaded application with no other means of being able to call Stop
    // unless you code in such logic.
    void Run();

    // Stops the networking system. All work is finished and no more
    // networking interactions will be possible afterwards until Reset is called.
    void Stop();

    // Restarts the networking system after Stop as been called. A new work
    // object is created ad the shutdown flag is cleared.
    void Reset();
};

//-----------------------------------------------------------------------------

#endif
