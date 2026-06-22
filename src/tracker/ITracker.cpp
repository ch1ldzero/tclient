#include "tracker/ITracker.hpp"

namespace tclient {

uint32_t ITracker::GetInterval() const {
    return 0;
}

uint32_t ITracker::GetSeeders() const {
    return 0;
}

uint32_t ITracker::GetLeechers() const {
    return 0;
}

} // namespace tclient

