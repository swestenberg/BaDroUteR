#ifndef CLICK_BADROUTERDUPLICATE_HH
#define CLICK_BADROUTERDUPLICATE_HH
#include <click/element.hh>
#include <click/task.hh>
#include <click/timer.hh>
#include <click/notifier.hh>
CLICK_DECLS

/*
=c

BadrouterDuplicate(DELAY)

=s shaping

delay-inducing pull-to-push converter

=d

Pulls packets from the single input port. Delays them for at least DELAY
seconds, with microsecond precision. A packet with timestamp T will be emitted
no earlier than time (T + DELAY). On output, the packet's timestamp is set to
the delayed time.

BadrouterDuplicate listens for upstream notification, such as that available from
Queue.

=h delay read/write

Return or set the DELAY parameter.

=a Queue, Unqueue, RatedUnqueue, BandwidthRatedUnqueue, LinkUnqueue,
DelayShaper, SetTimestamp */
#include <ctime>

class BadrouterDuplicate : public Element { public:

    BadrouterDuplicate() CLICK_COLD;

    const char *class_name() const	{ return "BadrouterDuplicate"; }
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

    double _duplicate_pct = 1;
    Packet *_p;
    Task _task;
    NotifierSignal _signal;

};

CLICK_ENDDECLS
#endif
