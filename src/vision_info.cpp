#include "vision_info.hpp"

namespace moving_robot {
    VisionInfo::VisionInfo(int max_traceback): max_traceback_{max_traceback} {}

    void VisionInfo::update(bool visited) {
        if (visited) {
            last_visited_ = 0;
        } else if (last_visited_ >= 0) {
            ++last_visited_;
        }

        if (last_visited_ >= max_traceback_) {
            last_visited_ = -1;
        }
    }

    int VisionInfo::last_visited() const {
        return last_visited_;
    }
}
