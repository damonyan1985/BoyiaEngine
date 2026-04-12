const vscode = require('vscode');
const fs = require('fs');
const CodeUtil = require('../code-util/CodeUtil');
const CodeGlobal = require('../code-global/CodeGlobal');
const CodeRegistry = require('./CodeRegistry');
const { KEYWORDS, GLOBALS, SHORTCUTS, KEYWORD_DOC } = require('./CodeKeywords');
const CodeIndex = require('./CodeIndex');

/** Global builtin class names (VM / CLI / SDK). */
const BUILTIN_GLOBAL_CLASSES = [
  'Array',
  'File',
  'Https',
  'Json',
  'Map',
  'MicroTask',
  'String',
  'Util',
  'Zip',
];

class CodeAssist {
  /**
   * @param {vscode.CompletionItem} item
   * @returns {string}
   */
  static completionItemLabelString(item) {
    if (!item || item.label == null) {
      return '';
    }
    const lab = item.label;
    return typeof lab === 'string' ? lab : /** @type {{ label?: string }} */ (lab).label || '';
  }

  static async initialize() {
    try {
      await CodeAssist.linkBoyiaFile();
      const content = fs.readFileSync(CodeUtil.getAbsolutePath(CodeGlobal.context, 'config/assist.json'), 'UTF-8');
      CodeAssist.config = JSON.parse(content);
      if (!CodeAssist.config.namespaces) {
        CodeAssist.config.namespaces = {};
      }
      CodeRegistry.register();
    } catch (e) {
      console.error('CodeAssist::initialize', e);
      CodeAssist.config = { namespaces: {}, apiDocs: {} };
      CodeRegistry.register();
    }
  }

  /**
   * @param {vscode.CompletionItem[]} items
   */
  static dedupeCompletionItems(items) {
    const map = new Map();
    for (const it of items) {
      const lab = CodeAssist.completionItemLabelString(it);
      if (lab && !map.has(lab)) {
        map.set(lab, it);
      }
    }
    return Array.from(map.values());
  }

  /**
   * Text from line start to cursor (exclusive of rest of line).
   * @param {string} lineText
   * @param {number} character
   */
  static linePrefix(lineText, character) {
    const end = Math.min(character, lineText.length);
    return lineText.substring(0, end);
  }

  /**
   * `Class.` + optional partial method: last identifier segment with trailing dot.
   * @param {string} before
   * @returns {{ namespace: string, partial: string } | null}
   */
  static parseDottedCompletion(before) {
    const m = before.match(/(?:^|[^\w.])([A-Za-z_]\w*\.)([\w]*)$/);
    if (!m) {
      return null;
    }
    return { namespace: m[1], partial: m[2] || '' };
  }

  /**
   * Identifier or prefix being typed at end of `before` (no leading dot segment for Util.foo).
   * @param {string} before
   */
  static parseWordPrefix(before) {
    const m = before.match(/([A-Za-z_][\w]*)$/);
    return m ? m[1] : '';
  }

  /**
   * @param {string[]} names
   * @param {string} partial
   * @param {vscode.CompletionItemKind} kind
   * @param {Record<string, string>} [detailByName]
   * @param {string} [categoryDetail] default `detail` when no per-name entry
   */
  static filterCompletionNames(names, partial, kind, detailByName, categoryDetail) {
    const p = partial.toLowerCase();
    return names
      .filter((n) => !p || n.toLowerCase().startsWith(p))
      .map((n) => {
        const item = new vscode.CompletionItem(n, kind);
        item.insertText = n;
        if (detailByName && detailByName[n]) {
          item.detail = detailByName[n];
        } else if (categoryDetail) {
          item.detail = categoryDetail;
        }
        return item;
      });
  }

  /**
   * Merges legacy `config.util` into `namespaces['Util.']` if present.
   * @returns {Record<string, string[]>}
   */
  static getEffectiveNamespaces() {
    const cfg = CodeAssist.config || {};
    const raw = Object.assign({}, cfg.namespaces || {});
    if (Array.isArray(cfg.util) && cfg.util.length) {
      const u = new Set((raw['Util.'] || []).concat(cfg.util));
      raw['Util.'] = Array.from(u).sort(function (a, b) {
        return a.localeCompare(b);
      });
    }
    return raw;
  }

  /**
   * Built-in `Class.method` completions with optional `apiDocs` from assist.json.
   * @param {string[]} names
   * @param {string} partial
   * @param {string} namespaceKey e.g. `Util.`
   * @param {Record<string, unknown>} [config]
   */
  static filterBuiltinNamespaceMethods(names, partial, namespaceKey, config) {
    const cfg = config || CodeAssist.config || {};
    const apiDocs = cfg.apiDocs || {};
    const cls = namespaceKey.replace(/\.$/, '');
    const p = partial.toLowerCase();
    return names
      .filter((n) => !p || n.toLowerCase().startsWith(p))
      .map((n) => {
        const item = new vscode.CompletionItem(n, vscode.CompletionItemKind.Method);
        item.insertText = n;
        item.detail = `${cls} · 内置`;
        const docKey = `${cls}.${n}`;
        if (apiDocs[docKey]) {
          item.documentation = new vscode.MarkdownString(String(apiDocs[docKey]));
        }
        return item;
      });
  }

