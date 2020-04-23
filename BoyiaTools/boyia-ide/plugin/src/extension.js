const vscode = require('vscode');
const CodeGlobal = require('./code-global/CodeGlobal');
const CodeAssist = require('./code-assist/CodeAssist');
const CodeLogImpl = require('./code-log/CodeLogImpl');

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
	CodeGlobal.context = context;
	CodeLogImpl.init();
	CodeAssist.reigister();
}
exports.activate = activate;

// this method is called when your extension is deactivated
function deactivate() { }

module.exports = {
	activate,
	deactivate
}
