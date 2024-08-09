const vscode = require('vscode');

class BaseRegister {
  exec(text, config) {
    return this.getAssist([]);
  }

  isApi() {
    return false;
  }

  isKeyword() {
    return false;
  }

  isConstant() {
    return false;
  }

  getAssist(apis) {
    return apis.map(api => {
      if (this.isKeyword()) {
        return new vscode.CompletionItem(api, vscode.CompletionItemKind.Keyword);
      }

      if (this.isConstant()) {
        return new vscode.CompletionItem(api, vscode.CompletionItemKind.Constant);
      }

      if (this.isApi()) {
        return new vscode.CompletionItem(api, vscode.CompletionItemKind.Method);
      }

      return new vscode.CompletionItem(api, vscode.CompletionItemKind.Field);
    });
  }
}

module.exports = BaseRegister;

