// OmmLoader
/*
The OmmLoader class is used for initial bootstrapping of the API, and is defined in Javascript stored on the application web server.  
A single function, getManager, connects to the locally-running middleware and returns a manager object.  

Promise<function(OmmManager)> OmmLoader.getManager(managerParameters:Object);

Where:
  managerParameters.hostname:String = "localhost"; 
*/

var OmmLoader = (function () {
	var my = {};

	//var privateVariable = 1;    
	//my.moduleProperty = 1;
	
	function loadScript(url, callbackSuccess, callbackFailure) {
		var resolved = false;
		
		// Create a new script element
		var script = document.createElement('script');

		function localSuccess() {
			if (resolved) { return; }
			resolved = true;
			if (callbackSuccess) { callbackSuccess(); }
		}
		
		// Bind the callback event
		script.onreadystatechange = function() {
			if (script.readyState == 'complete') {
				localSuccess();
			}
		};		
		script.onload = localSuccess;

		script.type = 'text/javascript';
		script.src = url;
		
		// Create timeouts
		setTimeout(function() {
			if (resolved) { return; }
			resolved = true;
			if (callbackFailure) { callbackFailure('Timed-out loading local script.'); }
		}, 8000);
			
		// Add the script tag to the head
		var head = document.getElementsByTagName('head')[0];
		head.appendChild(script);
	}
	
	my.getManager = function (managerParameters, callbackSuccess, callbackFail) {
	
		// Check for required features
		if (typeof WebSocket == 'undefined') { 
			if (callbackFail) { callbackFail("Unsupported browser - no WebSockets"); }
			return;
		}
		
		// Create empty managerParameters if none specified
		if (typeof managerParameters == "undefined") 
		{ 
			managerParameters = {}; 
		}
		
		// Automatically choose the address if not specified
		if (typeof managerParameters.domain == "undefined")
		{ 
			// Determine the default address
			var protocol = (location.protocol == "https:") ? "https:" : "http:";
			var host = "localhost"; //"127.0.0.1"; // "localhost";
			var port = (protocol == "https:") ? 1235 : 1234;
			managerParameters.domain = protocol + "//" + host + ":" + port;
		}
		
		var scriptUrl = managerParameters.domain + "/omm-manager.js";
		loadScript(scriptUrl, function() {
			if (typeof OmmManager == 'undefined') {
				if (callbackFail) { callbackFail('Problem loading script.'); }
				return;
			}
			var manager = OmmManager;
			manager.start(
				function () {
					if (callbackSuccess) { callbackSuccess(manager); }
				}, function () {
					if (callbackFail) { callbackFail('Problem running manager.'); }
				},
				managerParameters
			);
			
		}, function() {
			if (callbackFail) { callbackFail('Could not load manager, check middleware is running.'); }
		});
	};

	return my;
}());
