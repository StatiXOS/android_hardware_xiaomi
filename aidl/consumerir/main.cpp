/*
 * Copyright (C) 2021 The Android Open Source Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <aidl/android/hardware/ir/BnConsumerIr.h>
#include <aidl/android/hardware/ir/ConsumerIrFreqRange.h>
#include <android-base/logging.h>
#include <android/binder_interface_utils.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <hardware/consumerir.h>
#include <numeric>

#include <fcntl.h>
#include <linux/lirc.h>

#include <log/log.h>

#include "ir.h"

#define LOG_TAG "XiaomiConsumerIRAIDL"

using ::aidl::android::hardware::ir::ConsumerIrFreqRange;

namespace aidl::android::hardware::ir {

#define LIRC_DEV_PATH "/dev/lirc0"

static const int dutyCycle = 33;

static std::vector<ConsumerIrFreqRange> rangeVec {
    {.min = 30000, .max = 60000},
};

static int openLircDev() {
    int fd = open(LIRC_DEV_PATH, O_RDWR);

    if (fd < 0) {
        LOG(ERROR) << "failed to open " << LIRC_DEV_PATH << ", error " << fd;
    }

    return fd;
}

::ndk::ScopedAStatus ConsumerIr::getCarrierFreqs(std::vector<ConsumerIrFreqRange>* _aidl_return) {

    (*_aidl_return).minHz = static_cast<uint32_t>(rangeVec.min);
    (*_aidl_return).maxHz = static_cast<uint32_t>(rangeVec.max);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ConsumerIr::transmit(int32_t carrierFreqHz,
                                          const std::vector<int32_t>& pattern) {
    size_t entries = pattern.size();
    int rc;
    int lircFd;

    lircFd = openLircDev();
    if (lircFd < 0) {
        return lircFd;
    }

    rc = ioctl(lircFd, LIRC_SET_SEND_CARRIER, &carrierFreq);
    if (rc < 0) {
        LOG(ERROR) << "failed to set carrier " << carrierFreq << ", error: " << errno;
        goto out_close;
    }

    rc = ioctl(lircFd, LIRC_SET_SEND_DUTY_CYCLE, &dutyCycle);
    if (rc < 0) {
        LOG(ERROR) << "failed to set duty cycle " << dutyCycle << ", error: " << errno;
        goto out_close;
    }

    if ((entries & 1) != 0) {
        rc = write(lircFd, pattern.data(), sizeof(int32_t) * entries);
    } else {
        rc = write(lircFd, pattern.data(), sizeof(int32_t) * (entries - 1));
        usleep(pattern[entries - 1]);
    }

    if (rc < 0) {
        LOG(ERROR) << "failed to write pattern " << pattern.size() << ", error: " << errno;
        goto out_close;
    }

    rc = 0;

out_close:
    close(lircFd);

    return rc == 0;
}

}  // namespace aidl::android::hardware::ir

using aidl::android::hardware::ir::ConsumerIr;

int main() {
    auto binder = ::ndk::SharedRefBase::make<ConsumerIr>();
    const std::string name = std::string() + ConsumerIr::descriptor + "/default";
    CHECK_EQ(STATUS_OK, AServiceManager_addService(binder->asBinder().get(), name.c_str()))
            << "Failed to register " << name;

    ABinderProcess_setThreadPoolMaxThreadCount(0);
    ABinderProcess_joinThreadPool();

    return EXIT_FAILURE;  // should not reached
}
