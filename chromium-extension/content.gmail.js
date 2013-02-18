console.log("goa: GOA content script for GMail loading");

function detectLoginInfo() {
  console.log("goa: Scraping for login info");
  var r = document.evaluate('//*[@role="navigation"]//text()[contains(.,"@")]', document.body, null, XPathResult.STRING_TYPE, null);
  var loginName = r.stringValue;
  if (!loginName)
    return false;
  var msg = {
    type: 'login-detected',
    data: {
      identity: loginName,
      provider: 'google',
      services: ['mail'],
      authenticationDomain: 'google.com'
    }
  };
  console.log("goa: Detected login info:", msg);
  chrome.extension.sendMessage(msg);
  return true;
}

// Run detectLoginInfo() on any DOM change if not ready yet
if (!detectLoginInfo()) {
  var t = {};
  t.observer = new WebKitMutationObserver(function(mutations, observer) {
    if (detectLoginInfo()) {
      observer.disconnect();
      clearTimeout(t.timeout);
    }
  });
  t.observer.observe(document.body, { childList: true, subtree: true, characterData: true });
  t.timeout = setTimeout(function() {
      console.log("goa: Give up, unable to find any login info");
      t.observer.disconnect();
  }, 10000);
};
