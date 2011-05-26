#ifndef __REPLICATION_HEARTBEAT_MANAGER_HPP__
#define	__REPLICATION_HEARTBEAT_MANAGER_HPP__

#include "arch/timing.hpp"

/* The heartbeat sender periodically calls an on_heartbeat_send method once activated.
 Tt behaves very much like a repeating_timer_t, except that you can turn it on
 and off.
 (also we cannot really use a repeating timer because it has additional
 event queue roundtrips which destroy synchronization with stop_sending_heartbeats().) */

class heartbeat_sender_t {
protected:
    heartbeat_sender_t(int heartbeat_frequency_ms);
    virtual ~heartbeat_sender_t();

    virtual void send_heartbeat() = 0;

    void start_sending_heartbeats();
    void stop_sending_heartbeats();
    
private:
    void send_heartbeat_wrapper() {
        send_heartbeat();
    }

    const int heartbeat_frequency_ms_;
    timer_token_t *heartbeat_timer_;
    bool continue_firing;
    static void send_heartbeat_callback(void *data);
};

/* heartbeat_receiver_t implements the following functionality:
 once watch_heartbeat() has been called, on_heartbeat_timeout is invoked as soon
 as more than heartbeat_timeout_ms_ have passed without any call to note_heartbeat().
 */

class heartbeat_receiver_t {
public:
    void note_heartbeat();
    
protected:
    heartbeat_receiver_t(int heartbeat_timeout_ms);
    virtual ~heartbeat_receiver_t();

    void watch_heartbeat();
    void unwatch_heartbeat();
    virtual void on_heartbeat_timeout() = 0;

private:
    void on_heartbeat_timeout_wrapper() {
        on_heartbeat_timeout();
    }

    int heartbeat_timeout_ms_;
    timer_token_t *heartbeat_timer_;
    static void heartbeat_timeout_callback(void *data);
};

#endif	/* __REPLICATION_HEARTBEAT_MANAGER_HPP__ */