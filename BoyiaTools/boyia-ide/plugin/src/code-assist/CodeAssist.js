const vscode = require('vscode');
const fs = require('fs');
const CodeUtil = require('../code-util/CodeUtil');
const CodeGlobal = require('../code-global/CodeGlobal');
const CodeRegistry = require('./CodeRegistry');

class CodeAssist {
  static async initialize() {
    try {
      await CodeAssist.linkBoyiaFile();
      const content = fs.readFileSync(CodeUtil.getAbsolutePath(CodeGlobal.context, 'config/assist.json'), "UTF-8");
      CodeAssist.config = JSON.parse(content);

      CodeRegistry.register();
    } catch (e) {
      console.log('CodeAssist::initialize---err: ' + e);
    }
  }

  static provideDefinition(document, position, token, context) {
    console.log('CodeAssist::provideDefinition');
    const line = document.lineAt(position);
    const lineText = line.text.substring(0, position.character);
    const text = lineText.replace(/^\s*|\s*$/g, '');

    console.log('CodeAssist::provideDefinition---config=' + JSON.stringify(CodeAssist.config));
    console.log('CodeAssist::provideDefinition---text=' + text);

    const reg = /(c|f|p|w|d|r|v|e)/;
    const keys = text.split(' ');

    console.log('CodeAssist::provideDefinition---key=' + keys[keys.length - 1]);
    if (reg.test(keys[keys.length - 1])) {
      return CodeRegistry.keyword.exec(keys[keys.length - 1], null);
    }


    if (CodeRegistry.registers[text]) {
      return CodeRegistry.registers[text].exec(text, CodeAssist.config);
    }

    return null;
  }

  static resolveCompletionItem(item, token) {
    return item;
  }

  static reigister() {
    CodeAssist.initialize();
    CodeGlobal.context.subscriptions.push(
      vscode.languages.registerCompletionItemProvider(
        "boyia",
        {
          provideCompletionItems: CodeAssist.provideDefinition,
          resolveCompletionItem: CodeAssist.resolveCompletionItem
        },
        '.'
      )
    );
  };

  static async linkBoyiaFile() {
    const config = vscode.workspace.getConfiguration();
    const associateConfig = config.get('files.associations');

    if (associateConfig['*.boui'] === 'xml' && associateConfig['*.boss'] === 'css') {
      return;
    }

    await config['update'](
      'files.associations',
      Object.assign({}, associateConfig, {
        '*.boui': 'xml',
        '*.boss': 'css'
      })
    );
  }
}

module.exports = CodeAssist;