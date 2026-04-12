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
        const it = new vscode.CompletionItem(api, vscode.CompletionItemKind.Keyword);
        it.detail = '关键字';
        return it;
      }

      if (this.isConstant()) {
        const it = new vscode.CompletionItem(api, vscode.CompletionItemKind.Constant);
        it.detail = '常量';
        return it;
      }

      if (this.isApi()) {
        const it = new vscode.CompletionItem(api, vscode.CompletionItemKind.Method);
        it.detail = 'API';
        return it;
      }

      const it = new vscode.CompletionItem(api, vscode.CompletionItemKind.Field);
      it.detail = '字段';
      return it;
    });
  }
}

module.exports = BaseRegister;

