var buttonIgnore = document.getElementById("goa-popup-button-ignore");
var buttonCreate = document.getElementById("goa-popup-button-create");

function getAccountId() {
    return location.hash.replace(/^#/,'')
}

buttonIgnore.onclick = function(evt) {
    var accountId = getAccountId();
    var msg = { type: "account-ignore", accountId: accountId };
    chrome.extension.sendMessage(msg);
    window.close();
};

buttonCreate.onclick = function(evt) {
    var accountId = getAccountId();
    var msg = { type: "account-create", accountId: accountId };
    chrome.extension.sendMessage(msg);
    window.close();
};
