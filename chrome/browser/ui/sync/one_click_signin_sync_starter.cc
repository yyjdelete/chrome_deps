// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/sync/one_click_signin_sync_starter.h"

#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/signin/signin_manager.h"
#include "chrome/browser/signin/signin_manager_factory.h"
#include "chrome/browser/sync/profile_sync_service.h"
#include "chrome/browser/sync/profile_sync_service_factory.h"
#include "chrome/browser/sync/sync_prefs.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/chrome_pages.h"
#include "chrome/browser/ui/webui/signin/login_ui_service.h"
#include "chrome/browser/ui/webui/signin/login_ui_service_factory.h"
#include "chrome/common/url_constants.h"

OneClickSigninSyncStarter::OneClickSigninSyncStarter(
    Profile* profile,
    Browser* browser,
    const std::string& session_index,
    const std::string& email,
    const std::string& password,
    StartSyncMode start_mode)
    : profile_(profile),
      browser_(browser),
      signin_tracker_(profile_, this),
      start_mode_(start_mode) {
  DCHECK(profile_);

  // Let the sync service know that setup is in progress so it doesn't start
  // syncing until the user has finished any configuration.
  ProfileSyncService* profile_sync_service =
      ProfileSyncServiceFactory::GetForProfile(profile_);
  profile_sync_service->SetSetupInProgress(true);

  // Make sure the syncing is not suppressed, otherwise the SigninManager
  // will not be able to compelte sucessfully.
  browser_sync::SyncPrefs sync_prefs(profile_->GetPrefs());
  sync_prefs.SetStartSuppressed(false);

  SigninManager* manager = SigninManagerFactory::GetForProfile(profile_);
  manager->StartSignInWithCredentials(session_index, email, password);
}

OneClickSigninSyncStarter::~OneClickSigninSyncStarter() {
}

void OneClickSigninSyncStarter::GaiaCredentialsValid() {
}

void OneClickSigninSyncStarter::SigninFailed(
    const GoogleServiceAuthError& error) {
  ProfileSyncService* profile_sync_service =
      ProfileSyncServiceFactory::GetForProfile(profile_);
  profile_sync_service->SetSetupInProgress(false);
  delete this;
}

void OneClickSigninSyncStarter::SigninSuccess() {
  ProfileSyncService* profile_sync_service =
      ProfileSyncServiceFactory::GetForProfile(profile_);

  switch (start_mode_) {
    case SYNC_WITH_DEFAULT_SETTINGS:
      // Just kick off the sync machine, no need to configure it first.
      profile_sync_service->OnUserChoseDatatypes(true, syncer::ModelTypeSet());
      profile_sync_service->SetSyncSetupCompleted();
      profile_sync_service->SetSetupInProgress(false);
      break;
    case CONFIGURE_SYNC_FIRST: {
      // Give the user a chance to configure things. We don't clear the
      // ProfileSyncService::setup_in_progress flag because we don't want sync
      // to start up until after the configure UI is displayed (the configure UI
      // will clear the flag when the user is done setting up sync).
      LoginUIService* login_ui = LoginUIServiceFactory::GetForProfile(profile_);
      if (login_ui->current_login_ui()) {
        login_ui->current_login_ui()->FocusUI();
      } else if (browser_) {
        // Need to navigate to the settings page and display the UI.
        chrome::ShowSettingsSubPage(browser_, chrome::kSyncSetupSubPage);
      }
      break;
    }
    default:
      NOTREACHED() << "Invalid start_mode=" << start_mode_;
  }

  delete this;
}
