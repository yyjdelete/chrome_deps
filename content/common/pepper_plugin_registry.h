// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_PEPPER_PLUGIN_REGISTRY_H_
#define CONTENT_COMMON_PEPPER_PLUGIN_REGISTRY_H_

#include <list>
#include <map>

#include "PepperPluginInfo.h"
#include "webkit/plugins/ppapi/plugin_delegate.h"
#include "webkit/plugins/ppapi/plugin_module.h"
#include <wtf/text/StringHash.h>
#include <wtf/text/WTFString.h>
#include "content/common/content_export.h"

namespace ppapi {
class PluginModule;
}

namespace content {

/* FIXME
// Constructs a PepperPluginInfo from a WebPluginInfo. Returns false if
// the operation is not possible, in particular the WebPluginInfo::type
// must be one of the pepper types.
bool MakePepperPluginInfo(const webkit::WebPluginInfo& webplugin_info,
                          PepperPluginInfo* pepper_info);
*/

// This class holds references to all of the known pepper plugin modules.
//
// It keeps two lists. One list of preloaded in-process modules, and one list
// is a list of all live modules (some of which may be out-of-process and hence
// not preloaded).
class PepperPluginRegistry
    : public webkit::ppapi::PluginDelegate::ModuleLifetime {
 public:
  ~PepperPluginRegistry();

  static PepperPluginRegistry* GetInstance();

  // Computes the list of known pepper plugins.
  //
  // This method is static so that it can be used by the browser process, which
  // has no need to load the pepper plugin modules. It will re-compute the
  // plugin list every time it is called. Generally, code in the registry should
  // be using the cached plugin_list_ instead.
  CONTENT_EXPORT void ComputeList(
      std::vector<WebKit::PepperPluginInfo>* plugins);
  /* FIXME
  // Loads the (native) libraries but does not initialize them (i.e., does not
  // call PPP_InitializeModule). This is needed by the zygote on Linux to get
  // access to the plugins before entering the sandbox.
  static void PreloadModules();
  */

  // Retrieves the information associated with the given plugin info. The
  // return value will be NULL if there is no such plugin.
  //
  // The returned pointer is owned by the PluginRegistry.
  const WebKit::PepperPluginInfo* GetInfoForPlugin(
      const WebKit::PepperPluginInfo& info);

  // Returns an existing loaded module for the given path. It will search for
  // both preloaded in-process or currently active (non crashed) out-of-process
  // plugins matching the given name. Returns NULL if the plugin hasn't been
  // loaded.
  webkit::ppapi::PluginModule* GetLiveModule(const WTF::String& path);

  // Notifies the registry that a new non-preloaded module has been created.
  // This is normally called for out-of-process plugins. Once this is called,
  // the module is available to be returned by GetModule(). The module will
  // automatically unregister itself by calling PluginModuleDestroyed().
  void AddLiveModule(const WTF::String& path,
                     webkit::ppapi::PluginModule* module);

  // ModuleLifetime implementation.
  virtual void PluginModuleDead(
      webkit::ppapi::PluginModule* dead_module) OVERRIDE;

  void ComputeBuiltInPlugins(std::vector<WebKit::PluginModuleInfo>& plugins);
  void ComputeList(std::vector<WebKit::PluginModuleInfo>& plugins);

 private:
  PepperPluginRegistry();
  void AddPepperPlugins(std::vector<WebKit::PepperPluginInfo>* plugins);

  // All known pepper plugins.
  std::vector<WebKit::PepperPluginInfo> plugin_list_;

  // Plugins that have been preloaded so they can be executed in-process in
  // the renderer (the sandbox prevents on-demand loading).
  typedef std::map<base::FilePath, scoped_refptr<webkit::ppapi::PluginModule> >
      OwningModuleMap;
  OwningModuleMap preloaded_modules_;

  // A list of non-owning pointers to all currently-live plugin modules. This
  // includes both preloaded ones in preloaded_modules_, and out-of-process
  // modules whose lifetime is managed externally. This will contain only
  // non-crashed modules. If an out-of-process module crashes, it may
  // continue as long as there are WebKit references to it, but it will not
  // appear in this list.
  typedef std::map<base::FilePath, webkit::ppapi::PluginModule*> NonOwningModuleMap;
  NonOwningModuleMap live_modules_;

  DISALLOW_COPY_AND_ASSIGN(PepperPluginRegistry);
};

}  // namespace content

#endif  // CONTENT_COMMON_PEPPER_PLUGIN_REGISTRY_H_