  /**
   * @param {string} partial
   */
  static builtinClassCompletionItems(partial) {
    const p = partial.toLowerCase();
    return BUILTIN_GLOBAL_CLASSES.filter((c) => !p || c.toLowerCase().startsWith(p)).map((c) => {
      const item = new vscode.CompletionItem(c, vscode.CompletionItemKind.Class);
      item.insertText = c;
      item.detail = '内置类';
      return item;
    });
  }

  /**
   * @param {string[]} words
   * @param {string} partial
   */
  static keywordItems(words, partial) {
    return CodeAssist.filterCompletionNames(words, partial, vscode.CompletionItemKind.Keyword, undefined, '关键字');
  }

  /**
   * Document-local symbols (var / params / fun / class / prop names) before cursor.
   * @param {vscode.TextDocument} document
   * @param {vscode.Position} position
   * @param {string} word prefix being typed
   * @returns {vscode.CompletionItem[]}
   */
  static documentSymbolCompletionItems(document, position, word) {
    if (!word) {
      return [];
    }
    const text = document.getText();
    const offset = document.offsetAt(position);
    const raw = CodeIndex.collectSymbolNames(text, offset);
    const p = word.toLowerCase();
    const builtinLc = new Set(BUILTIN_GLOBAL_CLASSES.map((c) => c.toLowerCase()));
    const kwLc = new Set(KEYWORDS.concat(GLOBALS).map((k) => k.toLowerCase()));
    const names = raw.filter(function (n) {
      if (!n.toLowerCase().startsWith(p)) {
        return false;
      }
      const nl = n.toLowerCase();
      if (builtinLc.has(nl) || kwLc.has(nl)) {
        return false;
      }
      return true;
    });
    return names.map(function (n) {
      const item = new vscode.CompletionItem(n, vscode.CompletionItemKind.Variable);
      item.insertText = n;
      item.detail = '变量';
      return item;
    });
  }

  /**
   * @param {vscode.TextDocument} document
   * @param {vscode.Position} position
   * @param {vscode.CancellationToken} _token
   * @param {vscode.CompletionContext} context
   * @returns {vscode.CompletionItem[] | undefined}
   */
  static provideCompletionItems(document, position, _token, context) {
    if (!CodeAssist.config) {
      return undefined;
    }

    const lineText = document.lineAt(position).text;
    const before = CodeAssist.linePrefix(lineText, position.character);

    const dotted = CodeAssist.parseDottedCompletion(before);
    if (dotted) {
      const { namespace, partial } = dotted;
      const nsMap = CodeAssist.getEffectiveNamespaces();
      if (Object.prototype.hasOwnProperty.call(nsMap, namespace) && Array.isArray(nsMap[namespace])) {
        return CodeAssist.filterBuiltinNamespaceMethods(nsMap[namespace], partial, namespace, CodeAssist.config);
      }

      const receiver = namespace.slice(0, -1);
      const fullText = document.getText();
      const offset = document.offsetAt(position);
      const { classes } = CodeIndex.parseDocument(fullText);

      /** @type {string | null} */
      let instClass = null;
      if (receiver === 'this') {
        const ic = CodeIndex.innermostClassAtOffset(fullText, offset);
        instClass = ic && ic.name;
      } else {
        const vmap = CodeIndex.varToClassBeforeOffset(fullText, offset);
        instClass = vmap.get(receiver) || null;
      }

      if (instClass && classes.has(instClass)) {
        const mems = CodeIndex.membersForClass(instClass, classes);
        /** @type {Record<string, string>} */
        const detail = {};
        for (const n of mems) {
          detail[n] = `成员 · ${instClass}`;
        }
        return CodeAssist.filterCompletionNames(
          mems,
          partial,
          vscode.CompletionItemKind.Method,
          detail
        );
      }
      if (instClass) {
        const nsKey = instClass + '.';
        if (Object.prototype.hasOwnProperty.call(nsMap, nsKey) && Array.isArray(nsMap[nsKey])) {
          return CodeAssist.filterBuiltinNamespaceMethods(nsMap[nsKey], partial, nsKey, CodeAssist.config);
        }
      }

      return undefined;
    }

    const word = CodeAssist.parseWordPrefix(before);
    if (!word) {
      if (context.triggerKind === vscode.CompletionTriggerKind.Invoke) {
        return CodeAssist.dedupeCompletionItems(
          CodeAssist.builtinClassCompletionItems('').concat(
            CodeAssist.keywordItems(KEYWORDS.concat(GLOBALS), '')
          )
        );
      }
      return undefined;
    }

    /** @type {vscode.CompletionItem[]} */
    let items = [];

    items = items.concat(CodeAssist.builtinClassCompletionItems(word));

    if (CodeRegistry.registers && CodeRegistry.registers[word]) {
      const fromRegistry = CodeRegistry.registers[word].exec(word, CodeAssist.config);
      if (fromRegistry && fromRegistry.length) {
        items = items.concat(fromRegistry);
      }
    }

    const lastChar = word[word.length - 1];
    if (word.length === 1 && SHORTCUTS[lastChar]) {
      items = items.concat(CodeAssist.keywordItems(SHORTCUTS[lastChar], ''));
    }

    const merged = new Set(items.map((i) => CodeAssist.completionItemLabelString(i)));
    const kw = KEYWORDS.concat(GLOBALS).filter(function (k) {
      return k.toLowerCase().startsWith(word.toLowerCase()) && !merged.has(k);
    });
    items = items.concat(CodeAssist.keywordItems(kw, ''));

    items = items.concat(CodeAssist.documentSymbolCompletionItems(document, position, word));

    const out = CodeAssist.dedupeCompletionItems(items);
    return out.length ? out : undefined;
  }

