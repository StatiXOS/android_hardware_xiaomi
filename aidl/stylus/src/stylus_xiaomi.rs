// Copyright (C) 2023 StatiXOS
// SPDX-License-Identifer: Apache-2.0
use vendor_xiaomi_hardware_stylus::aidl::vendor::xiaomi::hardware::stylus::{
    IStylusControl::IStylusControl
};

use vendor_xiaomi_hardware_stylus::binder::{
    Interface, Result as BinderResult, StatusCode
};

const STYLUS_CONTROL_PATH: &str = "/sys/touchpanel/pen";
const STYLUS_PEN_MAC: &str = "/sys/class/power_supply/wireless/pen_mac";

// Static file functions
fn set(path: &str, value: String) -> Result<(), StatusCode> {
    std::fs::write(path, value).map_err(|_| StatusCode::PERMISSION_DENIED)
}

fn get(path: &str) -> Option<String> {
    std::fs::read_to_string(path).ok()
}

pub struct XiaomiStylusControl;

impl Interface for XiaomiStylusControl {}

impl IStylusControl for XiaomiStylusControl {

    fn PenControl(&self, enabled: bool) -> BinderResult<bool> {
        set(STYLUS_CONTROL_PATH, if enabled { "1".to_string() } else { "0".to_string() })?;
    }

    fn getPenStatus(&self) -> BinderResult<bool> {
        if let Some(ans) = get(STYLUS_PEN_MAC) {
            Ok(ans != "0")
            self.PenControl(true)
        } else {
            self.PenControl(false)
            Ok(false)
        }
    }
}

