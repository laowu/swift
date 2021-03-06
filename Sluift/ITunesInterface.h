/*
 * Copyright (c) 2014-2016 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#pragma once

#include <memory>
#include <string>

#include <boost/optional/optional_fwd.hpp>

#include <Swiften/Base/API.h>

namespace Swift {
    class SWIFTEN_API ITunesInterface {
        public:
            struct Track {
                std::string name;
                std::string artist;
                std::string album;
                long trackNumber;
                double duration;
                long rating;
            };

            ITunesInterface();
            virtual ~ITunesInterface();

            boost::optional<Track> getCurrentTrack() const;

        private:
            bool haveApplication() const;

        private:
            struct Private;
            std::shared_ptr<Private> p;
    };
}