  static resolveCompletionItem(item, _token) {
    const label = CodeAssist.completionItemLabelString(item);
    if (KEYWORD_DOC[label]) {
      item.documentation = new vscode.MarkdownString(KEYWORD_DOC[label]);
    }
    return item;
  }

  /**
   * @param {vscode.TextDocument} document
   * @param {vscode.Position} position
   */
  static provideHover(document, position) {
    if (!CodeAssist.config) {
      return null;
    }
    const range = document.getWordRangeAtPosition(position, /[A-Za-z_][\w.]*/);
    if (!range) {
      return null;
    }
    const word = document.getText(range);
    const doc = KEYWORD_DOC[word];
    if (doc) {
      const md = new vscode.MarkdownString(doc);
      md.isTrusted = true;
      return new vscode.Hover(md, range);
    }

    const dotted = word.match(/^([A-Za-z_]\w*)\.(\w+)$/);
    if (dotted) {
      const ns = `${dotted[1]}.`;
      const method = dotted[2];
      const nsMap = CodeAssist.getEffectiveNamespaces();
      const apis = nsMap[ns];
      if (Array.isArray(apis) && apis.includes(method)) {
        const cls = dotted[1];
        const docKey = `${cls}.${method}`;
        const apiDoc = (CodeAssist.config.apiDocs && CodeAssist.config.apiDocs[docKey]) || '';
        const body = apiDoc ? `${apiDoc}\n\n` : '';
        const md = new vscode.MarkdownString(`${body}\`${word}\` — **${cls}** 内置方法（\`assist.json\`）。`);
        md.isTrusted = true;
        return new vscode.Hover(md, range);
      }

      const fullText = document.getText();
      const offset = document.offsetAt(range.end);
      const { classes } = CodeIndex.parseDocument(fullText);
      const receiver = dotted[1];
      /** @type {string | null} */
      let instClass = null;
      if (receiver === 'this') {
        const ic = CodeIndex.innermostClassAtOffset(fullText, offset);
        instClass = ic && ic.name;
      } else {
        const vmap = CodeIndex.varToClassBeforeOffset(fullText, offset);
        instClass = vmap.get(receiver) || null;
      }
      if (instClass && classes.has(instClass)) {
        const mems = CodeIndex.membersForClass(instClass, classes);
        if (mems.includes(method)) {
          const md = new vscode.MarkdownString(`\`${word}\` — \`${instClass}\` 的成员。`);
          md.isTrusted = true;
          return new vscode.Hover(md, range);
        }
      } else if (instClass) {
        const nsKey = instClass + '.';
        const apis = nsMap[nsKey];
        if (Array.isArray(apis) && apis.includes(method)) {
          const cls = instClass;
          const docKey = `${cls}.${method}`;
          const apiDoc = (CodeAssist.config.apiDocs && CodeAssist.config.apiDocs[docKey]) || '';
          const body = apiDoc ? `${apiDoc}\n\n` : '';
          const md = new vscode.MarkdownString(`${body}\`${word}\` — **${cls}** 内置实例方法。`);
          md.isTrusted = true;
          return new vscode.Hover(md, range);
        }
      }
    }

    return null;
  }

  static register() {
    CodeAssist.initialize()
      .then(() => {
        const sel = { language: 'boyia' };
        CodeGlobal.context.subscriptions.push(
          vscode.languages.registerCompletionItemProvider(
            sel,
            {
              provideCompletionItems: CodeAssist.provideCompletionItems,
              resolveCompletionItem: CodeAssist.resolveCompletionItem,
            },
            '.',
            '('
          ),
          vscode.languages.registerHoverProvider(sel, {
            provideHover: CodeAssist.provideHover.bind(CodeAssist),
          })
        );
      })
      .catch((e) => console.error('CodeAssist::register', e));
  }

  /** @deprecated typo — use {@link CodeAssist.register} */
  static reigister() {
    CodeAssist.register();
  }

  static async linkBoyiaFile() {
    const config = vscode.workspace.getConfiguration();
    const associateConfig = config.get('files.associations') || {};

    if (associateConfig['*.boui'] === 'xml' && associateConfig['*.boss'] === 'css') {
      return;
    }

    await config.update(
      'files.associations',
      Object.assign({}, associateConfig, {
        '*.boui': 'xml',
        '*.boss': 'css',
      }),
      vscode.ConfigurationTarget.Global
    );
  }
}

module.exports = CodeAssist;
