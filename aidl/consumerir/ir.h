/*
 * Copyright (C) 2021 The Android Open Source Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android/binder_interface_utils.h>

namespace aidl {
namespace android {
namespace hardware {
namespace ir {

class ConsumerIr : public BnConsumerIr {
  private:
    ::ndk::ScopedAStatus getCarrierFreqs(std::vector<ConsumerIrFreqRange>* _aidl_return) override;
    ::ndk::ScopedAStatus transmit(int32_t carrierFreqHz,
                                  const std::vector<int32_t>& pattern) override;
};

} // namespace ir
} // namespace hardware
} // namespace android
} // namespace aidl
