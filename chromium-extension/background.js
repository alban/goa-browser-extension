var plugin = document.getElementById("gnome-online-accounts-plugin");
console.log("goa: GNOME Online Accounts background script loading");

chrome.extension.onMessage.addListener(function(request, sender, sendResponse) {
    console.log("goa: Message received", request);
    if (typeof(request) != 'object' || request['type'] != 'login-detected')
        return;

    var collectedData = request.data;

    chrome.cookies.getAll({domain:collectedData.authenticationDomain}, function(cookies) {
        collectedData.cookies = cookies.map(function(cookie) {
            var c = cookie;
            delete c.storeId;
            console.log("goa: cookie", c);
            return c;
        });
        plugin.loginDetected(JSON.stringify(collectedData));
    });
});
