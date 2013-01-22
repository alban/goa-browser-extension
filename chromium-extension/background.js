var plugin = document.getElementById("gnome-online-accounts-plugin");
console.log("goa: GNOME Online Accounts background script loading");

chrome.extension.onMessage.addListener(function(request, sender, sendResponse) {
    console.log("goa: Message received", request);
    if (typeof(request) != 'object' || request['type'] != 'login-detected')
        return;

    plugin.loginDetected(request.domain, request.login, null);
});
