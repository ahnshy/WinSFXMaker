/*
 * Copyright 2002, LifeLine Networks BV (www.lifeline.nl). All rights reserved.
 * Copyright 2002, Bastiaan Bakker. All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#if !defined(h_ebd0ee89_622d_4af1_9a9d_d0e057debe86)
#define h_ebd0ee89_622d_4af1_9a9d_d0e057debe86

#include <../include/LayoutAppender.hh>
#include <../include/TriggeringEventEvaluator.hh>
#include <list>
#include <memory>

namespace log4cpp
{
   class LOG4CPP_EXPORT BufferingAppender : public LayoutAppender 
   {
      public:
         BufferingAppender(const std::string name, unsigned long max_size, std::LOG4CPP_UNIQUE_PTR<Appender> sink,
                           std::LOG4CPP_UNIQUE_PTR<TriggeringEventEvaluator> evaluator);
      
         virtual void close() { sink_->close(); }
         
         bool getLossy() const { return lossy_; }
         void setLossy(bool lossy) { lossy_ = lossy; }

      protected:
         virtual void _append(const LoggingEvent& event);

      private:
         typedef std::list<LoggingEvent> queue_t;
         
         queue_t queue_;
         unsigned long max_size_;
         std::LOG4CPP_UNIQUE_PTR<Appender> sink_;
         std::LOG4CPP_UNIQUE_PTR<TriggeringEventEvaluator> evaluator_;
         bool lossy_;

         void dump();
   };
}

#endif // h_ebd0ee89_622d_4af1_9a9d_d0e057debe86
