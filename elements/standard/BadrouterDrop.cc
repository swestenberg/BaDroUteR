/*
 * BadrouterDrop.{cc,hh} -- element pulls packets from input and pushes packet to
 * output zero or one times.
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
#include "BadrouterDrop.hh"
#include <click/standard/scheduleinfo.hh>

CLICK_DECLS

BadrouterDrop::BadrouterDrop()
    : _task(this)
{
}

int
BadrouterDrop::configure(Vector<String> &conf, ErrorHandler *errh)
{
  double drop_prob = 0;
  int i = Args(conf, this, errh).read("PROB", drop_prob).complete();
  _drop_pct = drop_prob * 100;
  return i;
}

int
BadrouterDrop::initialize(ErrorHandler *errh)
{
    ScheduleInfo::initialize_task(this, &_task, errh);
    _signal = Notifier::upstream_empty_signal(this, 0, &_task);
    return 0;
}

void
BadrouterDrop::cleanup(CleanupStage)
{
}

bool
BadrouterDrop::run_task(Task *)
{
  bool worked = false;
  if ((_p = input(0).pull())) {
    if ((rand() % 100) >= _drop_pct)
      output(0).push(_p);
    else
      _p->kill();
    worked = true;
  }

  if (!_signal)
    return false;

  _task.fast_reschedule();
  return worked;
}

void
BadrouterDrop::add_handlers()
{
}

CLICK_ENDDECLS
EXPORT_ELEMENT(BadrouterDrop)
