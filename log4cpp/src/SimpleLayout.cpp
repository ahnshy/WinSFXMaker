/*
 * SimpleLayout.cpp
 *
 * Copyright 2000, LifeLine Networks BV (www.lifeline.nl). All rights reserved.
 * Copyright 2000, Bastiaan Bakker. All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include "PortabilityImpl.hh"
#include <../include/SimpleLayout.hh>
#include <../include/Priority.hh>
#include <ios>
#ifdef LOG4CPP_HAVE_SSTREAM
#include <sstream>
#endif

#include <memory>
#include <../include/FactoryParams.hh>

namespace log4cpp {

    SimpleLayout::SimpleLayout() {
    }
    
    SimpleLayout::~SimpleLayout() {
    }

    std::string SimpleLayout::format(const LoggingEvent& event) {
        std::ostringstream message;

        const std::string& priorityName = Priority::getPriorityName(event.priority);
		message.width(Priority::MESSAGE_SIZE);message.setf(std::ios::left);
		message << priorityName << ": " << event.message << std::endl;
        return message.str();
    }

   std::LOG4CPP_UNIQUE_PTR<Layout> create_simple_layout(const FactoryParams& params)
   {
      return std::LOG4CPP_UNIQUE_PTR<Layout>(new SimpleLayout);
   }
}
