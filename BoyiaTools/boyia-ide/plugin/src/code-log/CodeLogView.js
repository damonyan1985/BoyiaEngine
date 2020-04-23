const vscode = require('vscode');

class CodeLogView {
  constructor() {
    this.logView = vscode.window.createOutputChannel('Log (Boyia)');
    this.logView.show();
  }

  logPrint(log) {
    this.logView.appendLine(log)
  }
}

module.exports = CodeLogView;