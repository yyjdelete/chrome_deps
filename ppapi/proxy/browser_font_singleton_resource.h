// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_PROXY_BROWSER_FONT_SINGLETON_RESOURCE_H_
#define PPAPI_PROXY_BROWSER_FONT_SINGLETON_RESOURCE_H_

#include "ppapi/proxy/connection.h"
#include "ppapi/proxy/plugin_resource.h"
#include "ppapi/thunk/ppb_browser_font_singleton_api.h"

namespace ppapi {
namespace proxy {

// This handles the singleton calls (that don't take a PP_Resource parameter)
// on the browser font interface
class BrowserFontSingletonResource
    : public PluginResource,
      public thunk::PPB_BrowserFont_Singleton_API {
 public:
  BrowserFontSingletonResource(Connection connection, PP_Instance instance);
  virtual ~BrowserFontSingletonResource();

  // Resource override.
  virtual thunk::PPB_BrowserFont_Singleton_API*
      AsPPB_BrowserFont_Singleton_API() OVERRIDE;

  // thunk::PPB_BrowserFontSingleton_API implementation.
  virtual PP_Var GetFontFamilies(PP_Instance instance) OVERRIDE;

 private:
  // Lazily-filled-in list of font families.
  std::string families_;

  DISALLOW_COPY_AND_ASSIGN(BrowserFontSingletonResource);
};

}  // namespace proxy
}  // namespace ppapi

#endif  // PPAPI_PROXY_BROWSER_FONT_SINGLETON_RESOURCE_H_

