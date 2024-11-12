/*
 * Copyright 2002, Log4cpp Project. All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include <../include/PassThroughLayout.hh>
#include <../include/FactoryParams.hh>
#include <memory>

namespace log4cpp
{
   std::LOG4CPP_UNIQUE_PTR<Layout> create_pass_through_layout(const FactoryParams& params)
   {
      return std::LOG4CPP_UNIQUE_PTR<Layout>(new PassThroughLayout);
   }
}
