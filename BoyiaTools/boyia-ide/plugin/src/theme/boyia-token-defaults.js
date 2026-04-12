'use strict';

/**
 * Boyia editor default colors (Dark+-style). Single source of truth — edit here only.
 * Run `npm run sync-theme-defaults` (or `npm run build`, which runs it via prebuild)
 * to copy these into package.json → contributes.configurationDefaults.
 *
 * All scopes use the `*.boyia` suffix — no dependency on TypeScript / JS TextMate scopes.
 */

/** @type {Readonly<Record<string, string>>} */
const COLORS = Object.freeze({
  variable: '#9CDCFE',
  parameter: '#9CDCFE',
  function: '#DCDCAA',
  classType: '#4EC9B0',
  /** `{}` — class / block */
  braceCurly: '#FFD700',
  /** `[]` */
  braceSquare: '#C586C0',
  /** `()` — parameters & new(...) */
  braceRound: '#CE9178',
});

/**
 * @returns {Record<string, unknown>}
 */
function buildConfigurationDefaults() {
  return {
    '[boyia]': {
      'editor.bracketPairColorization.enabled': true,
      'editor.guides.bracketPairs': 'active',
      'editor.tokenColorCustomizations': {
        textMateRules: [
          {
            scope: ['variable.other.readwrite.boyia'],
            settings: { foreground: COLORS.variable },
          },
          {
            scope: ['variable.parameter.boyia'],
            settings: { foreground: COLORS.parameter },
          },
          {
            scope: ['entity.name.function.boyia'],
            settings: { foreground: COLORS.function },
          },
          {
            scope: ['entity.name.type.class.boyia', 'entity.other.inherited-class.boyia'],
            settings: { foreground: COLORS.classType },
          },
          {
            scope: ['punctuation.definition.block.boyia'],
            settings: { foreground: COLORS.braceCurly },
          },
          {
            scope: ['meta.brace.square.boyia'],
            settings: { foreground: COLORS.braceSquare },
          },
          {
            scope: [
              'punctuation.definition.parameters.begin.boyia',
              'punctuation.definition.parameters.end.boyia',
              'punctuation.definition.arguments.begin.boyia',
              'punctuation.definition.arguments.end.boyia',
            ],
            settings: { foreground: COLORS.braceRound },
          },
        ],
      },
    },
  };
}

module.exports = {
  COLORS,
  buildConfigurationDefaults,
};
