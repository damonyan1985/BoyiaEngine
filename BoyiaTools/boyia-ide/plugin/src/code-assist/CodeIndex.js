'use strict';

/**
 * Boyia document index: classes (members + extends), var → new(Class), this. resolution.
 */

/**
 * @param {string} text
 * @param {number} openBraceIndex index of `{` opening a block
 * @returns {number} index of matching `}`, or -1
 */
function indexOfMatchingBrace(text, openBraceIndex) {
  let depth = 1;
  let i = openBraceIndex + 1;
  let inStr = false;
  let strQuote = '';
  let esc = false;
  while (i < text.length) {
    const c = text[i];
    if (inStr) {
      if (esc) {
        esc = false;
        i++;
        continue;
      }
      if (c === '\\') {
        esc = true;
        i++;
        continue;
      }
      if (c === strQuote) {
        inStr = false;
      }
      i++;
      continue;
    }
    if (c === '"' || c === "'") {
      inStr = true;
      strQuote = c;
      i++;
      continue;
    }
    if (c === '/' && text[i + 1] === '/') {
      i += 2;
      while (i < text.length && text[i] !== '\n' && text[i] !== '\r') {
        i++;
      }
      continue;
    }
    if (c === '/' && text[i + 1] === '*') {
      i += 2;
      while (i < text.length - 1) {
        if (text[i] === '*' && text[i + 1] === '/') {
          i += 2;
          break;
        }
        i++;
      }
      continue;
    }
    if (c === '{') {
      depth++;
    } else if (c === '}') {
      depth--;
      if (depth === 0) {
        return i;
      }
    }
    i++;
  }
  return -1;
}

/**
 * If `fun` at funStart is part of `prop fun` / `prop async fun`, return true (do not treat as plain `fun`).
 * @param {string} body
 * @param {number} funStart index of `f` in `fun`
 */
function isPropFunKeyword(body, funStart) {
  let i = funStart - 1;
  while (i >= 0 && /\s/.test(body[i])) {
    i--;
  }
  if (i < 0) {
    return false;
  }
  let end = i;
  while (i >= 0 && /[\w]/.test(body[i])) {
    i--;
  }
  const wordBeforeFun = body.slice(i + 1, end + 1);
  if (wordBeforeFun === 'prop') {
    return true;
  }
  if (wordBeforeFun === 'async') {
    while (i >= 0 && /\s/.test(body[i])) {
      i--;
    }
    if (i < 0) {
      return false;
    }
    end = i;
    while (i >= 0 && /[\w]/.test(body[i])) {
      i--;
    }
    return body.slice(i + 1, end + 1) === 'prop';
  }
  return false;
}

/**
 * @param {string} body class body without outer braces
 * @returns {Set<string>}
 */
