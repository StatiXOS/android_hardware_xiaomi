/*
 * Copyright (C) 2019 The Android Open Source Project
 * Copyright (C) 2023 StatiXOS
 * SPDX-License-Identifer: Apache-2.0
 */

#include "vibrator-impl/Vibrator.h"

#include <android-base/logging.h>
#include <thread>

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

static constexpr char HAPTIC_NODES[] = { 
    "/sys/class/leds/vibrator/",
    "/sys/bus/i2c/drivers/aw8697_haptic/0-005a/",
    "/sys/bus/i2c/drivers/aw8697_haptic/1-005a/",
    "/sys/bus/i2c/drivers/aw8697_haptic/2-005a/",
    "/sys/bus/i2c/drivers/aw8697_haptic/3-005a/",
    "/sys/bus/i2c/drivers/awinic_haptic/0-005a/",
    "/sys/bus/i2c/drivers/awinic_haptic/1-005a/",
    "/sys/bus/i2c/drivers/awinic_haptic/2-005a/",
    "/sys/bus/i2c/drivers/awinic_haptic/3-005a/",
    "/sys/bus/i2c/drivers/aw86927_haptic/0-005a/",
    "/sys/bus/i2c/drivers/aw86927_haptic/1-005a/",
    "/sys/bus/i2c/drivers/aw86927_haptic/2-005a/",
    "/sys/bus/i2c/drivers/aw86927_haptic/3-005a/",
};

static char HAPTIC_NODE[];

// Common haptic nodes
static constexpr char ACTIVATE_NODE[] = "activate";
static constexpr char INDEX_NODE[] = "index";
static constexpr char DURATION_NODE[] = "duration";

// Waveform definitions
static constexpr uint32_t WAVEFORM_TICK_EFFECT_MS = 10;
static constexpr uint32_t WAVEFORM_TEXTURE_TICK_EFFECT_MS = 20;
static constexpr uint32_t WAVEFORM_CLICK_EFFECT_MS = 15;
static constexpr uint32_t WAVEFORM_HEAVY_CLICK_EFFECT_MS = 30;
static constexpr uint32_t WAVEFORM_DOUBLE_CLICK_EFFECT_MS = 60;
static constexpr uint32_t WAVEFORM_THUD_EFFECT_MS = 35;
static constexpr uint32_t WAVEFORM_POP_EFFECT_MS = 15;

// Select waveform index from firmware through index list
static constexpr uint32_t WAVEFORM_TICK_EFFECT_INDEX = 1;
static constexpr uint32_t WAVEFORM_TEXTURE_TICK_EFFECT_INDEX = 4;
static constexpr uint32_t WAVEFORM_CLICK_EFFECT_INDEX = 2;
static constexpr uint32_t WAVEFORM_HEAVY_CLICK_EFFECT_INDEX = 5;
static constexpr uint32_t WAVEFORM_DOUBLE_CLICK_EFFECT_INDEX = 6;
static constexpr uint32_t WAVEFORM_THUD_EFFECT_INDEX = 7;

template <typename T>
static void write_haptic_node(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value;
}

