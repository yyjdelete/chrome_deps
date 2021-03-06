// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Called by the common.js module.
function attachListeners() {
  document.querySelector('form').addEventListener('submit', askBall);
}

// Called by the common.js module.
function moduleDidLoad() {
  // The module is not hidden by default so we can easily see if the plugin
  // failed to load.
  common.hideModule();
}

function askBall(event) {
  var questionEl = document.getElementById('question');
  var query = questionEl.value;
  questionEl.value = '';
  document.getElementById('log').innerHTML += 'You asked:' + query + '<br>';
  common.naclModule.postMessage('query');
  event.preventDefault();
}
