/*
 * BadrouterCounter.{cc,hh} -- element counts packets, measures packet rate
 * Eddie Kohler
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2008 Regents of the University of California
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
#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>

#include <click/config.h>
#include <click/error.hh>
#include <click/confparse.hh>
#include <click/args.hh>
#include <click/handlercall.hh>

#include "BadrouterCounter.hh"

CLICK_DECLS

BadrouterCounter::BadrouterCounter()
  : _count_trigger_h(0), _byte_trigger_h(0)
{
}

BadrouterCounter::~BadrouterCounter()
{
  delete _count_trigger_h;
  delete _byte_trigger_h;
}

void
BadrouterCounter::reset()
{
  _count = _byte_count = 0;
  _count_triggered = _byte_triggered = false;
}

int
BadrouterCounter::configure(Vector<String> &conf, ErrorHandler *errh)
{
  Args(conf, this, errh).read("LOG_ID", _log_id).complete();

  _count_trigger = (counter_t)(-1);

  _byte_trigger = (counter_t)(-1);

  return 0;
}

int
BadrouterCounter::initialize(ErrorHandler *errh)
{
  if (_count_trigger_h && _count_trigger_h->initialize_write(this, errh) < 0)
    return -1;
  if (_byte_trigger_h && _byte_trigger_h->initialize_write(this, errh) < 0)
    return -1;
  reset();
  return 0;
}

Packet *
BadrouterCounter::simple_action(Packet *p)
{
    _count++;
    _byte_count += p->length();
    _rate.update(1);
    _byte_rate.update(p->length());

  if (_count == _count_trigger && !_count_triggered) {
    _count_triggered = true;
    if (_count_trigger_h)
      (void) _count_trigger_h->call_write();
  }
  if (_byte_count >= _byte_trigger && !_byte_triggered) {
    _byte_triggered = true;
    if (_byte_trigger_h)
      (void) _byte_trigger_h->call_write();
  }

  long unsigned int print_count = 10000;

  if (_byte_count % print_count == 0) {
    long double elapsed = (long double)(std::clock() - _sample_start_time) / ((long double)CLOCKS_PER_SEC);
    long double sample_byte_rate = (((long double)(print_count)) / elapsed);
    printf("Counter %d byte rate (Bps): %Lf\n", _log_id, sample_byte_rate);
    printf("time elapsed (seconds): %Lf\n", elapsed);
    _sample_start_time = std::clock();
    _prev_byte_count = _byte_count;

    _data.push_back(sample_byte_rate);

    if (_byte_count % (print_count*100) == 0) {
      String results_counterA = "/home/sam/18845/badrouter/results_counterA.txt";
      String results_counterB = "/home/sam/18845/badrouter/results_counterB.txt";

      std::ofstream outFile;
      const char *c;
      if (_log_id == 0) { c = results_counterA.c_str(); }
      else { c = results_counterB.c_str(); }
      outFile.open(c, std::ios_base::app);
      if (!outFile) {
        std::cerr << "Something's fucked, boys" << std::endl;
        exit(1);
      }

      for(double data : _data) {
        outFile << data << std::endl;
      }

      _data.clear();
      // outFile << elapsed << ", " << sample_byte_rate << std::endl;
      outFile.close();
    }
  }

  return p;
}


enum { H_COUNT, H_BYTE_COUNT, H_RATE, H_BIT_RATE, H_BYTE_RATE, H_RESET,
       H_COUNT_CALL, H_BYTE_COUNT_CALL };

String
BadrouterCounter::read_handler(Element *e, void *thunk)
{
   BadrouterCounter *c = (BadrouterCounter *)e;
    switch ((intptr_t)thunk) {
      case H_COUNT:
	return String(c->_count);
      case H_BYTE_COUNT:
	return String(c->_byte_count);
      case H_RATE:
	c->_rate.update(0);	// drop rate after idle period
	return c->_rate.unparse_rate();
      case H_BIT_RATE:
	c->_byte_rate.update(0); // drop rate after idle period
	// avoid integer overflow by adjusting scale factor instead of
	// multiplying
	if (c->_byte_rate.scale() >= 3)
	    return cp_unparse_real2(c->_byte_rate.scaled_average() * c->_byte_rate.epoch_frequency(), c->_byte_rate.scale() - 3);
	else
	    return cp_unparse_real2(c->_byte_rate.scaled_average() * c->_byte_rate.epoch_frequency() * 8, c->_byte_rate.scale());
      case H_BYTE_RATE:
	c->_byte_rate.update(0); // drop rate after idle period
	return c->_byte_rate.unparse_rate();
      case H_COUNT_CALL:
	if (c->_count_trigger_h)
	    return String(c->_count_trigger);
	else
	    return String();
      default:
	return "<error>";
    }
}

int
BadrouterCounter::write_handler(const String &in_str, Element *e, void *thunk, ErrorHandler *errh)
{
   BadrouterCounter *c = (BadrouterCounter *)e;
    String str = in_str;
    switch ((intptr_t)thunk) {
      case H_COUNT_CALL:
	  if (!IntArg().parse(cp_shift_spacevec(str), c->_count_trigger))
	    return errh->error("'count_call' first word should be unsigned (count)");
	if (HandlerCall::reset_write(c->_count_trigger_h, str, c, errh) < 0)
	    return -1;
	c->_count_triggered = false;
	return 0;
      case H_BYTE_COUNT_CALL:
	  if (!IntArg().parse(cp_shift_spacevec(str), c->_byte_trigger))
	    return errh->error("'byte_count_call' first word should be unsigned (count)");
	if (HandlerCall::reset_write(c->_byte_trigger_h, str, c, errh) < 0)
	    return -1;
	c->_byte_triggered = false;
	return 0;
      case H_RESET:
	c->reset();
	return 0;
      default:
	return errh->error("<internal>");
    }
}

void
BadrouterCounter::add_handlers()
{
    add_read_handler("count", read_handler, H_COUNT);
    add_read_handler("byte_count", read_handler, H_BYTE_COUNT);
    add_read_handler("rate", read_handler, H_RATE);
    add_read_handler("bit_rate", read_handler, H_BIT_RATE);
    add_read_handler("byte_rate", read_handler, H_BYTE_RATE);
    add_write_handler("reset", write_handler, H_RESET, Handler::f_button);
    add_write_handler("reset_counts", write_handler, H_RESET, Handler::f_button | Handler::f_uncommon);
    add_read_handler("count_call", read_handler, H_COUNT_CALL);
    add_write_handler("count_call", write_handler, H_COUNT_CALL);
    add_write_handler("byte_count_call", write_handler, H_BYTE_COUNT_CALL);
}

int
BadrouterCounter::llrpc(unsigned command, void *data)
{
  if (command == CLICK_LLRPC_GET_RATE) {
    uint32_t *val = reinterpret_cast<uint32_t *>(data);
    if (*val != 0)
      return -EINVAL;
    _rate.update(0);		// drop rate after idle period
    *val = _rate.rate();
    return 0;

  } else if (command == CLICK_LLRPC_GET_COUNT) {
    uint32_t *val = reinterpret_cast<uint32_t *>(data);
    if (*val != 0 && *val != 1)
      return -EINVAL;
    *val = (*val == 0 ? _count : _byte_count);
    return 0;

  } else if (command == CLICK_LLRPC_GET_COUNTS) {
    click_llrpc_counts_st *user_cs = (click_llrpc_counts_st *)data;
    click_llrpc_counts_st cs;
    if (CLICK_LLRPC_GET_DATA(&cs, data, sizeof(cs.n) + sizeof(cs.keys)) < 0
	|| cs.n >= CLICK_LLRPC_COUNTS_SIZE)
      return -EINVAL;
    for (unsigned i = 0; i < cs.n; i++) {
      if (cs.keys[i] == 0)
	cs.values[i] = _count;
      else if (cs.keys[i] == 1)
	cs.values[i] = _byte_count;
      else
	return -EINVAL;
    }
    return CLICK_LLRPC_PUT_DATA(&user_cs->values, &cs.values, sizeof(cs.values));

  } else
    return Element::llrpc(command, data);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(BadrouterCounter)
