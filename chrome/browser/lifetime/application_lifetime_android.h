// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_LIFETIME_APPLICATION_LIFETIME_ANDROID_H_
#define CHROME_BROWSER_LIFETIME_APPLICATION_LIFETIME_ANDROID_H_

#include "base/android/jni_android.h"

namespace browser {

void TerminateAndroid();

bool RegisterApplicationLifetimeAndroid(JNIEnv* env);

}  // namespace browser

#endif  // CHROME_BROWSER_LIFETIME_APPLICATION_LIFETIME_ANDROID_H_
