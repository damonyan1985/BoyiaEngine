/**
 * Boyia language keywords (VM lexer / tm grammar) and single-letter shortcuts.
 */

/** @type {string[]} */
const KEYWORDS = [
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
];

/** @type {string[]} */
const GLOBALS = ['new', 'require'];

/**
 * Legacy single-character triggers → keyword candidates.
 * @type {Record<string, string[]>}
 */
const SHORTCUTS = {
  a: ['async', 'await'],
  b: ['break'],
  c: ['class'],
  d: ['do'],
  e: ['extends'],
  f: ['fun', 'false', 'for'],
  i: ['if'],
  n: ['new', 'null'],
  p: ['prop'],
  r: ['return'],
  t: ['true', 'this'],
  v: ['var'],
  w: ['while'],
};

/** @type {Record<string, string>} */
const KEYWORD_DOC = {
  async: '异步函数修饰符，可与 `fun` 配合使用。',
  await: '在 async 函数内等待异步表达式结果。',
  break: '跳出最近一层 `while` / `for` / `do` 循环。',
  class: '定义类。',
  do: '`do { ... } while (cond);` 循环。',
  elif: '`if` 分支后的条件分支。',
  else: '`if` / `elif` 之后的否则分支。',
  extends: '类继承，如 `class B extends A`。',
  false: '布尔假。',
  for: '`for` 循环。',
  fun: '定义函数或方法。',
  if: '条件分支。',
  null: '空值。',
  prop: '定义属性方法（可访问 `this`）。',
  return: '从函数返回。',
  super: '调用父类实现。',
  this: '当前对象引用。',
  true: '布尔真。',
  var: '声明变量。',
  while: '`while` 循环。',
  new: '创建实例，如 `new(ClassName)`。',
  require: '加载并执行另一个 `.boyia` 脚本模块。',
};

module.exports = {
  KEYWORDS,
  GLOBALS,
  SHORTCUTS,
  KEYWORD_DOC,
};
