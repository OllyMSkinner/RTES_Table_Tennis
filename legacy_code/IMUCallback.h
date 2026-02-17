#ifndef IMUCALLBACK_H
#define IMUCALLBACK_H
#include "IMUSample.h"

class IMUCallback {
public:
    virtual void hasSample(IMUSample& sample) = 0;
};

#endif
