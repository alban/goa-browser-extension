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

// Get rid of collected data without removing the account id,
// so we can keep track of the accounts already handled
function accountDataSterilize(accountId) {
    accountData[accountId] = false;
}

function accountIsIgnored(accountId)
{
    return localStorage.getItem("ignore-"+accountId);
}

function accountSetIgnored(accountId)
{
    localStorage.setItem("ignore-"+accountId, new Date());
}

function loginDetected(request, sender) {
    var accountId = generateAccountId(request.data);
    if (!accountId)
        return;
    // Do nothing if already dismissed or configured
    if (accountIsIgnored(accountId) || accountDataContains(accountId))
        return;
    accountDataStore(accountId, request.data);
    chrome.experimental.infobars.show({
        tabId: sender.tab.id,
        path: "infobar.html#"+accountId
    });
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

    chrome.cookies.getAll({domain:data.authenticationDomain}, function(cookies) {
        data.cookies = cookies.map(function(cookie) {
            var c = cookie;
            delete c.storeId;
            console.log("goa: cookie", c);
            return c;
        });
        plugin.loginDetected(JSON.stringify(data));
    });
};

function accountIgnore(request, sender) {
    var accountId = request.accountId;
    console.log("goa: ignore account '"+accountId+"'");
    if (!accountId)
        return;
    if (accountIsIgnored(accountId))
      return;
    accountSetIgnored(accountId);
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
