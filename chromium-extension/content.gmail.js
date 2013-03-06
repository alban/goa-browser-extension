console.log("goa: GOA content script for GMail loading");

function detectLoginInfo(selector, services) {
  console.log("goa: Scraping for login info");
  var r = document.evaluate(selector, document.body, null, XPathResult.STRING_TYPE, null);
  var loginName = r.stringValue;
  if (!loginName)
    return false;
  var msg = {
    type: 'login-detected',
    data: {
      identity: loginName,
      provider: 'google',
      services: services,
      authenticationDomain: 'google.com'
    }
  };
  console.log("goa: Detected login info:", msg);
  chrome.extension.sendMessage(msg);
  return true;
}

function setupGoaIntegration(selector, services)
{
  // Run detectLoginInfo() on any DOM change if not ready yet
  if (!detectLoginInfo()) {
    var t = {};
    t.observer = new WebKitMutationObserver(function(mutations, observer) {
      if (detectLoginInfo(selector, services)) {
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
}

setupGoaIntegration (
  '//*[@role="navigation"]//*[starts-with(@href, "https://profiles.google.com/")]//text()[contains(.,"@")]',
  ['mail']
);
