var plugin = document.getElementById("gnome-online-accounts-plugin");
console.log("goa: GNOME Online Accounts background script loading");

var accountData = {};

function generateAccountId(requestData)
{
    if (!requestData || !requestData.provider || !requestData.identity)
        return;
    return "goa|"+requestData.provider+"|"+requestData.identity;
}

function accountDataStore(accountId, data) {
    accountData[accountId] = data;
}

function accountDataContains(accountId) {
    return (accountId in accountData);
}

// Grab the collected data and get rid of it
function accountDataTake(accountId) {
    if (!accountDataContains(accountId))
        return;
    data = accountData[accountId];
    accountDataSterilize(accountId);
    return data;
}

function accountDataPeek(accountId) {
    if (!accountDataContains(accountId))
        return;
    return accountData[accountId];
}

// Get rid of collected data without removing the account id,
// so we can keep track of the accounts already handled
function accountDataSterilize(accountId) {
    accountData[accountId] = false;
}

function accountDataIsSterilized(accountId) {
    return accountData[accountId] === false;
}

function accountIsPermanentlyIgnored(accountId)
{
    return localStorage.getItem("ignore-"+accountId);
}

function accountSetPermanentlyIgnored(accountId)
{
    localStorage.setItem("ignore-"+accountId, new Date());
}

function accountAlreadyConfigured (requestData)
{
    var accounts = plugin.listAccounts();
    for (var i=0; i<accounts.length; i++) {
        var account = accounts[i];
        if (account.providerType == requestData.provider && account.identity == requestData.identity)
          return true;
    }
    return false;
}

function loginDetected(request, sender) {
    if (accountAlreadyConfigured (request.data))
        return;
    var accountId = generateAccountId(request.data);
    if (!accountId)
        return;
    // Do nothing if already dismissed or configured
    if (accountIsPermanentlyIgnored(accountId) || accountDataIsSterilized(accountId))
        return;
    // Cleanup the page action icon on the old tab, if any
    var oldData = accountDataPeek(accountId);
    if (oldData && oldData.tabId != sender.tab.id)
        chrome.pageAction.hide(oldData.tabId);
    // Store/Refresh the collected data
    var data = {
        collected: request.data,
        tabId: sender.tab.id
    };
    accountDataStore(accountId, data);

    // Show the infobar only if experimental APIs are enabled
    if (chrome.experimental && chrome.experimental.infobars) {
        // Show the infobar the first time only
        if (!oldData) {
            chrome.experimental.infobars.show({
                tabId: sender.tab.id,
                path: "infobar.html#"+accountId
            });
        }
    }
    chrome.pageAction.setPopup({
        tabId: sender.tab.id,
        popup: "popup.html#"+accountId
    });
    chrome.pageAction.show(sender.tab.id);
}

function accountCreate(request, sender) {
    var accountId = request.accountId;
    console.log("goa: create account", accountId);
    if (!accountId)
        return;
    var data = accountDataTake(accountId);
    console.log("goa: collected data", data);
    // Do nothing if non-existent or already created
    if (!data)
        return;
    chrome.pageAction.hide(data.tabId);
    var collected = data.collected;
    chrome.cookies.getAll({domain:collected.authenticationDomain}, function(cookies) {
        collected.cookies = cookies.map(function(cookie) {
            var c = cookie;
            delete c.storeId;
            console.log("goa: cookie", c);
            return c;
        });
        plugin.loginDetected(JSON.stringify(collected));
    });
};

function accountIgnore(request, sender) {
    var accountId = request.accountId;
    console.log("goa: ignore account '"+accountId+"'");
    if (!accountId)
        return;
    var data = accountDataTake(accountId);
    // Do nothing if non-existent or already created
    if (!data)
        return;
    chrome.pageAction.hide(data.tabId);
    if (accountIsPermanentlyIgnored(accountId))
      return;
    accountSetPermanentlyIgnored(accountId);
};

chrome.extension.onMessage.addListener(function(request, sender, sendResponse) {
    console.log("goa: Message received", request, "from", sender);
    var ret = {};
    if (typeof(request) == 'object') {
      if (request['type'] == 'login-detected')
        ret = loginDetected(request, sender);
      else if (request['type'] == 'account-create')
        ret = accountCreate(request, sender);
      else if (request['type'] == 'account-ignore')
        ret = accountIgnore(request, sender);
    }
    sendResponse(ret);
});