ndk::ScopedAStatus VibratorManager::getVibrator(std::shared_ptr<IVibrator>* _aidl_return) {
    char triggerNode[];

    for (int i = 0; i < (sizeof(HAPTIC_NODES) + 1); i++) {
        triggerNode = HAPTIC_NODE[i] + ACTIVATE_NODE;
        if (!open(triggerNode, std::ios_base::in))
            continue;
        else
            HAPTIC_NODE = HAPTIC_NODES[i];
            break;
    }

    LOG(INFO) << "Vibrator manager getting vibrator " << HAPTIC_NODE;

    *_aidl_return = mDefaultVibrator;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getCapabilities(int32_t* _aidl_return) {
    LOG(VERBOSE) << "Vibrator reporting capabilities";
    *_aidl_return = IVibrator::CAP_ON_CALLBACK | IVibrator::CAP_PERFORM_CALLBACK;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::off() {
    LOG(VERBOSE) << "Vibrator off";
    /* Reset index before triggering another set of haptics */
    write_haptic_node(HAPTIC_NODE + INDEX_NODE, 0);
    write_haptic_node(HAPTIC_NODE + ACTIVATE_NODE, 0);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::on(int32_t timeoutMs,
                                const std::shared_ptr<IVibratorCallback>& callback) {
    write_haptic_node(HAPTIC_NODE + DURATION_NODE, timeoutMs);
    write_haptic_node(HAPTIC_NODE + ACTIVATE_NODE, 1);

    if (callback != nullptr) {
        // Note that thread lambdas aren't using implicit capture [=], to avoid capturing "this",
        // which may be asynchronously destructed.
        // If "this" is needed, use [sharedThis = this->ref<Vibrator>()].
        std::thread([timeoutMs, callback] {
            usleep(timeoutMs * 1000);
        }).detach();
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::perform(Effect effect, EffectStrength strength,
                                     const std::shared_ptr<IVibratorCallback>& callback,
                                     int32_t* _aidl_return) {
    uint32_t index = 0;
    uint32_t timeMs = 0;

    LOG(INFO) << "Vibrator perform";

    switch (effect) {
        case Effect::TICK:
            LOG(INFO) << "Vibrator effect set to TICK";
            index = WAVEFORM_TICK_EFFECT_INDEX;
            timeMs = WAVEFORM_TICK_EFFECT_MS;
            break;
        case Effect::TEXTURE_TICK:
            LOG(INFO) << "Vibrator effect set to TEXTURE_TICK";
            index = WAVEFORM_TEXTURE_TICK_EFFECT_INDEX;
            timeMs = WAVEFORM_TEXTURE_TICK_EFFECT_MS;
            break;
        case Effect::CLICK:
            LOG(INFO) << "Vibrator effect set to CLICK";
            index = WAVEFORM_CLICK_EFFECT_INDEX;
            timeMs = WAVEFORM_CLICK_EFFECT_MS;
            break;
        case Effect::HEAVY_CLICK:
            LOG(INFO) << "Vibrator effect set to HEAVY_CLICK";
            index = WAVEFORM_HEAVY_CLICK_EFFECT_INDEX;
            timeMs = WAVEFORM_HEAVY_CLICK_EFFECT_MS;
            break;
        case Effect::DOUBLE_CLICK:
            LOG(INFO) << "Vibrator effect set to DOUBLE_CLICK";
            index = WAVEFORM_DOUBLE_CLICK_EFFECT_INDEX;
            timeMs = WAVEFORM_DOUBLE_CLICK_EFFECT_MS;
            break;
        case Effect::THUD:
            LOG(INFO) << "Vibrator effect set to THUD";
            index = WAVEFORM_THUD_EFFECT_INDEX;
            timeMs = WAVEFORM_THUD_EFFECT_MS;
            break;
        case Effect::POP:
            LOG(INFO) << "Vibrator effect set to POP";
            index = WAVEFORM_TICK_EFFECT_INDEX;
            timeMs = WAVEFORM_POP_EFFECT_MS;
            break;
        default:
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    /* Setup effect index */
    write_haptic_node(index_node, index);

    if (callback != nullptr) {
        std::thread([callback] {
            usleep(timeMs * 1000);
            callback->onComplete();
        }).detach();
    }
    
    status = on(timeMs, nullptr);
    if (!status.isOk()) {
        return status;
    } else {
        *_aidl_return = timeMs;
        return status.isOk();
    }
}

ndk::ScopedAStatus Vibrator::getSupportedEffects(std::vector<Effect>* _aidl_return) {
    *_aidl_return = {
        Effect::TICK,
        Effect::TEXTURE_TICK,
        Effect::CLICK,
        Effect::HEAVY_CLICK,
        Effect::DOUBLE_CLICK,
        Effect::THUD,
        Effect::POP
    };

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setAmplitude(float amplitude) {
    LOG(VERBOSE) << "Vibrator set amplitude: " << amplitude;
    if (amplitude <= 0.0f || amplitude > 1.0f) {
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_ILLEGAL_ARGUMENT));
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setExternalControl(bool enabled) {
    LOG(VERBOSE) << "Vibrator set external control: " << enabled;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getCompositionDelayMax(int32_t* maxDelayMs) {
    *maxDelayMs = COMPOSE_DELAY_MAX_MS;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getCompositionSizeMax(int32_t* maxSize) {
    *maxSize = COMPOSE_SIZE_MAX;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getSupportedPrimitives(std::vector<CompositePrimitive>* supported) {
    *supported = {
            CompositePrimitive::NOOP,       CompositePrimitive::CLICK,
            CompositePrimitive::THUD,       CompositePrimitive::SPIN,
            CompositePrimitive::QUICK_RISE, CompositePrimitive::SLOW_RISE,
            CompositePrimitive::QUICK_FALL, CompositePrimitive::LIGHT_TICK,
            CompositePrimitive::LOW_TICK,
    };
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getPrimitiveDuration(CompositePrimitive primitive,
                                                  int32_t* durationMs) {
    std::vector<CompositePrimitive> supported;
    getSupportedPrimitives(&supported);
    if (std::find(supported.begin(), supported.end(), primitive) == supported.end()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    if (primitive != CompositePrimitive::NOOP) {
        *durationMs = 100;
    } else {
        *durationMs = 0;
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::compose(const std::vector<CompositeEffect>& composite,
                                     const std::shared_ptr<IVibratorCallback>& callback) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getSupportedAlwaysOnEffects(std::vector<Effect>* _aidl_return) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::alwaysOnEnable(int32_t id, Effect effect, EffectStrength strength) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::alwaysOnDisable(int32_t id) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getResonantFrequency(float *resonantFreqHz) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getQFactor(float *qFactor) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getFrequencyResolution(float *freqResolutionHz) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getFrequencyMinimum(float *freqMinimumHz) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getBandwidthAmplitudeMap(std::vector<float> *_aidl_return) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getPwlePrimitiveDurationMax(int32_t *durationMs) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getPwleCompositionSizeMax(int32_t *maxSize) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getSupportedBraking(std::vector<Braking> *supported) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

void resetPreviousEndAmplitudeEndFrequency(float &prevEndAmplitude, float &prevEndFrequency) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::composePwle(const std::vector<PrimitivePwle> &composite,
                                         const std::shared_ptr<IVibratorCallback> &callback) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
