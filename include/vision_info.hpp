#pragma once

class VisionInfo {
    public:
        explicit VisionInfo(int max_traceback);
        void update(bool visited);
        int last_visited() const;

    private:
        int last_visited_ = -1;
        int max_traceback_;
};
