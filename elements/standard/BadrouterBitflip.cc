/*
 * BadrouterBitflip.{cc,hh} -- element pulls packets from input and pushes packet
 * with bits flipped according to given probability.
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
#include "BadrouterBitflip.hh"
#include <click/standard/scheduleinfo.hh>

CLICK_DECLS

BadrouterBitflip::BadrouterBitflip()
    : _task(this)
{
}

int
BadrouterBitflip::configure(Vector<String> &conf, ErrorHandler *errh)
{
  double flip_prob = 0;
  int i = Args(conf, this, errh).read("PROB", flip_prob).complete();
  _flip_ppm = flip_prob * 1000000;
  return i;
}

int
BadrouterBitflip::initialize(ErrorHandler *errh)
{
    ScheduleInfo::initialize_task(this, &_task, errh);
    _signal = Notifier::upstream_empty_signal(this, 0, &_task);
    return 0;
}

void
BadrouterBitflip::cleanup(CleanupStage)
{
}

bool
BadrouterBitflip::run_task(Task *)
{
  bool worked = false;
  if ((_p = input(0).pull())) {
    _wp = _p->uniqueify();

    for (int byte = 0; byte < _wp->length(); ++byte)
      for (int bit = 0; bit < _wp->length() * 8; ++bit)
        if ((rand() % 1000000) < _flip_ppm)
          _wp->data()[byte] = _wp->data()[byte] ^ (1 << bit);

      output(0).push(_wp);

    worked = true;
  }

  if (!_signal)
    return false;

  _task.fast_reschedule();
  return worked;
}

void
BadrouterBitflip::add_handlers()
{
}

CLICK_ENDDECLS
EXPORT_ELEMENT(BadrouterBitflip)
