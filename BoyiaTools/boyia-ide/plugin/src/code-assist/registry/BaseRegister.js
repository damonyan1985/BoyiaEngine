const vscode = require('vscode');

class BaseRegister {
  exec(text, config) {
    return this.getAssist([]);
  }

  getAssist(apis) {
    return apis.map(api => {
      return new vscode.CompletionItem(api, vscode.CompletionItemKind.Field);
    })
  }
}

module.exports = BaseRegister;

