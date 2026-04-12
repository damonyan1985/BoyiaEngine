'use strict';

/**
 * Boyia editor default colors (Dark+-style). Single source of truth — edit here only.
 * Run `npm run sync-theme-defaults` (or `npm run build`, which runs it via prebuild)
 * to copy these into package.json → contributes.configurationDefaults.
 *
 * VS Code does not read token colors from extension JS at runtime; they must live in
 * the extension manifest. This module exists so values are hardcoded in code, not
 * hand-edited in package.json.
 */

/** @type {Readonly<Record<string, string>>} */
const COLORS = Object.freeze({
  variable: '#9CDCFE',
  /** Function parameters — same as variable by default; change COLORS.parameter to diverge */
  parameter: '#9CDCFE',
  function: '#DCDCAA',
  classType: '#4EC9B0',
});

/**
 * @returns {Record<string, unknown>}
 */
function buildConfigurationDefaults() {
  return {
    '[boyia]': {
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
        ],
      },
    },
  };
}

module.exports = {
  COLORS,
  buildConfigurationDefaults,
};
