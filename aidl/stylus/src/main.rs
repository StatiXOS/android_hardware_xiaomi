// Copyright (C) 2024 StatiXOS
// SPDX-License-Identifer: Apache-2.0

//! Implementation of vendor.xiaomi.hardware.stylus-V1 for Xiaomi devices.

mod stylus_xiaomi;

use stylus_xiaomi::XiaomiStylusControl;

use vendor_xiaomi_hardware_stylus::aidl::vendor::xiaomi::hardware::stylus::{
    IStylusControl::BnStylusControl
};

use vendor_xiaomi_hardware_stylus::binder::BinderFeatures;

static HAL_SERVICE_NAME: &str = "vendor.xiaomi.hardware.stylus.IStylusControl/default";

fn main() {
    let stylus_binder = BnStylusControl::new_binder(
        XiaomiStylusControl,
        BinderFeatures::default(),
    );
    binder::add_service(HAL_SERVICE_NAME, stylus_binder.as_binder())
        .expect("Failed to register service?");
    // Does not return - spawn or perform any work you mean to do before this call.
    binder::ProcessState::join_thread_pool()
}

