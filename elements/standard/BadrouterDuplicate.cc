/*
 * BadrouterDuplicate.{cc,hh} -- element pulls packets from input and pushes packet
 * once or twice to the output port, depending on given probability.
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
#include "BadrouterDuplicate.hh"
#include <click/standard/scheduleinfo.hh>

CLICK_DECLS

BadrouterDuplicate::BadrouterDuplicate()
    : _task(this)
{
}

int
BadrouterDuplicate::configure(Vector<String> &conf, ErrorHandler *errh)
{
	return Args(conf, this, errh).read("PROB", _duplicate_prob).complete();
}

int
BadrouterDuplicate::initialize(ErrorHandler *errh)
{
    ScheduleInfo::initialize_task(this, &_task, errh);
    _signal = Notifier::upstream_empty_signal(this, 0, &_task);
    return 0;
}

void
BadrouterDuplicate::cleanup(CleanupStage)
{
}

bool
BadrouterDuplicate::run_task(Task *)
{
  bool worked = false;
  if ((_p = input(0).pull())) {
    output(0).push(_p);

    if ((rand() % 100) < (_duplicate_prob * 100))
      output(0).push(_p);

    worked = true;
  }

  if (!_signal)
    return false;

  _task.fast_reschedule();
  return worked;
}

void
BadrouterDuplicate::add_handlers()
{
}

CLICK_ENDDECLS
EXPORT_ELEMENT(BadrouterDuplicate)
