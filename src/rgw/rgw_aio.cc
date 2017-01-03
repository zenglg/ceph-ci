// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2017 Red Hat, Inc.
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */

#include "include/rados/librados.hpp"
#include "librados/AioCompletionImpl.h"

/* XXX un-modular AioCompletion extension (Kraken-only, pre-deprecated) */

using namespace librados;

namespace rgw {

  void aiocompletion_exref(AioCompletion* c)
  {
    c->pc->get();
  }

  void aiocompletion_unref(AioCompletion* c)
  {
    c->pc->put();
  }

} /* namespace rgw */
