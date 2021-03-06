// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_PLUGIN_SERVICE_FILTER_H_
#define CONTENT_PUBLIC_BROWSER_PLUGIN_SERVICE_FILTER_H_

class GURL;

namespace webkit {
struct WebPluginInfo;
}

namespace content {

// Callback class to let the client filter the list of all installed plug-ins
// and block them from being loaded.
// This class is called on the FILE thread.
class PluginServiceFilter {
 public:
  virtual ~PluginServiceFilter() {}

  // Whether |plugin| is enabled. The client can return false to hide the
  // plugin, or return true and optionally change the passed in plugin.
  virtual bool IsPluginEnabled(int render_process_id,
                               int render_view_id,
                               const void* context,
                               const GURL& url,
                               const GURL& policy_url,
                               webkit::WebPluginInfo* plugin) = 0;

  // Whether the renderer has permission to load enabled |plugin|.
  virtual bool CanLoadPlugin(int render_process_id,
                             const FilePath& path) = 0;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_PLUGIN_SERVICE_FILTER_H_