function extractMembersFromClassBody(body) {
  const names = new Set();
  let m;
  const r1 = /\bprop\s+async\s+fun\s+(\w+)\s*\(/g;
  while ((m = r1.exec(body))) {
    names.add(m[1]);
  }
  const r2 = /\bprop\s+fun\s+(\w+)\s*\(/g;
  while ((m = r2.exec(body))) {
    names.add(m[1]);
  }
  const r3 = /\bprop\s+(?!async\b)(?!fun\b)(\w+)\s*[=;]/g;
  while ((m = r3.exec(body))) {
    names.add(m[1]);
  }
  const r4 = /\bfun\s+(\w+)\s*\(/g;
  while ((m = r4.exec(body))) {
    if (!isPropFunKeyword(body, m.index)) {
      names.add(m[1]);
    }
  }
  return names;
}

/**
 * @param {string} text full document
 * @returns {{ classes: Map<string, { members: Set<string>, extends: string | null }> }}
 */
function parseClasses(text) {
  /** @type {Map<string, { members: Set<string>, extends: string | null }>} */
  const classes = new Map();
  const re = /\bclass\s+(\w+)(?:\s+extends\s+(\w+))?\s*\{/g;
  let m;
  while ((m = re.exec(text)) !== null) {
    const openIdx = m.index + m[0].length - 1;
    const closeIdx = indexOfMatchingBrace(text, openIdx);
    if (closeIdx < 0) {
      continue;
    }
    const body = text.slice(openIdx + 1, closeIdx);
    const members = extractMembersFromClassBody(body);
    classes.set(m[1], {
      members,
      extends: m[2] || null,
    });
  }
  return { classes };
}

/**
 * Merge own + inherited members (own first, then parent-only names).
 * @param {string} className
 * @param {Map<string, { members: Set<string>, extends: string | null }>} classes
 * @param {Set<string>} [visited]
 * @returns {string[]}
 */
function membersForClass(className, classes, visited = new Set()) {
  if (visited.has(className)) {
    return [];
  }
  visited.add(className);
  const info = classes.get(className);
  if (!info) {
    return [];
  }
  const own = Array.from(info.members);
  const parentName = info.extends;
  if (!parentName || !classes.has(parentName)) {
    return own;
  }
  const parentList = membersForClass(parentName, classes, visited);
  const ownSet = new Set(own);
  const merged = own.concat(parentList.filter((n) => !ownSet.has(n)));
  return merged;
}

/**
 * @param {string} text
 * @param {number} offset only consider assignments ending at or before this offset
 * @returns {Map<string, string>} varName -> className
 */
function varToClassBeforeOffset(text, offset) {
  /** @type {Map<string, string>} */
  const map = new Map();
  const re = /\bvar\s+(\w+)\s*=\s*new\s*\(\s*(\w+)\s*\)/g;
  let m;
  while ((m = re.exec(text)) !== null) {
    const end = m.index + m[0].length;
    if (end <= offset) {
      map.set(m[1], m[2]);
    }
  }
  return map;
}

/**
 * Smallest containing class body for `this.` (innermost).
 * @param {string} text
 * @param {number} offset
 * @returns {{ name: string, extends: string | null } | null}
 */
function innermostClassAtOffset(text, offset) {
  const re = /\bclass\s+(\w+)(?:\s+extends\s+(\w+))?\s*\{/g;
  /** @type {{ name: string, extends: string | null, open: number, close: number, width: number }[]} */
  const ranges = [];
  let m;
  while ((m = re.exec(text)) !== null) {
    const open = m.index + m[0].length - 1;
    const close = indexOfMatchingBrace(text, open);
    if (close < 0) {
      continue;
    }
    ranges.push({
      name: m[1],
      extends: m[2] || null,
      open,
      close,
      width: close - open,
    });
  }
  const inside = ranges.filter((r) => offset > r.open && offset < r.close);
  if (!inside.length) {
    return null;
  }
  inside.sort((a, b) => a.width - b.width);
  const inner = inside[0];
  return { name: inner.name, extends: inner.extends };
}

/**
 * @param {string} text
 */
function parseDocument(text) {
  return parseClasses(text);
}

/** Reserved words — excluded from symbol completion. */
const RESERVED_SYMBOLS = new Set([
  'async',
  'await',
  'break',
  'class',
  'do',
  'elif',
  'else',
  'extends',
  'false',
  'for',
  'fun',
  'if',
  'null',
  'prop',
  'return',
  'super',
  'this',
  'true',
  'var',
  'while',
  'new',
  'require',
]);

/**
 * Collect identifier-like names from text before `offset` (locals / params / fun names / class names).
 * @param {string} text
 * @param {number} offset
 * @returns {string[]}
 */
function collectSymbolNames(text, offset) {
  const names = new Set();
  const head = text.slice(0, offset);
  let m;

  const reVar = /\bvar\s+(\w+)/g;
  while ((m = reVar.exec(head)) !== null) {
    names.add(m[1]);
  }

  const reFun = /\bfun\s+(\w+)\s*\(\s*([^)]*)\)/g;
  while ((m = reFun.exec(head)) !== null) {
    names.add(m[1]);
    const params = m[2];
    if (params) {
      params.split(',').forEach(function (p) {
        const id = p.trim().match(/^(\w+)/);
        if (id) {
          names.add(id[1]);
        }
      });
    }
  }

  const rePropField = /\bprop\s+(?!async\b)(?!fun\b)(\w+)\s*[=;]/g;
  while ((m = rePropField.exec(head)) !== null) {
    names.add(m[1]);
  }

  const rePropFun = /\bprop\s+async\s+fun\s+(\w+)\s*\(/g;
  while ((m = rePropFun.exec(head)) !== null) {
    names.add(m[1]);
  }
  const rePropFun2 = /\bprop\s+fun\s+(\w+)\s*\(/g;
  while ((m = rePropFun2.exec(head)) !== null) {
    names.add(m[1]);
  }

  const reClass = /\bclass\s+(\w+)/g;
  while ((m = reClass.exec(head)) !== null) {
    names.add(m[1]);
  }

  const out = [];
  names.forEach(function (n) {
    if (n && n.length && !RESERVED_SYMBOLS.has(n)) {
      out.push(n);
    }
  });
  out.sort(function (a, b) {
    return a.localeCompare(b);
  });
  return out;
}

module.exports = {
  parseDocument,
  membersForClass,
  varToClassBeforeOffset,
  innermostClassAtOffset,
  indexOfMatchingBrace,
  collectSymbolNames,
};
