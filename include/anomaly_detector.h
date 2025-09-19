_Pragma("once")
#include <vector>
#include "record.h"

class AnomalyDetector {
public:
    std::vector<size_t> detect(const std::vector<Record>& records, double contamination);
};

