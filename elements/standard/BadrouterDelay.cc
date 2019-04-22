/*
 * BadrouterDelay.{cc,hh} -- element pulls packets from input, delays pushing
 * the packet to output port.
 *
 * Copyright (c) 1999-2001 Massachusetts Institute of Technology
 * Copyright (c) 2002 International Computer Science Institute
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <fstream>
#include <iostream>
#include <string>
#include <ctime>
#include <random>

#include <click/config.h>
#include <click/error.hh>
#include <click/args.hh>
#include <click/glue.hh>
#include "BadrouterDelay.hh"
#include <click/standard/scheduleinfo.hh>

CLICK_DECLS

BadrouterDelay::BadrouterDelay()
    : _p(0), _task(this), _timer(&_task)
{
}

int
BadrouterDelay::configure(Vector<String> &conf, ErrorHandler *errh)
{
	
	std::cout << "Hello, world, from the BadrouterDelay" << std::endl;
	String config_path = "/home/sam/18845/badrouter/badrouter_configuration/config.txt";
	
	// This will be the delay
	double x;
	// This will be the standard deviation
	double y;

	// for (Vector<String>::iterator i = conf.begin(); i != conf.end(); ++i) {
	// 	std::cout << (*i).data() << std::endl;
	// 	std::cout << "Let's gooooo" << std::endl;
	// 	// config_path = pwd + *i;
	// 	// std::cout << config_path << std::endl;
	// }

	// for (int i = 0; i < conf.size(); ++i) {
	// 	std::cout << conf[i].data() << std::endl;
	// }

	int i = Args(conf, this, errh).read("DELAY", _delay).read("STDDEV", _stddev).complete();
	// std::cout << "result of Args" << i << std::endl;
	if (_stddev == 0) {
		std::cout << "No arguments passed in through configuration, using config.txt" << std::endl;
		
		std::ifstream inFile;
		const char *c = config_path.c_str();
		inFile.open(c);
		if (!inFile) {
			std::cerr << "Something's fucked, boys";
			exit(1);
		}

		inFile >> x;
		inFile >> y;
		_delay = Timestamp(x);
		_stddev = y;
	}

	std::cout << "Delay: " << _delay << std::endl;
	std::cout << "STDDEV: " << _stddev << std::endl;
    // return Args(conf, this, errh).read("DELAY", _delay).read("STDDEV", _stddev).read("CONFIG", _config_path).complete();
	return 1;
}

int
BadrouterDelay::initialize(ErrorHandler *errh)
{
    ScheduleInfo::initialize_task(this, &_task, errh);
    _timer.initialize(this);
    _signal = Notifier::upstream_empty_signal(this, 0, &_task);
    return 0;
}

void
BadrouterDelay::cleanup(CleanupStage)
{
    if (_p)
	_p->kill();
}

bool
BadrouterDelay::run_task(Task *)
{
    bool worked = false;
	String config_path = "/home/sam/18845/badrouter/badrouter_configuration/config.txt";
    int x, y;
	static std::default_random_engine generator(time(0));
	std::normal_distribution<double> distribution(3, _stddev);

  retry:
    
    // std::cout << "Just entered retry" << std::endl;

  	this_time = std::clock();

  	// Contrary to official documention of std::clock, this difference appears to be in milliseconds
  	// and not in clock cycles.
  	if ((double)(this_time - last_time) > (3000)) {
  		// Time to check for new parameters
  		std::cout << "Checking for new parameters" << std::endl;
		std::ifstream inFile;
		const char *c = config_path.c_str();
		inFile.open(c);
		if (!inFile) {
			std::cerr << "Something's fucked, boys";
			exit(1);
		}
		
		inFile >> x;
		inFile >> y;

		_delay = Timestamp(x);
		_stddev = y;
		std::normal_distribution<double> distribution(5, _stddev);

		inFile.close();

  		last_time = this_time; 
  	}

  	// Get a new packet and set a timestamp delay seconds from now.
    if (!_p && (_p = input(0).pull())) {

    	// std::cout << "Just got a new packet!!!!!!!!!!!!!!!!!!!!!" << std::endl;

		if (!_p->timestamp_anno().sec()) { // get timestamp if not set 
	    	_p->timestamp_anno().assign_now();
	    }

	    double sample = distribution(generator);

	    std::cout << sample << std::endl;
		_p->timestamp_anno() += Timestamp(sample);

		// at this point put the packet in our buffer
    }

    // Instead of doing if(_P) we should do if(buffer not empty) and then perform this operation
    // for every element in the buffer

    // instead of goto retry when we push something, goto retry 
    if (_p) {

		Timestamp now = Timestamp::now();

		if (_p->timestamp_anno() <= now) {
	    	// packet ready for output
	    	output(0).push(_p);
	    	_p = 0;
	    	worked = true;
	    	goto retry;
		}

		// This section coordinates sending the packet at the scheduled time.
		// If the packet is to be sent very soon (within one Timer::adjustment) 
		// then we just reschedule our task (poll the CPU) until it's time to send.

		// If the delta is large, we set a timer to wake the task up closer to 
		// the scheduled send time.

		Timestamp expiry = _p->timestamp_anno() - Timer::adjustment();
		if (expiry <= now)
	    // small delta, reschedule Task
	    /* Task rescheduled below */;
		else {
	    // large delta, schedule Timer
	    	_timer.schedule_at(expiry);
	    	return false;		// without rescheduling
		}
    } else {
	// no Packet available
		if (!_signal) {
	 	   return false;		// without rescheduling
		}
    }

    _task.fast_reschedule();
    return worked;
}

void
BadrouterDelay::add_handlers()
{
    add_data_handlers("DELAY", Handler::OP_READ | Handler::OP_WRITE, &_delay, true);
    // add_data_handlers("STDDEV", Handler::OP_READ | Handler::OP_WRITE, &_stddev, true);
    add_task_handlers(&_task);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(BadrouterDelay)
