#ifndef CLICK_BADROUTERDELAY_HH
#define CLICK_BADROUTERDELAY_HH
#include <click/element.hh>
#include <click/task.hh>
#include <click/timer.hh>
#include <click/notifier.hh>
#include <vector>
CLICK_DECLS

/*
=c

BadrouterDelay(DELAY)

=s shaping

delay-inducing pull-to-push converter

=d

Pulls packets from the single input port. Delays them for at least DELAY
seconds, with microsecond precision. A packet with timestamp T will be emitted
no earlier than time (T + DELAY). On output, the packet's timestamp is set to
the delayed time.

BadrouterDelay listens for upstream notification, such as that available from
Queue.

=h delay read/write

Return or set the DELAY parameter.

=a Queue, Unqueue, RatedUnqueue, BandwidthRatedUnqueue, LinkUnqueue,
DelayShaper, SetTimestamp */
#include <ctime>

class BadrouterDelay : public Element { public:

    BadrouterDelay() CLICK_COLD;

    const char *class_name() const	{ return "BadrouterDelay"; }
    const char *port_count() const	{ return PORTS_1_1; }
    const char *processing() const	{ return PULL_TO_PUSH; }

    int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
    int initialize(ErrorHandler *) CLICK_COLD;
    void cleanup(CleanupStage) CLICK_COLD;
    void add_handlers() CLICK_COLD;

    bool run_task(Task *);

  private:

    clock_t this_time = std::clock();
    clock_t last_time = this_time;

    Packet *_p;
    Timestamp _delay;
    double _stddev;
    std::vector<Packet*> _delayed_packets;


    String _delay_string;
    String _stddev_string;
    String _config_path;
    Task _task;
    Timer _timer;
    NotifierSignal _signal;

};

CLICK_ENDDECLS
#endif
